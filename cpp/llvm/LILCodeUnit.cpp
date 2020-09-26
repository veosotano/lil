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
 *      This file converts the ast into IR representation
 *
 ********************************************************************/

#include <unistd.h>

#include "LILCodeUnit.h"
#include "LILASTBuilder.h"
#include "LILCodeParser.h"
#include "LILIREmitter.h"
#include "LILPassManager.h"

#include "LILASTValidator.h"
#include "LILIREmitter.h"
#include "LILFieldSorter.h"
#include "LILMethodInserter.h"
#include "LILNameLowerer.h"
#include "LILParameterSorter.h"
#include "LILPassManager.h"
#include "LILStructureLowerer.h"
#include "LILToStringVisitor.h"
#include "LILTypeGuesser.h"

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
    class LILCodeUnitPrivate
    {
        friend class LILCodeUnit;

        LILCodeUnitPrivate()
        : astBuilder(std::make_unique<LILASTBuilder>())
        , parser(std::make_unique<LILCodeParser>(astBuilder.get()))
        , pm(std::make_unique<LILPassManager>())
        , irEmitter(nullptr)
        , verbose(false)
        , targetMachine(nullptr)
        , debugAST(false)
        , debugASTValidator(false)
        , debugTypeGuesser(false)
        , debugStructureLowerer(false)
        , debugMethodInserter(false)
        , debugNameLowerer(false)
        , debugFieldSorter(false)
        , debugParameterSorter(false)
        , debugIREmitter(false)
        {
        }
        LILString file;
        LILString dir;
        LILString source;
        std::unique_ptr<LILASTBuilder> astBuilder;
        std::unique_ptr<LILCodeParser> parser;
        std::unique_ptr<LILPassManager> pm;
        LILIREmitter * irEmitter;
        llvm::TargetMachine * targetMachine;
        bool verbose;
        bool debugAST;
        bool debugASTValidator;
        bool debugTypeGuesser;
        bool debugStructureLowerer;
        bool debugMethodInserter;
        bool debugNameLowerer;
        bool debugFieldSorter;
        bool debugParameterSorter;
        bool debugIREmitter;
    };
}


LILCodeUnit::LILCodeUnit()
: d(new LILCodeUnitPrivate())
{

}

LILCodeUnit::~LILCodeUnit()
{
    delete d;
}

llvm::Module * LILCodeUnit::getLLVMModule() const
{
    return d->irEmitter->getLLVMModule();
}

void LILCodeUnit::setFile(LILString file)
{
    d->file = file;
}

LILString LILCodeUnit::getFile() const
{
    return d->file;
}

void LILCodeUnit::setDir(LILString dir)
{
    d->dir = dir;
}

LILString LILCodeUnit::getDir() const
{
    return d->dir;
}

void LILCodeUnit::setSource(LILString source)
{
    d->source = source;
}

LILString LILCodeUnit::getSource() const
{
    return d->source;
}

