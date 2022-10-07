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
#include "LILColorMaker.h"
#include "LILConfiguration.h"
#include "LILPreprocessor.h"
#include "LILArgResolver.h"
#include "LILASTValidator.h"
#include "LILClassTemplateLowerer.h"
#include "LILConfigGetter.h"
#include "LILConversionInserter.h"
#include "LILConstantFolder.h"
#include "LILEnumLowerer.h"
#include "LILFieldSorter.h"
#include "LILForLowerer.h"
#include "LILTypeValidator.h"
#include "LILMethodInserter.h"
#include "LILNameLowerer.h"
#include "LILObjDefExpander.h"
#include "LILParameterSorter.h"
#include "LILPassManager.h"
#include "LILPathExpander.h"
#include "LILResourceGatherer.h"
#include "LILStringFnLowerer.h"
#include "LILStructureLowerer.h"
#include "LILToStringVisitor.h"
#include "LILTypeGuesser.h"
#include "LILTypeResolver.h"
#include "LILDOMBuilder.h"

using namespace LIL;

extern void LILPrintErrors(const std::vector<LILErrorMessage> & errors, const LILString & code);

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
        , config(nullptr)
        , verbose(false)
        , debugStdLil(false)
        , importStdLil(false)
        , needsConfigureDefaults(true)
        , debugConfigureDefaults(false)
        , isBeingImportedWithNeeds(false)
        , isBeingImportedWithImport(false)
        {
        }
        LILString file;
        LILString dir;
        LILString compilerDir;
        LILString source;
        LILString suffix;
        LILString stdLilPath;
        std::unique_ptr<LILASTBuilder> astBuilder;
        std::unique_ptr<LILCodeParser> parser;
        std::unique_ptr<LILPassManager> pm;
        std::map<LILString, std::vector<std::shared_ptr<LILNode>>> _alreadyImportedFilesNeeds;
        std::map<LILString, std::vector<std::shared_ptr<LILNode>>> _alreadyImportedFilesImport;
        bool isMain;
        std::vector<LILString> arguments;
        std::vector<std::pair<LILString, bool>> neededFiles;
        std::vector<LILString> resources;
        std::vector<LILString> constants;
        std::vector<LILString> imports;
        std::shared_ptr<LILElement> dom;

        LILConfiguration * config;
        bool verbose;
        bool debugStdLil;
        bool importStdLil;
        bool needsConfigureDefaults;
        bool debugConfigureDefaults;
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

void LILCodeUnit::setCompilerDir(LILString dir)
{
    d->compilerDir = dir;
}

