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

#include "LILPreprocessor.h"
#include "LILASTValidator.h"
#include "LILClassTemplateLowerer.h"
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
        , debugPreprocessor(false)
        , debugAST(false)
        , debugASTValidator(false)
        , debugTypeResolver(false)
        , debugClassTemplateLowerer(false)
        , debugTypeGuesser(false)
        , debugStructureLowerer(false)
        , debugConstantFolder(false)
        , debugMethodInserter(false)
        , debugNameLowerer(false)
        , debugFieldSorter(false)
        , debugParameterSorter(false)
        , debugTypeValidator(false)
        , debugConversionInserter(false)
        , needsConfigureDefaults(true)
        , isBeingImportedWithNeeds(false)
        , isBeingImportedWithImport(false)
        {
        }
        LILString file;
        LILString dir;
        LILString source;
        std::unique_ptr<LILASTBuilder> astBuilder;
        std::unique_ptr<LILCodeParser> parser;
        std::unique_ptr<LILPassManager> pm;
        std::map<LILString, std::vector<std::shared_ptr<LILNode>>> _alreadyImportedFilesNeeds;
        std::map<LILString, std::vector<std::shared_ptr<LILNode>>> _alreadyImportedFilesImport;
        std::vector<LILString> customArgs;

        bool isMain;

        bool verbose;
        bool debugLilStd;
        bool debugPreprocessor;
        bool debugAST;
        bool debugASTValidator;
        bool debugTypeResolver;
        bool debugClassTemplateLowerer;
        bool debugTypeGuesser;
        bool debugStructureLowerer;
        bool debugConstantFolder;
        bool debugMethodInserter;
        bool debugNameLowerer;
        bool debugFieldSorter;
        bool debugParameterSorter;
        bool debugTypeValidator;
        bool debugConversionInserter;
        bool needsConfigureDefaults;
        bool isBeingImportedWithNeeds;
        bool isBeingImportedWithImport;
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

void LILCodeUnit::setNeedsConfigureDefaults(bool value)
{
    d->needsConfigureDefaults = value;
}

bool LILCodeUnit::getNeedsConfigureDefaults() const
{
    return d->needsConfigureDefaults;
}

void LILCodeUnit::setIsBeingImportedWithNeeds(bool value)
{
    d->isBeingImportedWithNeeds = value;
}

bool LILCodeUnit::getIsBeingImportedWithNeeds() const
{
    return d->isBeingImportedWithNeeds;
}

void LILCodeUnit::setIsBeingImportedWithImport(bool value)
{
    d->isBeingImportedWithImport = value;
}

bool LILCodeUnit::getIsBeingImportedWithImport() const
{
    return d->isBeingImportedWithImport;
}

const std::vector<LILString> & LILCodeUnit::getCustomArgs() const
{
    return d->customArgs;
}

void LILCodeUnit::setCustomArgs(std::vector<LILString> & args)
{
    d->customArgs = args;
}

void LILCodeUnit::run()
{
    bool verbose = d->verbose;

    d->pm->setVerbose(verbose);

    this->buildAST();

    if (d->astBuilder->hasErrors()) {
        return;
    }

    if (this->getIsBeingImportedWithNeeds()) {
        this->runPassesForNeeds();
    } else if (this->getIsBeingImportedWithImport()) {
        this->runPassesForImport();
    } else {
        this->runPasses();
    }
}

void LILCodeUnit::buildAST()
{
    auto rootNode = this->getRootNode();
    if (d->needsConfigureDefaults) {
        if (d->verbose) {
            std::cerr << "============================\n";
            std::cerr << "==== CONFIGURE DEFAULTS ====\n";
            std::cerr << "============================\n\n";
        }
        LILString path = this->getDir()+"/configure_defaults.lil";
        std::ifstream file(path.data(), std::ios::in);
        if (file.fail()) {
            std::cerr << "\nERROR: Failed to read the file "+path.data()+"\n\n";
        } else {
            std::stringstream buffer;
            buffer << file.rdbuf();
            const LILString & lilStr(buffer.str());
            d->parser->parseString(lilStr);
        }
        if (d->verbose) {
            std::cerr << "\n\n";
        }
    }

    if (d->verbose) {
        std::cerr << "============================\n";
        std::cerr << "===== PARSE MAIN FILE ======\n";
        std::cerr << "============================\n\n";
    }
    d->parser->parseString(d->source);

    if (d->isMain) {
        if (rootNode->hasInitializers()) {
            auto snippet = std::make_shared<LILSnippetInstruction>();
            snippet->setName("LIL_INITIALIZERS");
            for (auto initializer : rootNode->getInitializers()) {
                snippet->add(initializer);
            }
            rootNode->add(snippet);
        }
        auto importInstr = std::make_shared<LILInstruction>();
        importInstr->setInstructionType(InstructionTypeImport);
        importInstr->setName("import");
        auto pathToMain = std::make_shared<LILStringLiteral>();
        pathToMain->setValue("\"std/main.lil\"");
        importInstr->setArgument(pathToMain);
        rootNode->add(importInstr);
    }
}

