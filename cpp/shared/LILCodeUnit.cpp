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
#include "LILConstantFolder.h"
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
        , debugLilStd(false)
        , debugNeedsImporter(false)
        , debugAST(false)
        , debugASTValidator(false)
        , debugTypeGuesser(false)
        , debugStructureLowerer(false)
        , debugConstantFolder(false)
        , debugMethodInserter(false)
        , debugNameLowerer(false)
        , debugFieldSorter(false)
        , debugParameterSorter(false)
        , debugTypeValidator(false)
        , debugConversionInserter(false)
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
        bool debugConstantFolder;
        bool debugMethodInserter;
        bool debugNameLowerer;
        bool debugFieldSorter;
        bool debugParameterSorter;
        bool debugTypeValidator;
        bool debugConversionInserter;
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

    std::vector<LILVisitor *> passes;

    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        passes.push_back(stringVisitor);
    }
    
    //handle #needs instructions
    auto needsImporter = new LILNeedsImporter();
    for (auto aif : d->alreadyImportedFiles) {
        needsImporter->addAlreadyImportedFile(aif);
    }
    needsImporter->setDebug(d->debugNeedsImporter);
    needsImporter->setDir(d->dir);
    needsImporter->setDebugAST(d->debugAST);
    passes.push_back(needsImporter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //ast validation
    auto astValidator = new LILASTValidator();
    astValidator->setDebug(d->debugASTValidator);
    passes.push_back(astValidator);

    //method inserter
    auto methodInserter = new LILMethodInserter();
    methodInserter->setDebug(d->debugMethodInserter);
    passes.push_back(methodInserter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //type guessing
    auto typeGuesser = new LILTypeGuesser();
    typeGuesser->setDebug(d->debugTypeGuesser);
    passes.push_back(typeGuesser);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //field sorting
    auto fieldSorter = new LILFieldSorter();
    fieldSorter->setDebug(d->debugFieldSorter);
    passes.push_back(fieldSorter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //parameter sorting
    auto parameterSorter = new LILParameterSorter();
    parameterSorter->setDebug(d->debugParameterSorter);
    passes.push_back(parameterSorter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //type validation
    auto tyValidator = new LILTypeValidator();
    tyValidator->setDebug(d->debugTypeValidator);
    passes.push_back(tyValidator);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    if (!this->getIsBeingImported()) {
        //conversion inserting
        auto convInserter = new LILConversionInserter();
        convInserter->setDebug(d->debugConversionInserter);
        passes.push_back(convInserter);
        if (verbose) {
            auto stringVisitor = new LILToStringVisitor();
            stringVisitor->setPrintHeadline(false);
            passes.push_back(stringVisitor);
        }

        //structure lowering
        auto structureLowerer = new LILStructureLowerer();
        structureLowerer->setDebug(d->debugStructureLowerer);
        passes.push_back(structureLowerer);
        if (verbose) {
            auto stringVisitor = new LILToStringVisitor();
            stringVisitor->setPrintHeadline(false);
            passes.push_back(stringVisitor);
        }
        
        //constant folding
        auto constantFolder = new LILConstantFolder();
        constantFolder->setDebug(d->debugConstantFolder);
        passes.push_back(constantFolder);
        if (verbose) {
            auto stringVisitor = new LILToStringVisitor();
            stringVisitor->setPrintHeadline(false);
            passes.push_back(stringVisitor);
        }

    } //end if not being imported
    
    //name lowering
    auto nameLowerer = new LILNameLowerer();
    nameLowerer->setDebug(d->debugNameLowerer);
    passes.push_back(nameLowerer);
    if (d->verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //execute the passes
    d->pm->execute(passes, d->astBuilder->getRootNode(), d->source);

    if (d->pm->hasErrors()) {
        std::cerr << "Errors encountered. Exiting.\n\n";
    }
    for (auto pass : passes) {
        delete pass;
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

void LILCodeUnit::setDebugConstantFolder(bool value)
{
    d->debugConstantFolder = value;
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
