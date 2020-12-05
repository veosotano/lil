/********************************************************************
 *
 *      LIL Is a Language
 *
 *      AUTHORS: Miro Keller
 *
 *      COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *      LICENSE: see LICENSE file
 *
 *      This file generates the final output using LLVM
 *
 ********************************************************************/

#include <unistd.h>

#include "LILOutputEmitter.h"
#include "LILCodeUnit.h"
#include "LILIREmitter.h"
#include "LILPassManager.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace LIL;


namespace LIL
{
    class LILOutputEmitterPrivate
    {
        friend class LILOutputEmitter;
        
        LILOutputEmitterPrivate()
        : pm(std::make_unique<LILPassManager>())
        , irEmitter(nullptr)
        , targetMachine(nullptr)
        , verbose(false)
        , debugIREmitter(false)
        {
        }
        LILString file;
        LILString dir;
        LILString source;

        std::unique_ptr<LILPassManager> pm;
        LILIREmitter * irEmitter;
        llvm::TargetMachine * targetMachine;
        
        bool verbose;
        bool debugIREmitter;
    };
}


LILOutputEmitter::LILOutputEmitter()
: d(new LILOutputEmitterPrivate())
{
    
}

LILOutputEmitter::~LILOutputEmitter()
{
    if (d->irEmitter != nullptr) {
        delete d->irEmitter;
    }
    delete d;
}

llvm::Module * LILOutputEmitter::getLLVMModule() const
{
    return d->irEmitter->getLLVMModule();
}

void LILOutputEmitter::run(LILCodeUnit * cu)
{
    d->pm->setVerbose(d->verbose);
    
    d->irEmitter = new LILIREmitter(this->getFile());
    
    std::error_code error_code;
    
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetDisassembler();
    
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        std::cerr << "Error: could not look up target.\n";
        return;
    }
    
    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto relocModel = llvm::Optional<llvm::Reloc::Model>();
    d->targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, relocModel);
    
    llvm::Module * theModule = d->irEmitter->getLLVMModule();
    theModule->setDataLayout(d->targetMachine->createDataLayout());
    theModule->setTargetTriple(targetTriple);
    
    //emit IR
    d->irEmitter->setDebug(d->debugIREmitter);
    std::vector<LILVisitor *> passes;
    passes.push_back(d->irEmitter);
    d->pm->execute(passes, cu->getRootNode(), d->source);
    
    if (d->pm->hasErrors()) {
        std::cerr << "Errors encountered. Exiting.\n";
        return;
    }

    bool broken = llvm::verifyModule(*theModule, &llvm::errs(), nullptr);
    if (broken) {
        std::cerr << "\n\n";
        std::cerr << "ERRORS FOUND. PLEASE CHECK OUTPUT ABOVE ^^^^^^^^\n";
        std::cerr << "\n\n";
    }
}

void LILOutputEmitter::compileToO(std::string name, LILCodeUnit * cu)
{
    std::error_code error_code;
    llvm::raw_fd_ostream dest(name.data(), error_code, llvm::sys::fs::OF_None);
    if (error_code) {
        std::cerr << "Error: could not open destination file.\n";
        return;
    }
    this->run(cu);
    
    if (d->pm->hasErrors()) {
        return;
    }
    
    if (d->verbose) {
        d->irEmitter->printIR(llvm::errs());
    }
    
    llvm::legacy::PassManager emitPassMngr;
    auto fileType = llvm::TargetMachine::CGFT_ObjectFile;
    
    if (d->targetMachine->addPassesToEmitFile(emitPassMngr, dest, nullptr, fileType)) {
        std::cerr << "Error: could not create file type for emitting code.\n";
        return;
    }
    llvm::Module * theModule = d->irEmitter->getLLVMModule();
    emitPassMngr.run(*theModule);
    
    dest.flush();
}

void LILOutputEmitter::compileToS(std::string name, LILCodeUnit * cu)
{
    std::error_code error_code;
    this->run(cu);
    
    if (d->pm->hasErrors()) {
        return;
    }
    
    if (d->verbose) {
        d->irEmitter->printIR(llvm::errs());
    }
    
    llvm::raw_fd_ostream dest(name.data(), error_code, llvm::sys::fs::OF_None);
    if (error_code) {
        std::cerr << "Error: could not open destination file.\n";
        return;
    }
    d->irEmitter->printIR(dest);
    dest.flush();
}

void LILOutputEmitter::printToOutput(LILCodeUnit * cu)
{
    this->run(cu);
    if (d->pm->hasErrors()) {
        return;
    }
    if (d->irEmitter){
        //avoid showing IR twice when outputting
        //isatty(1) checks stdout and isatty(2) checks stderr, they say
        if (d->verbose) {
            d->irEmitter->printIR(llvm::errs());
            std::cerr << "\n";
            
            //if stdout and stderr point to different things
            //we need to output on both places. we already got
            //stderr above, so if they match we do nothing
            if ( isatty(1) != isatty(2) ) {
                d->irEmitter->printIR(llvm::outs());
            }
        } else {
            d->irEmitter->printIR(llvm::outs());
        }
    }
}

void LILOutputEmitter::setVerbose(bool value)
{
    d->verbose = value;
}

void LILOutputEmitter::setDebugIREmitter(bool value)
{
    d->debugIREmitter = value;
}

void LILOutputEmitter::setFile(LILString file)
{
    d->file = file;
}

LILString LILOutputEmitter::getFile() const
{
    return d->file;
}

void LILOutputEmitter::setDir(LILString dir)
{
    d->dir = dir;
}

LILString LILOutputEmitter::getDir() const
{
    return d->dir;
}

void LILOutputEmitter::setSource(LILString source)
{
    d->source = source;
}

LILString LILOutputEmitter::getSource() const
{
    return d->source;
}

bool LILOutputEmitter::hasErrors() const
{
    return d->pm->hasErrors();
}
