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
 *      This file is responsible for coordinating a build
 *
 ********************************************************************/

#include "LILBuildManager.h"
#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILCodeUnit.h"
#include "LILConfiguration.h"
#include "LILDocumentation.h"
#include "LILDocumentationWriter.h"
#include "LILDocumentationTmplManager.h"
#include "LILErrorMessage.h"
#include "LILNumberLiteral.h"
#include "LILOutputEmitter.h"
#include "LILPlatformSupport.h"
#include "LILRule.h"
#include "LILRootNode.h"
#include "LILSelector.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILValueList.h"
#include "LILVarName.h"

#include <sys/stat.h>
#include <array>

using namespace LIL;

extern void LILPrintErrors(const std::vector<LILErrorMessage> & errors, const LILString & code);

LILBuildManager::LILBuildManager()
: _codeUnit(nullptr)
, _config(std::make_unique<LILConfiguration>())
, _hasErrors(false)
, _debug(false)
, _verbose(false)
, _noConfigureDefaults(false)
, _debugConfigureDefaults(false)
, _compileToS(false)
, _warningLevel(0)
{
    
}

inline bool file_exists (const std::string & name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

LILBuildManager::~LILBuildManager()
{
    
}

void LILBuildManager::read()
{
    std::string filePath = this->_file.data();
    if (filePath.substr(0, 1) != "/") {
        filePath = this->_directory.data() + "/" + filePath;
    }
    std::ifstream file(filePath, std::ios::in);
    if (file.fail()) {
        LILErrorMessage ei;
        ei.message = "\nERROR: Failed to read the file "+filePath;
        ei.file = filePath;
        ei.line = 0;
        ei.column = 0;
        this->_errors.push_back(ei);
        this->_hasErrors = true;
        
        LILPrintErrors(this->_errors, "");
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    LILString lilStr(buffer.str());

    this->_codeUnit = std::make_unique<LILCodeUnit>();
    this->_codeUnit->setVerbose(this->_debugConfigureDefaults);
    this->_codeUnit->setNeedsConfigureDefaults(!this->_noConfigureDefaults);
    this->_codeUnit->setDebugConfigureDefaults(this->_debugConfigureDefaults);
    this->_codeUnit->setIsBeingImportedWithImport(true);

    this->_codeUnit->setArguments(this->_arguments);

    this->_codeUnit->setFile(this->_file);
    std::vector<std::shared_ptr<LILNode>> emptyVect;
    this->_codeUnit->addAlreadyImportedFile(filePath, emptyVect, true);
    this->_codeUnit->addAlreadyImportedFile(filePath, emptyVect, false);
    this->_codeUnit->setDir(this->_directory);
    this->_codeUnit->setCompilerDir(this->_compilerDir);

    this->_codeUnit->setSource(lilStr);

    this->_codeUnit->run();
    if (this->_codeUnit->hasErrors()) {
        this->_hasErrors = true;
    }
}

void LILBuildManager::configure()
{
    const auto & config = this->_codeUnit->getRootNode()->getConfigure();
    std::unordered_map<std::string, std::vector<std::shared_ptr<LILRule>>> builds;
    std::unordered_map<std::string, std::vector<std::shared_ptr<LILRule>>> targets;
    for (const auto & conf : config) {
        if (conf->isA(InstructionTypeConfigure)) {
            const auto & nodes = conf->getChildNodes();
            for (const auto & node : nodes) {
                if (node->isA(NodeTypeRule)) {
                    auto rule = std::static_pointer_cast<LILRule>(node);
                    const auto & firstSel = rule->getFirstSelector();
                    if (firstSel && firstSel->isA(NodeTypeSelector)) {
                        auto sel = std::static_pointer_cast<LILSelector>(firstSel);
                        const auto & selName = sel->getName();
                        if (selName == "builds" || selName == "targets") {
                            for (const auto & childRule : rule->getChildRules()) {
                                const auto & firstChildSel = childRule->getFirstSelector();
                                if (firstChildSel) {
                                    switch (firstChildSel->getSelectorType()) {
                                        case SelectorTypeUniversalSelector:
                                        {
                                            for (const auto & node : childRule->getValues()) {
                                                this->_config->applyConfig(node);
                                            }
                                            break;
                                        }
                                        case SelectorTypeNameSelector:
                                        {
                                            auto childSel = std::static_pointer_cast<LILSelector>(firstChildSel);
                                            if (selName == "builds") {
                                                builds[childSel->getName().data()].push_back(childRule);
                                            } else {
                                                targets[childSel->getName().data()].push_back(childRule);
                                            }
                                            break;
                                        }
                                        default:
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
                else if (node->isA(NodeTypeAssignment))
                {
                    this->_config->applyConfig(node);
                }
                else if (node->isA(NodeTypeUnaryExpression))
                {
                    this->_config->applyConfig(node);
                }
            }
        }
    }
    auto build = this->_config->getConfigString("build");
    if (builds.count(build) > 0) {
        const auto & rules = builds[build];
        for (const auto & rule : rules) {
            for (const auto & node : rule->getValues()) {
                this->_config->applyConfig(node);
            }
        }
    }
    
    auto target = this->_config->getConfigString("target");
    if (target == "auto" || target == "") {
        target = LIL_getAutoTargetString();
    }
    std::shared_ptr<LILStringLiteral> targetStr = std::make_shared<LILStringLiteral>();
    targetStr->setValue(target);
    this->_config->setConfig("target", targetStr);
    if (targets.count(target) > 0) {
        const auto & targetRules = targets[target];
        for (const auto & targetRule : targetRules) {
            for (const auto & node : targetRule->getValues()) {
                this->_config->applyConfig(node);
            }
        }
    }
    
    auto isAppStr = this->_config->getConfigString("isApp");
    if (isAppStr == "auto") {
        std::shared_ptr<LILBoolLiteral> isAppBool = std::make_shared<LILBoolLiteral>();
        //if we have rules or main menu, build as an app
        bool isApp = (this->_codeUnit->getRootNode()->getRules().size() > 0) || (this->_codeUnit->getRootNode()->hasMainMenu());
        isAppBool->setValue(isApp);
        this->_config->setConfig("isApp", isAppBool);
    }
    
    auto formatStr = this->_config->getConfigString("format");
    if (formatStr == "llvm" || formatStr == "s") {
        this->_compileToS = true;
    }
    
    
    size_t dotIndex = this->_file.data().find_last_of(".");
    std::string outName;
    if (dotIndex != std::string::npos) {
        outName = this->_file.data().substr(0, dotIndex);
    } else {
        outName = this->_file.data();
    }
    std::shared_ptr<LILStringLiteral> strLit = std::make_shared<LILStringLiteral>();
    strLit->setValue(outName);
    this->_config->setConfig("outName", strLit);
    auto outStr = this->_config->getConfigString("out");
    if (outStr.length() == 0) {
        
        this->_config->setConfig("out", strLit);
    }

    std::shared_ptr<LILStringLiteral> dirStrLit = std::make_shared<LILStringLiteral>();
    dirStrLit->setValue(this->_directory);
    this->_config->setConfig("directory", dirStrLit);

    if (this->_verbose) {
        std::cerr << "============================\n";
        std::cerr << "==== USED CONFIGURATION ====\n";
        std::cerr << "============================\n\n";

        this->_config->printConfig();
        std::cerr << "\n\n";
    }
}

void LILBuildManager::build()
{
    std::string suffix = this->_config->getConfigString("suffix");
    bool isApp = this->_config->getConfigBool("isApp");
    std::string out = this->_config->getConfigString("out");
    std::string outName = this->_config->getConfigString("outName");
    std::string exeExt = this->_config->getConfigString("exeExt");
    std::string objExt = this->_config->getConfigString("objExt");
    std::string buildPath = this->_config->getConfigString("buildPath");
    if (buildPath.substr(0, 1) != "/") {
        buildPath = this->_currentWorkingDir.data() + "/" + buildPath;
    }

    LIL_makeDir(buildPath);
    
    bool needsCompile = this->_config->getConfigBool("compile", true);
    auto docConfigStr = this->_config->getConfigString("documentation");
    bool needsDocs;
    if (docConfigStr.length() > 0) {
        needsDocs = true;
    } else {
        needsDocs = this->_config->getConfigBool("documentation");
    }

    if (needsCompile || needsDocs) {
        std::string filePath;
        if (this->_file.data().substr(0, 1) == "/") {
            filePath = this->_file.data();
        } else {
            filePath = this->_directory.data() + "/" + this->_file.data();
        }
        std::ifstream file(filePath, std::ios::in);
        if (file.fail()) {
            LILErrorMessage ei;
            ei.message = "\nERROR: Failed to read the file "+filePath;
            ei.file = filePath;
            ei.line = 0;
            ei.column = 0;
            this->_errors.push_back(ei);
            this->_hasErrors = true;
            
            LILPrintErrors(this->_errors, "");
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        
        LILString lilStr(buffer.str());
        
        auto mainCodeUnit = std::make_unique<LILCodeUnit>();
        mainCodeUnit->setVerbose(this->_verbose);
        mainCodeUnit->setImportStdLil(this->_config->getConfigBool("importStdLil"));
        mainCodeUnit->setStdLilPath(this->_config->getConfigString("stdLilPath"));
        mainCodeUnit->setDebugStdLil(this->_config->getConfigBool("debugStdLil"));
        mainCodeUnit->setNeedsConfigureDefaults(false);
        mainCodeUnit->setIsMain(this->_config->getConfigBool("isMain"));

        std::vector<LILString> constants;
        for (auto imp : this->_config->getConfigItems("constants")) {
            auto tmp = this->_config->extractString(imp);
            if (tmp.length() > 0) {
                constants.push_back(tmp);
            }
        }
        mainCodeUnit->setConstants(constants);
        std::vector<LILString> imports;
        for (auto imp : this->_config->getConfigItems("imports")) {
            auto tmp = this->_config->extractString(imp);
            if (tmp.length() > 0) {
                imports.push_back(tmp);
            }
        }
        mainCodeUnit->setImports(imports);
        mainCodeUnit->setArguments(this->_arguments);
        mainCodeUnit->setConfiguration(this->_config.get());
        
        mainCodeUnit->setFile(this->_file);
        std::vector<std::shared_ptr<LILNode>> emptyVect;
        auto fullPath = this->_directory+"/"+this->_file;
        mainCodeUnit->addAlreadyImportedFile(fullPath, emptyVect, true);
        mainCodeUnit->addAlreadyImportedFile(fullPath, emptyVect, false);
        mainCodeUnit->setDir(this->_directory);
        mainCodeUnit->setCompilerDir(this->_compilerDir);
        mainCodeUnit->setSuffix(suffix);
        
        mainCodeUnit->setSource(lilStr);
        
        mainCodeUnit->run();
        if (mainCodeUnit->hasErrors()) {
            this->_hasErrors = true;
            return;
        }
        
        if (needsDocs)
        {
            std::string oDir = buildPath+"/docs";
            LIL_makeDir(oDir);
            
            auto tmplManager = std::make_unique<LILDocumentationTmplManager>();
            tmplManager->setTemplatePath(this->_config->getConfigString("docTemplatesPath"));
            
            std::unique_ptr<LILDocumentationWriter> docWriter = std::make_unique<LILDocumentationWriter>();
            docWriter->setTemplateManager(tmplManager.get());
            docWriter->initializeVisit();

            if (docConfigStr.length() == 0) {
                for (const auto & doc : mainCodeUnit->getRootNode()->getDocs()) {
                    auto str = doc->getContent().data();
                    docWriter->setValue(doc);
                    docWriter->performVisit(mainCodeUnit->getRootNode());
                    std::string outFilePath;
                    if (str.substr(0, 5) == "class") {
                        outFilePath = oDir+"/class_"+str.substr(7, std::string::npos)+".html";
                    } else if (str.substr(0, 2) == "fn") {
                        outFilePath = oDir+"/fn_"+str.substr(3, std::string::npos)+".html";
                    } else {
                        std::cout << "!! UNKNOWN DOC TYPE FAIL !!!!!!\n";
                        return;
                    }
                    std::ofstream outFile(outFilePath, std::ios::out);
                    const std::string & result = docWriter->getResult();
                    outFile << result;
                    outFile.close();
                }
            } else {
                auto rootNode = mainCodeUnit->getRootNode();
                std::string outFilePath;
                std::string result;
                if (docConfigStr.substr(0, 5) == "class") {
                    std::string className = docConfigStr.substr(7, std::string::npos);
                    auto classDecl = rootNode->findClassWithName(className);
                    if (!classDecl) {
                        std::cout << "!! CLASS NOT FOUND FAIL !!!!!!\n";
                        return;
                    }
                    result = docWriter->createBoilerplateClass(classDecl.get(), rootNode.get());
                    outFilePath = oDir+"/"+docConfigStr.substr(7, std::string::npos)+".doc.lil";
                } else if (docConfigStr.substr(0, 2) == "fn") {
                    std::string fnName = docConfigStr.substr(3, std::string::npos);
                    auto functionDeclNode = rootNode->getLocalVariable(fnName);
                    if (functionDeclNode->getNodeType() == NodeTypeFunctionDecl) {
                        auto fd = std::static_pointer_cast<LILFunctionDecl>(functionDeclNode);
                        result = docWriter->createBoilerplateFn(fd.get(), rootNode.get());
                        outFilePath = oDir+"/"+fnName+".doc.lil";
                    }
                }

                std::ofstream outFile(outFilePath, std::ios::out);
                outFile << result;
                outFile.close();
            }
        }
        else
        {
            std::unique_ptr<LILOutputEmitter> outEmitter = std::make_unique<LILOutputEmitter>();
            outEmitter->setVerbose(this->_verbose);
            outEmitter->setDebugIREmitter(this->_debug);
            LILString oFile = outName;
            oFile += (this->_compileToS ? ".s" : this->_config->getConfigString("objExt") );

            outEmitter->setInFile(this->_file);
            outEmitter->setOutFile(oFile);
            outEmitter->setDir(buildPath);

            outEmitter->setCPU(this->_config->getConfigString("cpu"));
            outEmitter->setVendor(this->_config->getConfigString("vendor"));

            //instantiate the IREmitter
            outEmitter->prepare();
            outEmitter->setDOM(mainCodeUnit->getDOM());

            if (this->_config->getConfigBool("printOnly")) {
                outEmitter->printToOutput(mainCodeUnit->getRootNode());
            } else {
                if (this->_compileToS) {
                    outEmitter->compileToS(mainCodeUnit->getRootNode());
                } else {
                    outEmitter->compileToO(mainCodeUnit->getRootNode());
                }
            }
        }
        
        if (!this->_config->getConfigBool("singleFile")) {
            std::vector<std::string> linkFiles;

            for (const auto & filePair : mainCodeUnit->getNeededFilesForBuild()) {
                std::string fileDirAndName;
                std::string fileNameExt;
                std::string fileName;
                std::string fileDir;
                const std::string & fileStr = filePair.first.data();
                bool fileIsVerbose = filePair.second;
                
                //remove the path of the main file from this one to see the relative subdir
                size_t dirIndex = fileStr.find(this->_directory.data());
                if (dirIndex != std::string::npos) {
                    fileDirAndName = fileStr.substr(this->_directory.length()+1);
                } else {
                    fileDirAndName = fileStr;
                }
                
                if (this->_verbose) {
                    std::cerr << "Compiling " << fileDirAndName << "\n";
                }

                size_t slashIndex = fileDirAndName.find_last_of("/");
                if (slashIndex != std::string::npos) {
                    fileDir = fileDirAndName.substr(0, slashIndex);
                    fileNameExt = fileDirAndName.substr(slashIndex+1);
                } else {
                    fileDir = "";
                    fileNameExt = fileDirAndName;
                }
                size_t dotIndex = fileNameExt.find_last_of(".");
                if (dotIndex != std::string::npos) {
                    fileName = fileNameExt.substr(0, dotIndex);
                } else {
                    fileName = fileNameExt;
                }

                std::string fpath = fileStr;
                if (suffix.length() > 0) {
                    size_t extensionIndex = fileStr.find_last_of(".");
                    
                    if (extensionIndex != std::string::npos) {
                        std::string spath = fpath.substr(0, extensionIndex) + "@" + suffix + fpath.substr(extensionIndex, fpath.length() - extensionIndex);
                        fpath = spath;
                    }
                }
                std::ifstream file(fpath, std::ios::in);
                if (file.fail()) {
                    file = std::ifstream(fileStr.data(), std::ios::in);
                }
                if (file.fail()) {
                    LILErrorMessage ei;
                    ei.message = "\nERROR: Failed to read the file "+fileStr;
                    ei.file = fileStr;
                    ei.line = 0;
                    ei.column = 0;
                    this->_errors.push_back(ei);
                    this->_hasErrors = true;

                    LILPrintErrors(this->_errors, "");
                    return;
                }
                
                std::stringstream buffer;
                buffer << file.rdbuf();
                
                LILString lilStr(buffer.str());
                
                auto codeUnit = std::make_unique<LILCodeUnit>();
                codeUnit->setVerbose(fileIsVerbose);
                codeUnit->setNeedsConfigureDefaults(false);
                codeUnit->setIsMain(false);
                
                codeUnit->setConstants(constants);
                codeUnit->setArguments(this->_arguments);
                codeUnit->setConfiguration(this->_config.get());
                codeUnit->setSuffix(this->_config->getConfigString("suffix"));
                
                codeUnit->setFile(fileNameExt);
                std::vector<std::shared_ptr<LILNode>> emptyVect;
                codeUnit->addAlreadyImportedFile(fileStr, emptyVect, true);
                codeUnit->addAlreadyImportedFile(fileStr, emptyVect, false);
                if (fileDir.length() > 0) {
                    codeUnit->setDir(fileDir);
                } else {
                    codeUnit->setDir(this->_directory);
                }
                codeUnit->setCompilerDir(this->_compilerDir);
                
                codeUnit->setSource(lilStr);
                
                codeUnit->run();

                if (!needsDocs) {
                    std::unique_ptr<LILOutputEmitter> outEmitter = std::make_unique<LILOutputEmitter>();
                    outEmitter->setVerbose(fileIsVerbose);
                    outEmitter->setDebugIREmitter(this->_debug);
                    std::string oFile = fileName+this->_config->getConfigString("objExt");
                    std::string oDir = buildPath+"/"+fileDir;
                    outEmitter->setInFile(fileNameExt);
                    outEmitter->setOutFile(oFile);
                    outEmitter->setDir(oDir);
                    outEmitter->setCPU(this->_config->getConfigString("cpu"));
                    outEmitter->setVendor(this->_config->getConfigString("vendor"));

                    //instantiate the IREmitter
                    outEmitter->prepare();
                    outEmitter->setDOM(codeUnit->getDOM());

                    LIL_makeDir(oDir);
                    
                    if (this->_config->getConfigBool("printOnly")) {
                        outEmitter->printToOutput(codeUnit->getRootNode());
                    } else {
                        outEmitter->compileToO(codeUnit->getRootNode());
                    }
                    std::string linkFileStr = oDir + "/" + oFile;
                    if (std::find(linkFiles.begin(), linkFiles.end(), linkFileStr) == linkFiles.end()) {
                        linkFiles.push_back(linkFileStr);
                    }
                }
            } //for
            
            if (needsDocs) {
                return;
            }
            
            std::string outFileName = buildPath + "/" + out;
            
            if (isApp && this->_config->getConfigBool("buildResources")) {
                if (this->_verbose) {
                    std::cerr << "\n============================" << "\n";
                    std::cerr << "==== BUILDING RESOURCES ====" << "\n";
                    std::cerr << "============================" << "\n";
                }

                std::vector<std::string> steps;
                for (const auto & buildStep : this->_config->getConfigItems("resourceBuildSteps")) {
                    auto tmp = this->_config->extractString(buildStep);
                    if (tmp.length() > 0) {
                        steps.push_back(tmp);
                    }
                }
                for (const auto & step : steps) {
                    if (this->_verbose) {
                        std::cerr << step << "\n\n";
                    }
                    
                    std::string stepCommand = step;
                    stepCommand.append(" 2>&1");
                    std::unique_ptr<FILE, decltype(&pclose)> stepPipe(popen(stepCommand.c_str(), "r"), pclose);
                    if (!stepPipe) {
                        std::cerr << "ERROR: popen() failed while trying to execute resource build step! \n";
                        return;
                    }
                    std::array<char, 128> stepBuffer;
                    std::string stepResult;
                    while (fgets(stepBuffer.data(), stepBuffer.size(), stepPipe.get()) != nullptr) {
                        stepResult += stepBuffer.data();
                    }
                    if (stepResult.length() > 0) {
                        std::cerr << stepResult;
                    }
                }
            }
            
            if (this->_config->getConfigBool("link")) {
#if defined(_WIN32)
                std::string linkCommand = "LINK \"" + buildPath + "/" + outName + objExt + "\"";
#else
                std::string linkCommand = "ld \"" + buildPath + "/" + outName + objExt + "\"";
#endif

                for (const auto & linkFile : linkFiles) {
                    linkCommand += " \"" + linkFile + "\"";
                }
                
                if (isApp) {
                    std::vector<std::string> flags;
                    for (const auto & linkerFlag : this->_config->getConfigItems("linkerFlagsApp")) {
                        auto tmp = this->_config->extractString(linkerFlag);
                        if (tmp.length() > 0) {
                            flags.push_back(tmp);
                        }
                    }
                    for (const auto & flag : flags) {
                        linkCommand += " " + flag;
                    }
                } else {
                    std::vector<std::string> flags;
                    for (const auto & linkerFlag : this->_config->getConfigItems("linkerFlags")) {
                        auto tmp = this->_config->extractString(linkerFlag);
                        if (tmp.length() > 0) {
                            flags.push_back(tmp);
                        }
                    }
                    for (const auto & flag : flags) {
                        linkCommand += " " + flag;
                    }
                }

                linkCommand += " -o \"" + outFileName;
#if defined(_WIN32)
                linkCommand += ".exe";
#endif
                linkCommand += "\"";

                if (this->_verbose) {
                    std::cerr << "\n============================" << "\n";
                    std::cerr << "========= LINKING ==========" << "\n";
                    std::cerr << "============================" << "\n";
                    std::cerr << linkCommand << "\n\n";
                }
                std::array<char, 128> linkBuffer;
                std::string linkResult;
                linkCommand.append(" 2>&1");
                std::unique_ptr<FILE, decltype(&pclose)> linkPipe(popen(linkCommand.c_str(), "r"), pclose);
                if (!linkPipe) {
                    std::cerr << "popen() to the linker failed!";
                    return;
                }
                while (fgets(linkBuffer.data(), linkBuffer.size(), linkPipe.get()) != nullptr) {
                    linkResult += linkBuffer.data();
                }
                if (linkResult.length() > 0) {
                    std::cerr << linkResult;
                }
                if (!file_exists(outFileName)) {
                    std::cerr << "\nThere was an error while linking. Exiting.\n\n";
                    return;
                }

                if (isApp) {
                    if (this->_verbose) {
                        std::cerr << "\n============================" << "\n";
                        std::cerr << "======== PACKAGING =========" << "\n";
                        std::cerr << "============================" << "\n";
                    }

                    std::vector<std::string> steps;
                    for (const auto & buildStep : this->_config->getConfigItems("appBuildSteps")) {
                        auto tmp = this->_config->extractString(buildStep);
                        if (tmp.length() > 0) {
                            steps.push_back(tmp);
                        }
                    }
                    for (const auto & step : steps) {
                        if (this->_verbose) {
                            std::cerr << step << "\n\n";
                        }
                        
                        std::array<char, 128> stepBuffer;
                        std::string stepResult;
                        std::string stepCommand = step;
                        stepCommand.append(" 2>&1");
                        std::unique_ptr<FILE, decltype(&pclose)> stepPipe(popen(stepCommand.c_str(), "r"), pclose);
                        if (!stepPipe) {
                            std::cerr << "ERROR: popen() failed while trying to execute build step! \n";
                            return;
                        }
                        while (fgets(stepBuffer.data(), stepBuffer.size(), stepPipe.get()) != nullptr) {
                            stepResult += stepBuffer.data();
                        }
                        if (stepResult.length() > 0) {
                            std::cerr << stepResult;
                            std::cerr << "\nThere was an error while executing a build step. Exiting.\n\n";
                            return;
                        }
                    }
                    if (this->_config->getConfigBool("copyResources")) {
                        if (this->_verbose) {
                            std::cerr << "\n============================" << "\n";
                            std::cerr << "==== COPYING RESOURCES =====" << "\n";
                            std::cerr << "============================" << "\n";
                        }
                        auto resourcesPath = this->_config->getConfigString("resourcesPath");

                        for (auto resFile : mainCodeUnit->getResources()) {
                            std::array<char, 128> copyResBuffer;
                            std::string copyResResult;
                            std::string sourcePath;
                            if (resFile.substr(0, 1) == "/") {
                                sourcePath = resFile.data();
                            } else {
                                sourcePath = this->_directory.data() + "/" + resFile.data();
                            }
                            std::string copyResCommand = "cp '"+sourcePath+"' '"+resourcesPath.data()+resFile.data()+"'";
                            if (this->_verbose) {
                                std::cerr << copyResCommand << "\n";
                            }
                            copyResCommand.append(" 2>&1");
                            std::unique_ptr<FILE, decltype(&pclose)> copyResPipe(popen(copyResCommand.c_str(), "r"), pclose);
                            if (!copyResPipe) {
                                std::cerr << "ERROR: popen() failed while trying copy resource! \n";
                                return;
                            }
                            while (fgets(copyResBuffer.data(), copyResBuffer.size(), copyResPipe.get()) != nullptr) {
                                copyResResult += copyResBuffer.data();
                            }
                            if (copyResResult.length() > 0) {
                                std::cerr << copyResResult;
                            }
                        }
                    }
                }
                
                if (this->_config->getConfigBool("run")) {
                    if (this->_verbose) {
                        std::cerr << "\n============================" << "\n";
                        std::cerr << "======== RUNNING =========" << "\n";
                        std::cerr << "============================" << "\n";
                    }
                    std::string runCommand = "";
                    if (isApp) {
                        runCommand = this->_config->getConfigString("runCommandApp");
                    } else {
                        runCommand = this->_config->getConfigString("runCommand");
                    }
                    if (this->_verbose) {
                        std::cerr << runCommand << "\n\n";
                    }
                    std::array<char, 128> runBuffer;
                    runCommand.append(" 2>&1");
                    
                    std::unique_ptr<FILE, decltype(&pclose)> runPipe(popen(runCommand.c_str(), "r"), pclose);
                    if (!runPipe) {
                        std::cerr << "popen() to the executable failed!";
                        return;
                    }
                    while (fgets(runBuffer.data(), runBuffer.size(), runPipe.get()) != nullptr) {
                        std::cerr << runBuffer.data();
                    }
                }
            }
        }
    }
}

bool LILBuildManager::hasErrors() const
{
    return this->_hasErrors;
}

void LILBuildManager::setDirectory(LILString value)
{
    this->_directory = value;
}

void LILBuildManager::setFile(LILString value)
{
    this->_file = value;
}

void LILBuildManager::setCompilerDir(LILString value)
{
    this->_compilerDir = value;
    auto strLit = std::make_shared<LILStringLiteral>();
    strLit->setValue(value);
    this->_config->setConfig("compilerDir", strLit);
}

void LILBuildManager::setCurrentWorkingDir(LILString value)
{
    this->_currentWorkingDir = value;
    auto strLit = std::make_shared<LILStringLiteral>();
    strLit->setValue(value);
    this->_config->setConfig("currentWorkingDir", strLit);
}

void LILBuildManager::setVerbose(bool value)
{
    this->_verbose = value;
}

void LILBuildManager::setNoConfigureDefaults(bool value)
{
    this->_noConfigureDefaults = value;
}

void LILBuildManager::setDebugConfigureDefaults(bool value)
{
    this->_debugConfigureDefaults = value;
}

void LILBuildManager::setWarningLevel(int value)
{
    this->_warningLevel = value;
}

void LILBuildManager::setArguments(std::vector<LILString> &&args)
{
    this->_arguments = std::move(args);
}