void LILCodeUnit::run()
{
    bool verbose = d->verbose;

    d->pm->setVerbose(verbose);

    d->parser->parseString(d->source);

    if (verbose) {
        d->pm->addPass(std::make_unique<LILToStringVisitor>());
    }

    //validation
    auto validator = std::make_unique<LILASTValidator>();
    validator->setDebug(d->debugASTValidator);
    d->pm->addPass(std::move(validator));

    //field sorting
    auto fieldSorter = std::make_unique<LILFieldSorter>();
    fieldSorter->setDebug(d->debugFieldSorter);
    d->pm->addPass(std::move(fieldSorter));
    if (verbose) {
        auto stringVisitor0 = std::make_unique<LILToStringVisitor>();
        stringVisitor0->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor0));
    }

    //parameter sorting
    auto parameterSorter = std::make_unique<LILParameterSorter>();
    parameterSorter->setDebug(d->debugParameterSorter);
    d->pm->addPass(std::move(parameterSorter));
    if (verbose) {
        auto stringVisitor0 = std::make_unique<LILToStringVisitor>();
        stringVisitor0->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor0));
    }
    
    //method inserter
    auto methodInserter = std::make_unique<LILMethodInserter>();
    methodInserter->setDebug(d->debugMethodInserter);
    d->pm->addPass(std::move(methodInserter));
    if (verbose) {
        auto stringVisitor1 = std::make_unique<LILToStringVisitor>();
        stringVisitor1->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor1));
    }

    //type guessing
    auto typeGuesser = std::make_unique<LILTypeGuesser>();
    typeGuesser->setDebug(d->debugTypeGuesser);
    d->pm->addPass(std::move(typeGuesser));
    if (verbose) {
        auto stringVisitor2 = std::make_unique<LILToStringVisitor>();
        stringVisitor2->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor2));
    }

    //structure lowering
    auto structureLowerer = std::make_unique<LILStructureLowerer>();
    structureLowerer->setDebug(d->debugStructureLowerer);
    d->pm->addPass(std::move(structureLowerer));
    if (verbose) {
        auto stringVisitor3 = std::make_unique<LILToStringVisitor>();
        stringVisitor3->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor3));
    }

    //name lowering
    auto nameLowerer = std::make_unique<LILNameLowerer>();
    nameLowerer->setDebug(d->debugNameLowerer);
    d->pm->addPass(std::move(nameLowerer));
    if (d->verbose) {
        auto stringVisitor4 = std::make_unique<LILToStringVisitor>();
        stringVisitor4->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor4));
    }

    //emit IR
    auto irEmitter = std::make_unique<LILIREmitter>(this->getFile());
    irEmitter->setDebug(d->debugIREmitter);
    d->irEmitter = irEmitter.get();
    d->pm->addPass(std::move(irEmitter));

    //execute the passes
    d->pm->execute(d->astBuilder->getRootNode(), d->source);

    if (d->pm->hasErrors()) {
        std::cerr << "Errors encountered. Exiting.";
        return;
    }

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
    
    bool broken = llvm::verifyModule(*theModule, &llvm::errs(), nullptr);
    if (broken) {
        std::cerr << "\n\n";
        std::cerr << "ERRORS FOUND. PLEASE CHECK OUTPUT ABOVE ^^^^^^^^\n";
        std::cerr << "\n\n";
    }
}

void LILCodeUnit::compileToO(std::string outName)
{
    std::error_code error_code;
    llvm::raw_fd_ostream dest(outName.data(), error_code, llvm::sys::fs::OF_None);
    if (error_code) {
        std::cerr << "Error: could not open destination file.\n";
        return;
    }
    this->run();

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

void LILCodeUnit::compileToS(std::string outName)
{
    std::error_code error_code;
    this->run();

    if (d->verbose) {
        d->irEmitter->printIR(llvm::errs());
    }

    llvm::raw_fd_ostream dest(outName.data(), error_code, llvm::sys::fs::OF_None);
    if (error_code) {
        std::cerr << "Error: could not open destination file.\n";
        return;
    }
    d->irEmitter->printIR(dest);
    dest.flush();
}

void LILCodeUnit::printToOutput()
{
    this->run();
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

void LILCodeUnit::setVerbose(bool value)
{
    d->verbose = value;
    if(d->astBuilder){
        d->astBuilder->setVerbose(value);
    }
}

void LILCodeUnit::setDebugAST(bool value)
{
    d->debugAST = value;
    if(d->astBuilder){
        d->astBuilder->setDebugAST(value);
    }
}

void LILCodeUnit::setDebugASTValidator(bool value)
{
    d->debugASTValidator = value;
}

void LILCodeUnit::setDebugTypeGuesser(bool value)
{
    d->debugTypeGuesser = value;
}

void LILCodeUnit::setDebugStructureLowerer(bool value)
{
    d->debugStructureLowerer = value;
}

void LILCodeUnit::setDebugFieldSorter(bool value)
{
    d->debugFieldSorter = value;
}

void LILCodeUnit::setDebugParameterSorter(bool value)
{
    d->debugParameterSorter = value;
}

void LILCodeUnit::setDebugMethodInserter(bool value)
{
    d->debugMethodInserter = value;
}

void LILCodeUnit::setDebugNameLowerer(bool value)
{
    d->debugNameLowerer = value;
}

void LILCodeUnit::setDebugIREmitter(bool value)
{
    d->debugIREmitter = value;
}

