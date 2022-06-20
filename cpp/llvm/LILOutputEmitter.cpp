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
#include "LILRootNode.h"
#include "LILIREmitter.h"

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
        : irEmitter(nullptr)
        , targetMachine(nullptr)
        , verbose(false)
        , debugIREmitter(false)
        {
        }
        LILString inFile;
        LILString outFile;
        LILString dir;
        LILString source;
        LILString cpu;
        LILString vendor;

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

void LILOutputEmitter::run(std::shared_ptr<LILRootNode> rootNode)
{
    d->irEmitter = new LILIREmitter(this->getInFile());
    
    std::error_code error_code;
    
    std::string targetTriple;
    const std::string & cpuString = this->getCPU().data();
    const std::string & vendorString = this->getVendor().data();

    if (cpuString.length() == 0 || vendorString.length() == 0) {
        std::cerr << "Error: Unknown CPU or vendor: " << cpuString << "/" << vendorString << ".\n";
        targetTriple = llvm::sys::getDefaultTargetTriple();
    } else {
        targetTriple = cpuString + "-" + vendorString;
    }

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmPrinter();

    LLVMInitializeARMTargetInfo();
    LLVMInitializeARMTarget();
    LLVMInitializeARMTargetMC();
    LLVMInitializeARMAsmPrinter();
    
    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64Target();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64AsmPrinter();

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        std::cerr << "Error: could not look up target: " << targetTriple << "\n";
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
    d->irEmitter->setVerbose(d->verbose);
    d->irEmitter->initializeVisit();
    d->irEmitter->performVisit(rootNode);
    if (d->irEmitter->hasErrors()) {
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

void LILOutputEmitter::compileToO(std::shared_ptr<LILRootNode> rootNode)
{
    std::error_code error_code;
    llvm::raw_fd_ostream dest(this->getDir().data() + "/" + this->getOutFile().data(), error_code, llvm::sys::fs::OF_None);
    if (error_code) {
        std::cerr << "Error: could not open destination file.\n";
        return;
    }
    this->run(rootNode);

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

void LILOutputEmitter::compileToS(std::shared_ptr<LILRootNode> rootNode)
{
    std::error_code error_code;
    this->run(rootNode);

    if (d->verbose) {
        d->irEmitter->printIR(llvm::errs());
    }
    
    llvm::raw_fd_ostream dest(this->getDir().data() + "/" + this->getOutFile().data(), error_code, llvm::sys::fs::OF_None);
    if (error_code) {
        std::cerr << "Error: could not open destination file.\n";
        return;
    }
    d->irEmitter->printIR(dest);
    dest.flush();
}

void LILOutputEmitter::printToOutput(std::shared_ptr<LILRootNode> rootNode)
{
    this->run(rootNode);

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

void LILOutputEmitter::setInFile(const LILString & file)
{
    d->inFile = file;
}

const LILString & LILOutputEmitter::getInFile() const
{
    return d->inFile;
}

void LILOutputEmitter::setOutFile(const LILString & file)
{
    d->outFile = file;
}

const LILString & LILOutputEmitter::getOutFile() const
{
    return d->outFile;
}

void LILOutputEmitter::setDir(const LILString & dir)
{
    d->dir = dir;
}

const LILString & LILOutputEmitter::getDir() const
{
    return d->dir;
}

void LILOutputEmitter::setSource(const LILString & source)
{
    d->source = source;
}

const LILString & LILOutputEmitter::getSource() const
{
    return d->source;
}

void LILOutputEmitter::setCPU(const LILString & value)
{
    d->cpu = value;
}

const LILString & LILOutputEmitter::getCPU() const
{
    return d->cpu;
}

void LILOutputEmitter::setVendor(const LILString & value)
{
    d->vendor = value;
}

const LILString & LILOutputEmitter::getVendor() const
{
    return d->vendor;
}
