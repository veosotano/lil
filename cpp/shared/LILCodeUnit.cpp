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

#include "LILCodeUnit.h"
#include "LILASTBuilder.h"
#include "LILCodeParser.h"

#include "LILNeedsImporter.h"
#include "LILASTValidator.h"
#include "LILFieldSorter.h"
#include "LILTypeValidator.h"
#include "LILMethodInserter.h"
#include "LILNameLowerer.h"
#include "LILParameterSorter.h"
#include "LILPassManager.h"
#include "LILStructureLowerer.h"
#include "LILToStringVisitor.h"
#include "LILTypeGuesser.h"

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
        , isMain(false)
        , verbose(false)
        , debugNeedsImporter(false)
        , debugAST(false)
        , debugASTValidator(false)
        , debugTypeGuesser(false)
        , debugStructureLowerer(false)
        , debugMethodInserter(false)
        , debugNameLowerer(false)
        , debugFieldSorter(false)
        , debugParameterSorter(false)
        , debugTypeValidator(false)
        {
        }
        LILString file;
        LILString dir;
        LILString source;
        std::unique_ptr<LILASTBuilder> astBuilder;
        std::unique_ptr<LILCodeParser> parser;
        std::unique_ptr<LILPassManager> pm;

        bool isMain;

        bool verbose;
        bool debugNeedsImporter;
        bool debugAST;
        bool debugASTValidator;
        bool debugTypeGuesser;
        bool debugStructureLowerer;
        bool debugMethodInserter;
        bool debugNameLowerer;
        bool debugFieldSorter;
        bool debugParameterSorter;
        bool debugTypeValidator;
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

std::shared_ptr<LILRootNode> LILCodeUnit::getRootNode() const
{
    return d->astBuilder->getRootNode();
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
    
    this->buildAST();

    if (d->astBuilder->hasErrors()) {
        d->astBuilder->printErrors(d->source);
        return;
    }
    
    this->runPasses();
}

void LILCodeUnit::buildAST()
{
    d->parser->parseString(d->source);
}

void LILCodeUnit::runPasses()
{
    bool verbose = d->verbose;

    if (verbose) {
        d->pm->addPass(std::make_unique<LILToStringVisitor>());
    }
    
    //handle #needs instructions
    auto needsImporter = std::make_unique<LILNeedsImporter>();
    needsImporter->setDebug(d->debugNeedsImporter);
    needsImporter->setDir(d->dir);
    needsImporter->setDebugAST(d->debugAST);
    d->pm->addPass(std::move(needsImporter));
    if (verbose) {
        auto stringVisitor = std::make_unique<LILToStringVisitor>();
        stringVisitor->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor));
    }

    //ast validation
    auto astValidator = std::make_unique<LILASTValidator>();
    astValidator->setDebug(d->debugASTValidator);
    d->pm->addPass(std::move(astValidator));

    //method inserter
    auto methodInserter = std::make_unique<LILMethodInserter>();
    methodInserter->setDebug(d->debugMethodInserter);
    d->pm->addPass(std::move(methodInserter));
    if (verbose) {
        auto stringVisitor = std::make_unique<LILToStringVisitor>();
        stringVisitor->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor));
    }

    //type guessing
    auto typeGuesser = std::make_unique<LILTypeGuesser>();
    typeGuesser->setDebug(d->debugTypeGuesser);
    d->pm->addPass(std::move(typeGuesser));
    if (verbose) {
        auto stringVisitor = std::make_unique<LILToStringVisitor>();
        stringVisitor->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor));
    }
    
    //field sorting
    auto fieldSorter = std::make_unique<LILFieldSorter>();
    fieldSorter->setDebug(d->debugFieldSorter);
    d->pm->addPass(std::move(fieldSorter));
    if (verbose) {
        auto stringVisitor = std::make_unique<LILToStringVisitor>();
        stringVisitor->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor));
    }
    
    //parameter sorting
    auto parameterSorter = std::make_unique<LILParameterSorter>();
    parameterSorter->setDebug(d->debugParameterSorter);
    d->pm->addPass(std::move(parameterSorter));
    if (verbose) {
        auto stringVisitor = std::make_unique<LILToStringVisitor>();
        stringVisitor->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor));
    }
    
    //type validation
    auto tyValidator = std::make_unique<LILTypeValidator>();
    tyValidator->setDebug(d->debugTypeValidator);
    d->pm->addPass(std::move(tyValidator));
    if (verbose) {
        auto stringVisitor = std::make_unique<LILToStringVisitor>();
        stringVisitor->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor));
    }

    if (d->isMain) {
        //structure lowering
        auto structureLowerer = std::make_unique<LILStructureLowerer>();
        structureLowerer->setDebug(d->debugStructureLowerer);
        d->pm->addPass(std::move(structureLowerer));
        if (verbose) {
            auto stringVisitor = std::make_unique<LILToStringVisitor>();
            stringVisitor->setPrintHeadline(false);
            d->pm->addPass(std::move(stringVisitor));
        }

        //name lowering
        auto nameLowerer = std::make_unique<LILNameLowerer>();
        nameLowerer->setDebug(d->debugNameLowerer);
        d->pm->addPass(std::move(nameLowerer));
        if (d->verbose) {
            auto stringVisitor = std::make_unique<LILToStringVisitor>();
            stringVisitor->setPrintHeadline(false);
            d->pm->addPass(std::move(stringVisitor));
        }

    } //end if isMain

    //execute the passes
    d->pm->execute(d->astBuilder->getRootNode(), d->source);

    if (d->pm->hasErrors()) {
        std::cerr << "Errors encountered. Exiting.\n\n";
    }
}

void LILCodeUnit::setIsMain(bool value)
{
    d->isMain = value;
    d->astBuilder->setIsMain(value);
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

void LILCodeUnit::setDebugNeedsImporter(bool value)
{
    d->debugNeedsImporter = value;
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

void LILCodeUnit::setDebugTypeValidator(bool value)
{
    d->debugTypeValidator = value;
}

bool LILCodeUnit::hasErrors() const
{
    return d->astBuilder->hasErrors() || d->pm->hasErrors();
}