LILString LILCodeUnit::getCompilerDir() const
{
    return d->compilerDir;
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

void LILCodeUnit::setDebugConfigureDefaults(bool value)
{
    d->debugConfigureDefaults = value;
}

bool LILCodeUnit::getDebugConfigureDefaults() const
{
    return d->debugConfigureDefaults;
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

const std::vector<LILString> & LILCodeUnit::getArguments() const
{
    return d->arguments;
}

void LILCodeUnit::setArguments(const std::vector<LILString> & args)
{
    d->arguments = args;
}

void LILCodeUnit::setSuffix(const LILString & value)
{
    d->suffix = value;
}

void LILCodeUnit::setConstants(const std::vector<LILString> & values)
{
    d->constants = values;
}

void LILCodeUnit::setImports(const std::vector<LILString> & values)
{
    d->imports = values;
}

void LILCodeUnit::setConfiguration(LILConfiguration * value)
{
    d->config = value;
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
        bool previousVerboseValue = d->verbose;
        this->setVerbose(d->debugConfigureDefaults);
        if (d->debugConfigureDefaults) {
            std::cerr << "============================\n";
            std::cerr << "==== CONFIGURE DEFAULTS ====\n";
            std::cerr << "============================\n\n";
        }
        LILString path = this->getCompilerDir()+"/std/configure_defaults.lil";
        std::ifstream file(path.data(), std::ios::in);
        if (file.fail()) {
            std::cerr << "\nERROR: Failed to read the file "+path.data()+"\n\n";
        } else {
            std::stringstream buffer;
            buffer << file.rdbuf();
            const LILString & lilStr(buffer.str());
            d->parser->parseString(lilStr);
        }
        if (d->debugConfigureDefaults) {
            std::cerr << "\n\n";
        } else {
            for (const auto & conf : rootNode->getConfigure()) {
                conf->hidden = true;
            }
        }
        this->setVerbose(previousVerboseValue);
    }
    
    if (d->importStdLil) {
        auto importInstr = std::make_shared<LILInstruction>();
        LILNode::SourceLocation loc;
        loc.line = 0;
        loc.column = 0;
        importInstr->setSourceLocation(loc);
        importInstr->setInstructionType(InstructionTypeImport);
        importInstr->setName("import");
        auto strConst = std::make_shared<LILStringLiteral>();
        strConst->setValue(d->stdLilPath);
        importInstr->setArgument(strConst);
        importInstr->setVerbose(d->debugStdLil);
        importInstr->hidden = !d->debugStdLil;
        rootNode->add(importInstr);
    }
    
    for (auto importFile : d->imports) {
        auto importInstr = std::make_shared<LILInstruction>();
        LILNode::SourceLocation loc;
        loc.line = 0;
        loc.column = 0;
        importInstr->setSourceLocation(loc);
        importInstr->setInstructionType(InstructionTypeImport);
        importInstr->setName("import");
        auto strConst = std::make_shared<LILStringLiteral>();
        strConst->setValue(importFile);
        importInstr->setArgument(strConst);
        rootNode->add(importInstr);
    }
    
    for (auto constant : d->constants) {
        auto vd = std::make_shared<LILVarDecl>();
        LILNode::SourceLocation loc;
        loc.line = 0;
        loc.column = 0;
        vd->setSourceLocation(loc);
        vd->setName(constant);
        vd->setIsConst(true);
        auto boolVal = std::make_shared<LILBoolLiteral>();
        boolVal->setSourceLocation(loc);
        boolVal->setValue(true);
        
        auto boolTy = LILType::make("bool");
        boolVal->setType(boolTy);

        vd->setInitVal(boolVal);

        rootNode->add(vd);
    }

    if (d->verbose) {
        std::cerr << "============================\n";
        std::cerr << "===== PARSE MAIN FILE ======\n";
        std::cerr << "============================\n\n";
    }
    d->parser->parseString(d->source);

    if (d->astBuilder->hasErrors()) {
        LILPrintErrors(d->astBuilder->errors, d->source);
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
    
    //handle #arg instructions
    auto argResolver = new LILArgResolver();
    argResolver->setCustomArgs(d->arguments);
    passes.push_back(argResolver);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //handle #getConfig instructions
    auto configGetter = new LILConfigGetter();
    configGetter->setConfiguration(d->config);
    passes.push_back(configGetter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //handle color instructions
    auto colorMaker = new LILColorMaker();
    passes.push_back(colorMaker);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //handle #needs/#import, #if and #snippet/#paste instructions
    auto preprocessor = new LILPreprocessor();
    for (auto it = d->_alreadyImportedFilesNeeds.begin(); it != d->_alreadyImportedFilesNeeds.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, true);
    }
    for (auto it = d->_alreadyImportedFilesImport.begin(); it != d->_alreadyImportedFilesImport.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, false);
    }
    preprocessor->setDir(d->dir);
    preprocessor->setSuffix(d->suffix);
    preprocessor->setConstants(d->constants);
    preprocessor->setConfiguration(d->config);
    passes.push_back(preprocessor);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //ast validation
    auto astValidator = new LILASTValidator();
    passes.push_back(astValidator);

    //method inserter
    auto methodInserter = new LILMethodInserter();
    passes.push_back(methodInserter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //type resolving
    auto typeResolver = new LILTypeResolver();
    passes.push_back(typeResolver);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //class template lowerer
    auto classTemplateLowerer = new LILClassTemplateLowerer();
    passes.push_back(classTemplateLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //structure lowering
    auto structureLowerer = new LILStructureLowerer();
    passes.push_back(structureLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //build the DOM of static elements
    auto domBuilder = new LILDOMBuilder();
    passes.push_back(domBuilder);

    //type guessing
    auto typeGuesser = new LILTypeGuesser(domBuilder);
    passes.push_back(typeGuesser);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //path expander
    auto pathExpander = new LILPathExpander();
    passes.push_back(pathExpander);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //enum lowering
    auto enumLowerer = new LILEnumLowerer();
    passes.push_back(enumLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //string function lowering
    auto stringFnLowerer = new LILStringFnLowerer();
    passes.push_back(stringFnLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //for lowering
    auto forLowerer = new LILForLowerer();
    passes.push_back(forLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //field sorting
    auto fieldSorter = new LILFieldSorter();
    passes.push_back(fieldSorter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //parameter sorting
    auto parameterSorter = new LILParameterSorter();
    passes.push_back(parameterSorter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //conversion inserting
    auto convInserter = new LILConversionInserter();
    passes.push_back(convInserter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //constant folding
    auto constantFolder = new LILConstantFolder();
    passes.push_back(constantFolder);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //name lowering
    auto nameLowerer = new LILNameLowerer();
    passes.push_back(nameLowerer);
    if (d->verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //objDef expansion
    auto objDefExpander = new LILObjDefExpander();
    passes.push_back(objDefExpander);
    if (d->verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //type validation
    auto tyValidator = new LILTypeValidator();
    passes.push_back(tyValidator);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //resource gathering
    auto resourceGatherer = new LILResourceGatherer();
    passes.push_back(resourceGatherer);

    //execute the passes
    d->pm->execute(passes, d->astBuilder->getRootNode(), d->source);

    if (d->pm->hasErrors()) {
        std::cerr << "Errors encountered. Exiting.\n\n";
    } else {
        const auto & neededFiles = preprocessor->getNeededFilesForBuild();
        for (const auto & neededFile : neededFiles) {
            this->addNeededFileForBuild(neededFile.first, neededFile.second);
        }
        const auto & importedResources = preprocessor->getResources();
        for (const auto & resource : importedResources) {
            this->addResource(resource);
        }
        const auto & localResources = resourceGatherer->gatherResources();
        for (const auto & resource : localResources) {
            this->addResource(resource);
        }
        this->setDOM(domBuilder->getDOM());
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

    //handle #arg instructions
    auto argResolver = new LILArgResolver();
    argResolver->setCustomArgs(d->arguments);
    passes.push_back(argResolver);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //handle #getConfig instructions
    auto configGetter = new LILConfigGetter();
    configGetter->setConfiguration(d->config);
    passes.push_back(configGetter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //handle #needs/#import, #if and #snippet/#paste instructions
    auto preprocessor = new LILPreprocessor();
    for (auto it = d->_alreadyImportedFilesNeeds.begin(); it != d->_alreadyImportedFilesNeeds.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, true);
    }
    for (auto it = d->_alreadyImportedFilesImport.begin(); it != d->_alreadyImportedFilesImport.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, false);
    }
    preprocessor->setDir(d->dir);
    preprocessor->setSuffix(d->suffix);
    preprocessor->setConstants(d->constants);
    preprocessor->setConfiguration(d->config);
    passes.push_back(preprocessor);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //ast validation
    auto astValidator = new LILASTValidator();
    passes.push_back(astValidator);

    //method inserter
    auto methodInserter = new LILMethodInserter();
    passes.push_back(methodInserter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //type resolving
    auto typeResolver = new LILTypeResolver();
    passes.push_back(typeResolver);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //class template lowerer
    auto classTemplateLowerer = new LILClassTemplateLowerer();
    passes.push_back(classTemplateLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }
    
    //structure lowering
    auto structureLowerer = new LILStructureLowerer();
    passes.push_back(structureLowerer);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //build the DOM of static elements
    auto domBuilder = new LILDOMBuilder();
    passes.push_back(domBuilder);

    //type guessing
    auto typeGuesser = new LILTypeGuesser(domBuilder);
    passes.push_back(typeGuesser);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //field sorting
    auto fieldSorter = new LILFieldSorter();
    passes.push_back(fieldSorter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //parameter sorting
    auto parameterSorter = new LILParameterSorter();
    passes.push_back(parameterSorter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //name lowering
    auto nameLowerer = new LILNameLowerer();
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
    } else {
        const auto & neededFiles = preprocessor->getNeededFilesForBuild();
            for (const auto & neededFile : neededFiles) {
                this->addNeededFileForBuild(neededFile.first, neededFile.second);
            }
            const auto & resources = preprocessor->getResources();
            for (const auto & resource : resources) {
                this->addResource(resource);
            }
            this->setDOM(domBuilder->getDOM());
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

    //handle #arg instructions
    auto argResolver = new LILArgResolver();
    argResolver->setCustomArgs(d->arguments);
    passes.push_back(argResolver);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //handle #getConfig instructions
    auto configGetter = new LILConfigGetter();
    configGetter->setConfiguration(d->config);
    passes.push_back(configGetter);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //handle #needs/#import, #if and #snippet/#paste instructions
    auto preprocessor = new LILPreprocessor();
    for (auto it = d->_alreadyImportedFilesNeeds.begin(); it != d->_alreadyImportedFilesNeeds.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, true);
    }
    for (auto it = d->_alreadyImportedFilesImport.begin(); it != d->_alreadyImportedFilesImport.end(); ++it) {
        auto pair = *it;
        preprocessor->addAlreadyImportedFile(pair.first, pair.second, false);
    }
    preprocessor->setDir(d->dir);
    preprocessor->setSuffix(d->suffix);
    preprocessor->setConstants(d->constants);
    preprocessor->setConfiguration(d->config);
    passes.push_back(preprocessor);
    if (verbose) {
        auto stringVisitor = new LILToStringVisitor();
        stringVisitor->setPrintHeadline(false);
        passes.push_back(stringVisitor);
    }

    //ast validation
    auto astValidator = new LILASTValidator();
    passes.push_back(astValidator);

    //execute the passes
    d->pm->execute(passes, d->astBuilder->getRootNode(), d->source);

    const auto & neededFiles = preprocessor->getNeededFilesForBuild();
    for (const auto & neededFile : neededFiles) {
        this->addNeededFileForBuild(neededFile.first, neededFile.second);
    }
    const auto & resources = preprocessor->getResources();
    for (const auto & resource : resources) {
        this->addResource(resource);
    }

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

bool LILCodeUnit::getDebugStdLil() const
{
    return d->debugStdLil;
}

void LILCodeUnit::setDebugStdLil(bool value)
{
    d->debugStdLil = value;
}

bool LILCodeUnit::getImportStdLil() const
{
    return d->importStdLil;
}

void LILCodeUnit::setImportStdLil(bool value)
{
    d->importStdLil = value;
}

const LILString & LILCodeUnit::getStdLilPath() const
{
    return d->stdLilPath;
}

void LILCodeUnit::setStdLilPath(const LILString & value)
{
    d->stdLilPath = value;
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

void LILCodeUnit::addNeededFileForBuild(const LILString & path, bool verbose)
{
    std::pair<LILString, bool> item = { path, verbose };
    if (std::find(d->neededFiles.begin(), d->neededFiles.end(), item) == d->neededFiles.end()) {
        d->neededFiles.push_back( item );
    }
}
const std::vector<std::pair<LILString, bool>> & LILCodeUnit::getNeededFilesForBuild() const
{
    return d->neededFiles;
}

void LILCodeUnit::addResource(const LILString & path)
{
    if (std::find(d->resources.begin(), d->resources.end(), path) == d->resources.end()) {
        d->resources.push_back( path );
    }
}

const std::vector<LILString> & LILCodeUnit::getResources() const
{
    return d->resources;
}

const std::shared_ptr<LILElement> & LILCodeUnit::getDOM() const
{
    return d->dom;
}

void LILCodeUnit::setDOM(const std::shared_ptr<LILElement> & dom)
{
    d->dom = dom;
}