void LILCodeUnit::runPasses()
{
    bool verbose = d->verbose;

    std::vector<LILVisitor *> passes;

    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        passes.push_back(stringVisitor);
    }

    //handle #needs, #if and #snippet instructions
    auto preprocessor = new LILPreprocessor();
    for (auto it = d->_alreadyImportedFilesNeeds.begin(); it != d->_alreadyImportedFilesNeeds.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, true);
    }
    for (auto it = d->_alreadyImportedFilesImport.begin(); it != d->_alreadyImportedFilesImport.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, false);
    }
    preprocessor->setDebug(d->debugPreprocessor);
    preprocessor->setDir(d->dir);
    preprocessor->setDebugAST(d->debugAST);
    preprocessor->setCustomArgs(d->customArgs);
    passes.push_back(preprocessor);
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

    //type resolving
    auto typeResolver = new LILTypeResolver();
    typeResolver->setDebug(d->debugTypeResolver);
    passes.push_back(typeResolver);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //class template lowerer
    auto classTemplateLowerer = new LILClassTemplateLowerer();
    classTemplateLowerer->setDebug(d->debugClassTemplateLowerer);
    passes.push_back(classTemplateLowerer);
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

    //conversion inserting
    auto convInserter = new LILConversionInserter();
    convInserter->setDebug(d->debugConversionInserter);
    passes.push_back(convInserter);
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

    //structure lowering
    auto structureLowerer = new LILStructureLowerer();
    structureLowerer->setDebug(d->debugStructureLowerer);
    passes.push_back(structureLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //name lowering
    auto nameLowerer = new LILNameLowerer();
    nameLowerer->setDebug(d->debugNameLowerer);
    passes.push_back(nameLowerer);
    if (d->verbose) {
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

    //execute the passes
    d->pm->execute(passes, d->astBuilder->getRootNode(), d->source);

    if (d->pm->hasErrors()) {
        std::cerr << "Errors encountered. Exiting.\n\n";
    }
    for (auto pass : passes) {
        delete pass;
    }
}

void LILCodeUnit::runPassesForNeeds()
{
    bool verbose = d->verbose;

    std::vector<LILVisitor *> passes;

    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        passes.push_back(stringVisitor);
    }

    //handle #needs, #if and #snippet instructions
    auto preprocessor = new LILPreprocessor();
    for (auto it = d->_alreadyImportedFilesNeeds.begin(); it != d->_alreadyImportedFilesNeeds.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, true);
    }
    for (auto it = d->_alreadyImportedFilesImport.begin(); it != d->_alreadyImportedFilesImport.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, false);
    }
    preprocessor->setDebug(d->debugPreprocessor);
    preprocessor->setDir(d->dir);
    preprocessor->setDebugAST(d->debugAST);
    passes.push_back(preprocessor);
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

    //type resolving
    auto typeResolver = new LILTypeResolver();
    typeResolver->setDebug(d->debugTypeResolver);
    passes.push_back(typeResolver);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //class template lowerer
    auto classTemplateLowerer = new LILClassTemplateLowerer();
    classTemplateLowerer->setDebug(d->debugClassTemplateLowerer);
    passes.push_back(classTemplateLowerer);
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

    //structure lowering
    auto structureLowerer = new LILStructureLowerer();
    structureLowerer->setDebug(d->debugStructureLowerer);
    passes.push_back(structureLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //name lowering
    auto nameLowerer = new LILNameLowerer();
    nameLowerer->setDebug(d->debugNameLowerer);
    passes.push_back(nameLowerer);
    if (d->verbose) {
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

    //execute the passes
    d->pm->execute(passes, d->astBuilder->getRootNode(), d->source);

    if (d->pm->hasErrors()) {
        std::cerr << "Errors encountered. Exiting.\n\n";
    }
    for (auto pass : passes) {
        delete pass;
    }
}

void LILCodeUnit::runPassesForImport()
{
    bool verbose = d->verbose;

    std::vector<LILVisitor *> passes;

    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        passes.push_back(stringVisitor);
    }

    //handle #needs, #if and #snippet instructions
    auto preprocessor = new LILPreprocessor();
    for (auto it = d->_alreadyImportedFilesNeeds.begin(); it != d->_alreadyImportedFilesNeeds.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, true);
    }
    for (auto it = d->_alreadyImportedFilesImport.begin(); it != d->_alreadyImportedFilesImport.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, false);
    }
    preprocessor->setDebug(d->debugPreprocessor);
    preprocessor->setDir(d->dir);
    preprocessor->setDebugAST(d->debugAST);
    passes.push_back(preprocessor);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //ast validation
    auto astValidator = new LILASTValidator();
    astValidator->setDebug(d->debugASTValidator);
    passes.push_back(astValidator);

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

void LILCodeUnit::setDebugPreprocessor(bool value)
{
    d->debugPreprocessor = value;
}

void LILCodeUnit::setDebugASTValidator(bool value)
{
    d->debugASTValidator = value;
}

void LILCodeUnit::setDebugTypeResolver(bool value)
{
    d->debugTypeResolver = value;
}

void LILCodeUnit::setDebugClassTemplateLowerer(bool value)
{
    d->debugClassTemplateLowerer = value;
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

void LILCodeUnit::addAlreadyImportedFile(const LILString & path, const std::vector<std::shared_ptr<LILNode>> & nodes, bool isNeeds)
{
    std::vector<std::shared_ptr<LILNode>> data;
    for (auto node : nodes) {
        auto clone = node->clone();
        clone->setIsExported(false);
        data.push_back(clone);
    }
    if (isNeeds) {
        d->_alreadyImportedFilesNeeds[path] = data;
    } else {
        d->_alreadyImportedFilesImport[path] = data;
    }
}

bool LILCodeUnit::isAlreadyImported(const LILString & path, bool isNeeds)
{
    if (isNeeds) {
        return d->_alreadyImportedFilesNeeds.count(path) > 0;
    } else {
        return d->_alreadyImportedFilesImport.count(path) > 0;
    }
}
