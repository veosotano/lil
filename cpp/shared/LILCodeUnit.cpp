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
#include "LILConversionInserter.h"
#include "LILFieldSorter.h"
#include "LILTypeValidator.h"
#include "LILMethodInserter.h"
#include "LILNameLowerer.h"
#include "LILParameterSorter.h"
#include "LILPassManager.h"
#include "LILStructureLowerer.h"
#include "LILToStringVisitor.h"
#include "LILTypeGuesser.h"
#include "LILTypeResolver.h"

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
        , debugLilStd(false)
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
        , debugConversionInserter(false)
        , debugTypeResolver(false)
        , needsStdLil(true)
        , isBeingImported(false)
        {
        }
        LILString file;
        LILString dir;
        LILString source;
        std::unique_ptr<LILASTBuilder> astBuilder;
        std::unique_ptr<LILCodeParser> parser;
        std::unique_ptr<LILPassManager> pm;
        std::vector<LILString> alreadyImportedFiles;

        bool isMain;

        bool verbose;
        bool debugLilStd;
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
        bool debugConversionInserter;
        bool debugTypeResolver;
        bool needsStdLil;
        bool isBeingImported;
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

void LILCodeUnit::setNeedsStdLil(bool value)
{
    d->needsStdLil = value;
}

bool LILCodeUnit::getNeedsStdLil() const
{
    return d->needsStdLil;
}

void LILCodeUnit::setIsBeingImported(bool value)
{
    d->isBeingImported = value;
}

bool LILCodeUnit::getIsBeingImported() const
{
    return d->isBeingImported;
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
    if (d->needsStdLil) {
        auto rootNode = this->getRootNode();
        auto needsInstr = std::make_shared<LILInstruction>();
        needsInstr->setInstructionType(InstructionTypeNeeds);
        needsInstr->setName("needs");
        auto strConst = std::make_shared<LILStringLiteral>();
        strConst->setValue("\"std/lil.lil\"");
        needsInstr->setArgument(strConst);
        rootNode->addNode(needsInstr);
        if (!d->debugLilStd) {
            needsInstr->setVerbose(false);
            needsInstr->hidden = true;
        }
    }

    d->parser->parseString(d->source);
}

void LILCodeUnit::runPasses()
{
    bool verbose = d->verbose;

    if (verbose) {
        d->pm->addPass(std::make_unique<LILToStringVisitor>());
    }
    
    //handle #needs instructions
    auto needsImporter = new LILNeedsImporter();
    for (auto aif : d->alreadyImportedFiles) {
        needsImporter->addAlreadyImportedFile(aif);
    }
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

    if (!this->getIsBeingImported()) {
        //conversion inserting
        auto convInserter = std::make_unique<LILConversionInserter>();
        convInserter->setDebug(d->debugConversionInserter);
        d->pm->addPass(std::move(convInserter));
        if (verbose) {
            auto stringVisitor = std::make_unique<LILToStringVisitor>();
            stringVisitor->setPrintHeadline(false);
            d->pm->addPass(std::move(stringVisitor));
        }

        //type resolving
        auto tyResolver = std::make_unique<LILTypeResolver>();
        tyResolver->setDebug(d->debugTypeResolver);
        d->pm->addPass(std::move(tyResolver));
        if (verbose) {
            auto stringVisitor = std::make_unique<LILToStringVisitor>();
            stringVisitor->setPrintHeadline(false);
            d->pm->addPass(std::move(stringVisitor));
        }
        
        //structure lowering
        auto structureLowerer = std::make_unique<LILStructureLowerer>();
        structureLowerer->setDebug(d->debugStructureLowerer);
        d->pm->addPass(std::move(structureLowerer));
        if (verbose) {
            auto stringVisitor = std::make_unique<LILToStringVisitor>();
            stringVisitor->setPrintHeadline(false);
            d->pm->addPass(std::move(stringVisitor));
        }

    } //end if not being imported
    
    //name lowering
    auto nameLowerer = new LILNameLowerer();
    nameLowerer->setDebug(d->debugNameLowerer);
    passes.push_back(nameLowerer);
    if (d->verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        d->pm->addPass(std::move(stringVisitor));
    }

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

void LILCodeUnit::setDebugLilStd(bool value)
{
    d->debugLilStd = value;
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

void LILCodeUnit::setDebugConversionInserter(bool value)
{
    d->debugConversionInserter = value;
}

void LILCodeUnit::setDebugTypeResolver(bool value)
{
    d->debugTypeResolver = value;
}

bool LILCodeUnit::hasErrors() const
{
    return d->astBuilder->hasErrors() || d->pm->hasErrors();
}

void LILCodeUnit::addAlreadyImportedFile(const LILString & path)
{
    d->alreadyImportedFiles.push_back(path);
}

bool LILCodeUnit::isAlreadyImported(const LILString & path)
{
    for (auto str : d->alreadyImportedFiles) {
        if (str == path) {
            return true;
        }
    }
    return false;
}
