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
 *      This file handles the preprocessing of the AST
 *
 ********************************************************************/

#include <glob.h>

#include "LILPreprocessor.h"
#include "LILASTBuilder.h"
#include "LILAliasDecl.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCodeParser.h"
#include "LILCodeUnit.h"
#include "LILErrorMessage.h"
#include "LILFunctionDecl.h"
#include "LILInstruction.h"
#include "LILNodeToString.h"
#include "LILObjectType.h"
#include "LILRootNode.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILStringLiteral.h"
#include "LILTypeDecl.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

using namespace LIL;

LILPreprocessor::LILPreprocessor()
: _debugAST(false)
, _needsAnotherPass(false)
{
}

LILPreprocessor::~LILPreprocessor()
{
}

void LILPreprocessor::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  #PREPROCESSING  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILPreprocessor::visit(LILNode *node)
{
    
}

void LILPreprocessor::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    this->_needsAnotherPass = true;
    while (this->_needsAnotherPass) {
        this->_needsAnotherPass = false;
        if (this->hasErrors()) {
            return;
        }
        this->processImportingInstr(rootNode);
        if (this->hasErrors()) {
            return;
        }
        this->processIfInstr(rootNode);
        if (this->hasErrors()) {
            return;
        }
        this->processPasteInstr(rootNode);
        if (this->hasErrors()) {
            return;
        }
    }
    this->removeSnippets(rootNode);
}

void LILPreprocessor::processImportingInstr(const std::shared_ptr<LILRootNode> & rootNode)
{
    const std::vector<std::shared_ptr<LILNode>> & nodes = rootNode->getNodes();
    auto it = nodes.begin();
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    while (it != nodes.end()) {
        auto node = *it;
        if (!node->isA(InstructionTypeNeeds) && !node->isA(InstructionTypeImport)) {
            resultNodes.push_back(node);
        } else {
            std::vector<std::shared_ptr<LILNode>> newNodes;
            bool isNeeds = node->isA(InstructionTypeNeeds);
            if (this->getDebug()) {
                std::cerr << "##  importing " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
            }
            auto instr = std::static_pointer_cast<LILInstruction>(node);
            auto arg = instr->getArgument();
            if (!arg->isA(NodeTypeStringLiteral)) {
                if (this->getDebug()) {
                    std::cerr << "Argument was not a string literal, skipping.\n";
                }
                newNodes.push_back(node);
                continue;
            }
            LILString argStr = std::static_pointer_cast<LILStringLiteral>(arg)->getValue().stripQuotes();
            auto paths = this->_resolveFilePaths(argStr);
            for (auto path : paths) {
                if (this->isAlreadyImported(path, isNeeds)) {
                    if (this->getVerbose() && instr->getVerbose()) {
                        std::cerr << "File " << path.data() << " was already imported. Skipping.\n\n";
                    }
                    auto aiNodes = this->getNodesForAlreadyImportedFile(path, isNeeds);
                    resultNodes.insert(resultNodes.end(), aiNodes.begin(), aiNodes.end());
                    continue;
                }
                if (this->getVerbose() && instr->getVerbose()) {
                    std::cerr << "Start of file " << path.data() << "\n\n========================================\n\n";
                }
                
                std::unique_ptr<LILCodeUnit> codeUnit = std::make_unique<LILCodeUnit>();
                codeUnit->setNeedsConfigureDefaults(false);
                if (isNeeds) {
                    codeUnit->setIsBeingImportedWithNeeds(true);
                    for (auto it = this->_alreadyImportedFilesNeeds.begin(); it != this->_alreadyImportedFilesNeeds.end(); ++it) {
                        auto pair = *it;
                        codeUnit->addAlreadyImportedFile(pair.first, pair.second, true);
                    }
                } else {
                    codeUnit->setIsBeingImportedWithImport(true);
                    for (auto it = this->_alreadyImportedFilesImport.begin(); it != this->_alreadyImportedFilesImport.end(); ++it) {
                        auto pair = *it;
                        codeUnit->addAlreadyImportedFile(pair.first, pair.second, false);
                    }
                }
                auto newRoot = codeUnit->getRootNode();
                codeUnit->setFile(path);
                LILString dir = this->_getDir(path);
                codeUnit->setDir(dir);
                std::ifstream file(path.data(), std::ios::in);
                if (file.fail()) {
                    std::cerr << "\nERROR: Failed to read the file "+path.data()+"\n\n";
                    continue;
                }
                std::stringstream buffer;
                buffer << file.rdbuf();
                LILString lilStr(buffer.str());
                codeUnit->setSource(lilStr);
                codeUnit->setDebugAST(this->getDebugAST());
                codeUnit->setVerbose(this->getVerbose() && instr->getVerbose());
                codeUnit->run();
                bool hasErrs = codeUnit->hasErrors();
                bool instrIsExported = instr->getIsExported();
                if (!hasErrs) {
                    if (isNeeds) {
                        for (auto node : newRoot->getChildNodes()) {
                            this->_importNodeIfNeeded(&newNodes, node, instrIsExported);
                        }
                    } else {
                        for (const auto & node : newRoot->getChildNodes()) {
                            if (node->getIsExported()) {
                                node->setIsExported(instrIsExported);
                                newNodes.push_back(node);
                            }
                        }
                    }
                }
                if (this->getVerbose() && instr->getVerbose()) {
                    std::cerr << "\nEnd of file " << path.data() << "\n\n========================================\n\n";
                }
                if (hasErrs) {
                    LILErrorMessage ei;
                    ei.message =  "Errors encountered while importing file "+path;
                    LILNode::SourceLocation sl = node->getSourceLocation();
                    ei.file = sl.file;
                    ei.line = sl.line;
                    ei.column = sl.column;
                    this->errors.push_back(ei);
                    return;
                }
                this->_needsAnotherPass = true;
                for (auto newNode : newNodes) {
                    if ( ! (this->getVerbose() && instr->getVerbose()) ) {
                        newNode->hidden = true;
                    }
                }
                this->addAlreadyImportedFile(path, newNodes, isNeeds);
                resultNodes.insert(resultNodes.end(), newNodes.begin(), newNodes.end());
            }
        }
        it += 1;
    }
    rootNode->clearNodes();
    for (auto resultIt = resultNodes.begin(); resultIt != resultNodes.end(); resultIt += 1) {
        rootNode->add(*resultIt);
    }
}

void LILPreprocessor::processIfInstr(const std::shared_ptr<LILRootNode> & rootNode)
{
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    bool hasChanges = false;
    for (auto node : nodes) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            //out with the old
            switch (node->getNodeType()) {
                case NodeTypeVarDecl:
                {
                    auto vd = std::static_pointer_cast<LILVarDecl>(node);
                    rootNode->unsetLocalVariable(vd->getName());
                    break;
                }
                case NodeTypeFunctionDecl:
                {
                    auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
                    rootNode->unsetLocalVariable(fd->getName());
                    break;
                }
                default:
                    break;
            }
            rootNode->removeNode(node);
            //in with the new
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
                switch (newNode->getNodeType()) {
                    case NodeTypeVarDecl:
                    {
                        auto vd = std::static_pointer_cast<LILVarDecl>(newNode);
                        rootNode->setLocalVariable(vd->getName(), vd);
                        break;
                    }
                    case NodeTypeFunctionDecl:
                    {
                        auto fd = std::static_pointer_cast<LILFunctionDecl>(newNode);
                        rootNode->setLocalVariable(fd->getName(), fd);
                        break;
                    }
                    default:
                        break;
                }
            }
            hasChanges = true;
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        rootNode->setChildNodes(std::move(resultNodes));
    }
}

bool LILPreprocessor::processIfInstr(std::shared_ptr<LILNode> node)
{
    if (this->getDebug()) {
        std::cerr << "##  processing #if instructions in " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeRoot:
        case NodeTypeNull:
        case NodeTypeBoolLiteral:
        case NodeTypeNumberLiteral:
        case NodeTypePercentage:
        case NodeTypeStringLiteral:
        case NodeTypeCStringLiteral:
        case NodeTypePropertyName:
        case NodeTypeVarName:
        case NodeTypeAliasDecl:
        case NodeTypeTypeDecl:
        case NodeTypeType:
        case NodeTypeMultipleType:
        case NodeTypeFunctionType:
        case NodeTypeObjectType:
        case NodeTypePointerType:
        case NodeTypeStaticArrayType:
        case NodeTypeFlag:
        case NodeTypeFilter:
        case NodeTypeSelector:
        case NodeTypeCombinator:
        case NodeTypeForeignLang:
        case NodeTypeComment:
        case NodeTypeInvalid:
        case NodeTypeDocumentation:
            //do nothing
            break;
            
        case NodeTypeNegation:
            std::cerr << "UNIMPLEMENTED FAIL !!!!!\n\n";
            return false;
            
        case NodeTypeExpression:
        {
            auto value = std::static_pointer_cast<LILExpression>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeUnaryExpression:
        {
            auto value = std::static_pointer_cast<LILUnaryExpression>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeStringFunction:
        {
            auto value = std::static_pointer_cast<LILStringFunction>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeVarDecl:
        case NodeTypeConstDecl:
        {
            auto value = std::static_pointer_cast<LILVarDecl>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeConversionDecl:
        {
            auto value = std::static_pointer_cast<LILConversionDecl>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeClassDecl:
        {
            auto value = std::static_pointer_cast<LILClassDecl>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeObjectDefinition:
        {
            auto value = std::static_pointer_cast<LILObjectDefinition>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeAssignment:
        {
            auto value = std::static_pointer_cast<LILAssignment>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeRule:
        {
            auto value = std::static_pointer_cast<LILRule>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeSelectorChain:
        {
            auto value = std::static_pointer_cast<LILSelectorChain>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeFunctionDecl:
        {
            auto value = std::static_pointer_cast<LILFunctionDecl>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeFunctionCall:
        {
            auto value = std::static_pointer_cast<LILFunctionCall>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeFlowControl:
        {
            auto value = std::static_pointer_cast<LILFlowControl>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeFlowControlCall:
        {
            auto value = std::static_pointer_cast<LILFlowControlCall>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeInstruction:
        {
            auto value = std::static_pointer_cast<LILInstruction>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeIfInstruction:
        {
            auto value = std::static_pointer_cast<LILIfInstruction>(node);
            return this->_processIfInstrIfInstr(value);
        }
        case NodeTypeSnippetInstruction:
        {
            auto value = std::static_pointer_cast<LILSnippetInstruction>(node);
            return this->_processIfInstrSnippetInstr(value);
        }
        case NodeTypeValueList:
        {
            auto value = std::static_pointer_cast<LILValueList>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeIndexAccessor:
        {
            auto value = std::static_pointer_cast<LILIndexAccessor>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeValuePath:
        {
            auto value = std::static_pointer_cast<LILValuePath>(node);
            return this->_processIfInstr(value);
        }
        case NodeTypeSimpleSelector:
        {
            auto value = std::static_pointer_cast<LILSimpleSelector>(node);
            return this->_processIfInstr(value);
        }
    }
    return false;
}

void LILPreprocessor::processPasteInstr(const std::shared_ptr<LILRootNode> & rootNode)
{
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    bool hasChanges = false;
    for (auto node : nodes) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            rootNode->removeNode(node);
            //in with the new
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
                rootNode->add(newNode, false);
            }
            hasChanges = true;
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        rootNode->setChildNodes(std::move(resultNodes));
    }
}

bool LILPreprocessor::processPasteInstr(std::shared_ptr<LILNode> node)
{
    if (this->getDebug()) {
        std::cerr << "##  processing #paste instructions in " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeRoot:
        case NodeTypeNull:
        case NodeTypeBoolLiteral:
        case NodeTypeNumberLiteral:
        case NodeTypePercentage:
        case NodeTypeStringLiteral:
        case NodeTypeCStringLiteral:
        case NodeTypePropertyName:
        case NodeTypeVarName:
        case NodeTypeAliasDecl:
        case NodeTypeTypeDecl:
        case NodeTypeType:
        case NodeTypeMultipleType:
        case NodeTypeFunctionType:
        case NodeTypeObjectType:
        case NodeTypePointerType:
        case NodeTypeStaticArrayType:
        case NodeTypeFlag:
        case NodeTypeFilter:
        case NodeTypeSelector:
        case NodeTypeCombinator:
        case NodeTypeForeignLang:
        case NodeTypeComment:
        case NodeTypeInvalid:
        case NodeTypeDocumentation:
            //do nothing
            break;
            
        case NodeTypeNegation:
            std::cerr << "UNIMPLEMENTED FAIL !!!!!\n\n";
            return false;
            
        case NodeTypeExpression:
        {
            auto value = std::static_pointer_cast<LILExpression>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeUnaryExpression:
        {
            auto value = std::static_pointer_cast<LILUnaryExpression>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeStringFunction:
        {
            auto value = std::static_pointer_cast<LILStringFunction>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeVarDecl:
        case NodeTypeConstDecl:
        {
            auto value = std::static_pointer_cast<LILVarDecl>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeConversionDecl:
        {
            auto value = std::static_pointer_cast<LILConversionDecl>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeClassDecl:
        {
            auto value = std::static_pointer_cast<LILClassDecl>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeObjectDefinition:
        {
            auto value = std::static_pointer_cast<LILObjectDefinition>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeAssignment:
        {
            auto value = std::static_pointer_cast<LILAssignment>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeRule:
        {
            auto value = std::static_pointer_cast<LILRule>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeSelectorChain:
        {
            auto value = std::static_pointer_cast<LILSelectorChain>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeFunctionDecl:
        {
            auto value = std::static_pointer_cast<LILFunctionDecl>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeFunctionCall:
        {
            auto value = std::static_pointer_cast<LILFunctionCall>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeFlowControl:
        {
            auto value = std::static_pointer_cast<LILFlowControl>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeFlowControlCall:
        {
            auto value = std::static_pointer_cast<LILFlowControlCall>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeInstruction:
        {
            auto value = std::static_pointer_cast<LILInstruction>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeIfInstruction:
        {
            auto value = std::static_pointer_cast<LILIfInstruction>(node);
            return this->_processPasteInstrIfInstr(value);
        }
        case NodeTypeSnippetInstruction:
        {
            auto value = std::static_pointer_cast<LILSnippetInstruction>(node);
            return this->_processPasteInstrSnippetInstr(value);
        }
        case NodeTypeValueList:
        {
            auto value = std::static_pointer_cast<LILValueList>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeIndexAccessor:
        {
            auto value = std::static_pointer_cast<LILIndexAccessor>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeValuePath:
        {
            auto value = std::static_pointer_cast<LILValuePath>(node);
            return this->_processPasteInstr(value);
        }
        case NodeTypeSimpleSelector:
        {
            auto value = std::static_pointer_cast<LILSimpleSelector>(node);
            return this->_processPasteInstr(value);
        }
    }
    return false;
}

void LILPreprocessor::removeSnippets(std::shared_ptr<LILRootNode> rootNode)
{
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    
    for (auto node : nodes) {
        this->_removeSnippets(node);
    }
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    bool hasChanges = false;
    for (auto node : nodes) {
        if (node->isA(NodeTypeSnippetInstruction)) {
            hasChanges = true;
        } else {
            resultNodes.push_back(node);
        }
    }
    if (hasChanges) {
        rootNode->setChildNodes(std::move(resultNodes));
    }
}

void LILPreprocessor::_removeSnippets(std::shared_ptr<LILNode> node)
{
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    bool hasChanges = false;
    for (auto child : node->getChildNodes()) {
        if (child->isA(NodeTypeSnippetInstruction)) {
            hasChanges = true;
        } else {
            resultNodes.push_back(child);
        }
    }
    if (hasChanges) {
        node->setChildNodes(std::move(resultNodes));
    }
}


void LILPreprocessor::setDir(LILString dir)
{
    this->_dir = dir;
}

LILString LILPreprocessor::getDir() const
{
    return this->_dir;
}

bool LILPreprocessor::getDebugAST() const
{
    return this->_debugAST;
}

void LILPreprocessor::setDebugAST(bool value)
{
    this->_debugAST = value;
}

void LILPreprocessor::addAlreadyImportedFile(LILString path, std::vector<std::shared_ptr<LILNode>> nodes, bool isNeeds)
{
    if (isNeeds) {
        this->_alreadyImportedFilesNeeds[path] = nodes;
    } else {
        this->_alreadyImportedFilesImport[path] = nodes;
    }
}

bool LILPreprocessor::isAlreadyImported(const LILString & path, bool isNeeds)
{
    if (isNeeds) {
        return this->_alreadyImportedFilesNeeds.count(path) > 0;
    } else {
        return this->_alreadyImportedFilesImport.count(path) > 0;
    }
}

std::vector<std::shared_ptr<LILNode>> LILPreprocessor::getNodesForAlreadyImportedFile(const LILString & path, bool isNeeds)
{
    if (isNeeds && this->_alreadyImportedFilesNeeds.count(path)) {
        return this->_alreadyImportedFilesNeeds.at(path);
    } else if (!isNeeds && this->_alreadyImportedFilesImport.count(path)) {
        return this->_alreadyImportedFilesImport.at(path);
    }
    std::vector<std::shared_ptr<LILNode>> emptyVect;
    return emptyVect;
}

std::vector<LILString> LILPreprocessor::_resolveFilePaths(LILString argStr) const
{
    std::vector<LILString> ret;
    std::string dir = this->getDir().data() + "/";
    std::string arg = argStr.data();
    if (arg.find("*") == std::string::npos) {
        ret.push_back(dir+argStr.data());
    } else {
        std::vector<std::string> paths = this->_glob(dir+argStr.data());
        for (auto path : paths) {
            ret.push_back(path);
        }
    }
    return ret;
}

//FIXME: needs windows counterpart
std::vector<std::string> LILPreprocessor::_glob(const std::string& pattern) const
{
    std::vector<std::string> filenames;
    
    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));
    
    // do the glob operation
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        std::cerr << "glob() failed with return_value " << return_value << std::endl;
        return filenames;
    }

    // collect all the filenames into a std::list<std::string>
    
    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(std::string(glob_result.gl_pathv[i]));
    }
    
    // cleanup
    globfree(&glob_result);
    
    // done
    return filenames;
}

void LILPreprocessor::_importNodeIfNeeded(std::vector<std::shared_ptr<LILNode>> * newNodes, std::shared_ptr<LILNode> node, bool isExported) const
{
    if (!node->getIsExported()) {
        return;
    }
    switch (node->getNodeType()) {
        case NodeTypeVarDecl:
        {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            auto ty = vd->getType();
            if (ty && ty->isA(TypeTypeFunction)) {
                auto newVd = std::make_shared<LILVarDecl>();
                newVd->setIsExtern(true);
                newVd->setIsExported(isExported);
                newVd->setName(vd->getName());
                newVd->setType(ty->clone());
                newNodes->push_back(newVd);
            } else {
                auto clone = node->clone();
                clone->setIsExported(isExported);
                newNodes->push_back(clone);
            }
            break;
        }
            
        case NodeTypeClassDecl:
        {
            auto cd = std::static_pointer_cast<LILClassDecl>(node);
            if (cd->isTemplate())
            {
                newNodes->push_back(cd);
            }
            else
            {
                auto newCd = std::make_shared<LILClassDecl>();
                newCd->setIsExtern(true);
                newCd->setIsExported(isExported);
                auto newTy = std::make_shared<LILObjectType>();
                newTy->setTypeType(TypeTypeObject);
                newTy->setName(cd->getName());
                newCd->setType(newTy);
                
                auto fields = cd->getFields();
                for (auto field : fields) {
                    if (!field->isA(NodeTypeVarDecl)) {
                        continue;
                    }
                    auto fldVd = std::static_pointer_cast<LILVarDecl>(field);
                    auto newVd = std::make_shared<LILVarDecl>();
                    auto fldTy = field->getType();
                    if (!fldTy) {
                        continue;
                    }
                    newVd->setType(fldTy->clone());
                    newVd->setName(fldVd->getName());
                    newCd->addField(newVd);
                }
                
                auto methods = cd->getMethods();
                for (auto method : methods) {
                    if (!method->isA(NodeTypeVarDecl)) {
                        continue;
                    }
                    auto metVd = std::static_pointer_cast<LILVarDecl>(method);
                    auto newVd = std::make_shared<LILVarDecl>();
                    auto metTy = method->getType();
                    if (!metTy) {
                        continue;
                    }
                    newVd->setType(metTy->clone());
                    newVd->setName(metVd->getName());
                    newCd->addMethod(newVd);
                }
                
                newNodes->push_back(newCd);
            }
            break;
        }
            
        case NodeTypeAliasDecl:
        case NodeTypeTypeDecl:
        {
            auto clone = node->clone();
            clone->setIsExported(isExported);
            newNodes->push_back(clone);
            break;
        }

        case NodeTypeFunctionDecl:
        {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
            auto newFd = std::make_shared<LILFunctionDecl>();
            newFd->setFunctionDeclType(FunctionDeclTypeFn);
            newFd->setName(fd->getName());
            newFd->setIsExtern(true);
            newFd->setIsExported(isExported);
            newFd->setHasOwnType(true);
            newFd->setType(fd->getType()->clone());
            
            if (fd->getHasMultipleImpls()) {
                newFd->setHasMultipleImpls(true);
                for (auto impl : fd->getImpls()) {
                    auto newImpl = std::make_shared<LILFunctionDecl>();
                    newImpl->setFunctionDeclType(FunctionDeclTypeFn);
                    newImpl->setName(impl->getName());
                    newImpl->setIsExtern(true);
                    newImpl->setHasOwnType(true);
                    newImpl->setType(impl->getType()->clone());
                    newFd->addImpl(newImpl);
                }
            }

            newNodes->push_back(newFd);
            break;
        }
            
        case NodeTypeDocumentation:
        {
            //ignore
            break;
        }

        default:
            std::cerr << "UNIMPLEMENTED FAIL !!!!!! \n\n";
            break;
    }
}

LILString LILPreprocessor::_getDir(LILString path) const
{
    std::string dir = path.data();
    size_t i = 0;
    size_t pos = 0;
    bool found = false;
    for (std::string::iterator it = dir.begin(); it != dir.end(); ++it) {
        char cc = *it;
        if (cc == '/' || cc == '\\') {
            found = true;
            pos = i;
        }
        i += 1;
    }
    if (found) {
        return dir.substr(0, pos);
    } else {
        return "";
    }
}


bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILExpression> value)
{
    auto left = value->getLeft();
    if (!left) {
        std::cerr << "EXPRESSION HAD NO LEFT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeLeft = this->processIfInstr(left);
    if (removeLeft) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #if leaves expression without left side value. Use an else block and provide an alternate value.";
        LILNode::SourceLocation sl = left->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 1) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #if for left side value of expression returns multiple values";
        LILNode::SourceLocation sl = left->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() == 1) {
        value->setLeft(this->_nodeBuffer.back().back());
        this->_needsAnotherPass = true;
    }
    this->_nodeBuffer.pop_back();
    
    auto right = value->getRight();
    if (!right) {
        std::cerr << "EXPRESSION HAD NO RIGHT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeRight = this->processIfInstr(right);
    if (removeRight) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #if leaves expression without right side value. Use an else block and provide an alternate value.";
        LILNode::SourceLocation sl = right->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 1) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #if for right side value of expression returns multiple values";
        LILNode::SourceLocation sl = right->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() == 1) {
        value->setRight(this->_nodeBuffer.back().back());
        this->_needsAnotherPass = true;
    }
    this->_nodeBuffer.pop_back();
    
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILUnaryExpression> value)
{
    auto val = value->getValue();
    if (val && val->isA(InstructionTypeIf)) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(val);
        if (remove) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #if leaves unary expression without value. Use an else block and provide an alternate value.";
            LILNode::SourceLocation sl = val->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() == 1) {
            value->setValue(this->_nodeBuffer.back().back());
            this->_needsAnotherPass = true;
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILStringFunction> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        value->setNodes(std::move(resultNodes));
        this->_needsAnotherPass = true;
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILVarDecl> value)
{
    auto initVal = value->getInitVal();
    if (initVal) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->processIfInstr(initVal);
        if (this->_nodeBuffer.back().size() > 0) {
            value->setInitVals(this->_nodeBuffer.back());
            this->_needsAnotherPass = true;
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILClassDecl> value)
{
    if (value->getIsExtern()) {
        return false;
    }
    
    std::vector<std::shared_ptr<LILNode>> newNodes;
    
    std::vector<std::shared_ptr<LILNode>> nodes = value->getMethods();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        resultNodes.push_back(node);
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->processIfInstr(node);
        for (auto newNode : this->_nodeBuffer.back()) {
            resultNodes.push_back(newNode);
            newNodes.push_back(newNode);
        }
        this->_nodeBuffer.pop_back();
    }
    if (newNodes.size() > 0) {
        this->_needsAnotherPass = true;
        for (auto newNode : newNodes) {
            value->addMethod(newNode);
        }
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILObjectDefinition> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        value->setNodes(std::move(resultNodes));
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILAssignment> value)
{
    auto val = value->getValue();
    if (val && val->isA(InstructionTypeIf)) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(val);
        if (remove) {
            value->clearValue();
        } else if (this->_nodeBuffer.back().size() == 1) {
            value->setValue(this->_nodeBuffer.back().back());
            this->_needsAnotherPass = true;
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILValuePath> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        value->setNodes(resultNodes);
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILRule> value)
{
    bool hasChangesSCh = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesSCh;
    for (auto node : value->getSelectorChains()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesSCh.push_back(node);
        } else {
            hasChangesSCh = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesSCh.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesSCh) {
        value->setSelectorChains(std::move(resultNodesSCh));
    }
    
    for (auto rule : value->getChildRules()) {
        this->processIfInstr(rule);
    }
    
    bool hasChangesVal = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesVal;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesVal.push_back(node);
        } else {
            hasChangesVal = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesVal.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesVal) {
        value->setValues(std::move(resultNodesVal));
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILSelectorChain> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        value->setNodes(std::move(resultNodes));
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILSimpleSelector> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        value->setNodes(std::move(resultNodes));
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILFunctionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        this->_needsAnotherPass = true;
        value->setBody(resultNodes);
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILFunctionCall> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesArgs.push_back(node);
        } else {
            hasChangesArgs = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesArgs.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesArgs) {
        this->_needsAnotherPass = true;
        value->setArguments(std::move(resultNodesArgs));
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILFlowControl> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesArgs.push_back(node);
        } else {
            hasChangesArgs = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesArgs.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesArgs) {
        this->_needsAnotherPass = true;
        value->setArguments(std::move(resultNodesArgs));
    }
    
    bool hasChangesThen = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesThen;
    for (auto node : value->getThen()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesThen.push_back(node);
        } else {
            hasChangesThen = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesThen.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesThen) {
        this->_needsAnotherPass = true;
        value->setThen(std::move(resultNodesThen));
    }
    
    bool hasChangesElse = false;
    
    std::vector<std::shared_ptr<LILNode>> resultNodesElse;
    for (auto node : value->getElse()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesElse.push_back(node);
        } else {
            hasChangesElse = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesElse.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesElse) {
        this->_needsAnotherPass = true;
        value->setElse(std::move(resultNodesElse));
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILFlowControlCall> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processIfInstr(arg);
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(arg);
        if (remove) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #if leaves instruction without argument. Use an else block and provide an alternate value.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() == 1) {
            this->_needsAnotherPass = true;
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILPreprocessor::_processIfInstrIfInstr(std::shared_ptr<LILIfInstruction> value)
{
    bool ret = false;
    auto arg = value->getArgument();
    if (arg) {
        auto remoteNode = this->recursiveFindNode(arg);
        if (remoteNode && remoteNode->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(remoteNode);
            if (!vd->getIsConst()) {
                LILErrorMessage ei;
                ei.message =  "Argument of #if instruction was not const.";
                LILNode::SourceLocation sl = value->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
                return true;
            }
            auto val = vd->getInitVal();
            if (!val) {
                LILErrorMessage ei;
                ei.message =  "Constant declaration has no value";
                LILNode::SourceLocation sl = vd->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
                return true;
            }
            auto & nbb = this->_nodeBuffer.back();
            if (this->_evaluate(val)) {
                auto then = value->getThen();
                if (then.size() > 0) {
                    for (auto child : then) {
                        nbb.push_back(child);
                    }
                } else {
                    ret = true;
                }
                
            } else {
                auto els = value->getElse();
                if (els.size() > 0) {
                    for (auto child : value->getElse()) {
                        nbb.push_back(child);
                    }
                } else {
                    ret = true;
                }
            }
        }
    }
    return ret;
}

bool LILPreprocessor::_processIfInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(arg);
        if (remove) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #if leaves instruction without argument. Use an else block and provide an alternate value.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() == 1) {
            this->_needsAnotherPass = true;
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }

    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        this->_needsAnotherPass = true;
        value->setBody(std::move(resultNodes));
    }
    
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILValueList> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultValues;
    for (auto value : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(value);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultValues.push_back(value);
        } else {
            hasChanges = true;
            for (auto newValue : this->_nodeBuffer.back()) {
                resultValues.push_back(newValue);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        value->setValues(resultValues);
    }
    return false;
}


bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILIndexAccessor> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processIfInstr(arg);
    }
    return false;
}

bool LILPreprocessor::_processIfInstr(std::shared_ptr<LILConversionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processIfInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        this->_needsAnotherPass = true;
        value->setBody(std::move(resultNodes));
    }
    return false;
}

bool LILPreprocessor::_evaluate(std::shared_ptr<LILNode> node)
{
    switch (node->getNodeType()) {
        case NodeTypeBoolLiteral:
        {
            auto value = std::static_pointer_cast<LILBoolLiteral>(node);
            return value->getValue();
        }
        case NodeTypeExpression:
        {
            auto value = std::static_pointer_cast<LILExpression>(node);
            auto left = this->recursiveFindNode(value->getLeft());
            if (!left) {
                std::cerr << "LEFT NODE OF EXPRESSION MISSING FAIL!!!!!!!!\n\n";
                return false;
            }
            auto right = this->recursiveFindNode(value->getRight());
            if (!right) {
                std::cerr << "RIGHT NODE OF EXPRESSION MISSING FAIL!!!!!!!!\n\n";
                return false;
            }
            auto leftTy = left->getType();
            if (!leftTy) {
                std::cerr << "TYPE OF LEFT NODE OF EXPRESSION MISSING FAIL!!!!!!!!\n\n";
                return false;
            }
            auto rightTy = right->getType();
            if (!rightTy) {
                std::cerr << "TYPE OF RIGHT NODE OF EXPRESSION MISSING FAIL!!!!!!!!\n\n";
                return false;
            }
            if (!leftTy->equalTo(rightTy)) {
                std::cerr << "EXPRESSION TYPE MISMATCH FAIL!!!!!!!!\n\n";
                return false;
            }
            switch (value->getExpressionType()) {
                case ExpressionTypeEqualComparison:
                {
                    return left->equalTo(right);
                }
                default:
                    std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!\n\n";
                    break;
            }
        }
            
        default:
            std::cerr << "UNKNOWN NODE TYPE TO EVALUATE FAIL!!!!!!!!\n\n";
            break;
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILExpression> value)
{
    auto left = value->getLeft();
    if (!left) {
        std::cerr << "EXPRESSION HAD NO LEFT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeLeft = this->processPasteInstr(left);
    if (removeLeft) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #paste leaves expression without left side value. Use an else block and provide an alternate value.";
        LILNode::SourceLocation sl = left->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 1) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #paste for left side value of expression returns multiple values";
        LILNode::SourceLocation sl = left->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() == 1) {
        value->setLeft(this->_nodeBuffer.back().back());
        this->_needsAnotherPass = true;
    }
    this->_nodeBuffer.pop_back();
    
    auto right = value->getRight();
    if (!right) {
        std::cerr << "EXPRESSION HAD NO RIGHT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeRight = this->processPasteInstr(right);
    if (removeRight) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #paste leaves expression without right side value. Use an else block and provide an alternate value.";
        LILNode::SourceLocation sl = right->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 1) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #paste for right side value of expression returns multiple values";
        LILNode::SourceLocation sl = right->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() == 1) {
        value->setRight(this->_nodeBuffer.back().back());
        this->_needsAnotherPass = true;
    }
    this->_nodeBuffer.pop_back();
    
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILUnaryExpression> value)
{
    auto val = value->getValue();
    if (val && val->isA(InstructionTypeIf)) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(val);
        if (remove) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #paste leaves unary expression without value. Use an else block and provide an alternate value.";
            LILNode::SourceLocation sl = val->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() == 1) {
            value->setValue(this->_nodeBuffer.back().back());
            this->_needsAnotherPass = true;
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILStringFunction> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        value->setNodes(std::move(resultNodes));
        this->_needsAnotherPass = true;
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILVarDecl> value)
{
    auto initVal = value->getInitVal();
    if (initVal) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->processPasteInstr(initVal);
        if (this->_nodeBuffer.back().size() > 0) {
            value->setInitVals(this->_nodeBuffer.back());
            this->_needsAnotherPass = true;
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILClassDecl> value)
{
    if (value->getIsExtern()) {
        return false;
    }
    
    std::vector<std::shared_ptr<LILNode>> newNodes;
    
    std::vector<std::shared_ptr<LILNode>> nodes = value->getMethods();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        resultNodes.push_back(node);
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->processPasteInstr(node);
        for (auto newNode : this->_nodeBuffer.back()) {
            resultNodes.push_back(newNode);
            newNodes.push_back(newNode);
        }
        this->_nodeBuffer.pop_back();
    }
    if (newNodes.size() > 0) {
        this->_needsAnotherPass = true;
        for (auto newNode : newNodes) {
            value->addMethod(newNode);
        }
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILObjectDefinition> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        value->setNodes(std::move(resultNodes));
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILAssignment> value)
{
    auto val = value->getValue();
    if (val && val->isA(InstructionTypeIf)) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(val);
        if (remove) {
            value->clearValue();
        } else if (this->_nodeBuffer.back().size() == 1) {
            value->setValue(this->_nodeBuffer.back().back());
            this->_needsAnotherPass = true;
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILValuePath> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        value->setNodes(resultNodes);
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILRule> value)
{
    std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!\n\n";
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILSelectorChain> value)
{
    std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!\n\n";
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILSimpleSelector> value)
{
    std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!\n\n";
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILFunctionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        this->_needsAnotherPass = true;
        value->setBody(resultNodes);
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILFunctionCall> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesArgs.push_back(node);
        } else {
            hasChangesArgs = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesArgs.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesArgs) {
        this->_needsAnotherPass = true;
        value->setArguments(std::move(resultNodesArgs));
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILFlowControl> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesArgs.push_back(node);
        } else {
            hasChangesArgs = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesArgs.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesArgs) {
        this->_needsAnotherPass = true;
        value->setArguments(std::move(resultNodesArgs));
    }
    
    bool hasChangesThen = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesThen;
    for (auto node : value->getThen()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesThen.push_back(node);
        } else {
            hasChangesThen = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesThen.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesThen) {
        this->_needsAnotherPass = true;
        value->setThen(std::move(resultNodesThen));
    }
    
    bool hasChangesElse = false;
    
    std::vector<std::shared_ptr<LILNode>> resultNodesElse;
    for (auto node : value->getElse()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesElse.push_back(node);
        } else {
            hasChangesElse = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesElse.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesElse) {
        this->_needsAnotherPass = true;
        value->setElse(std::move(resultNodesElse));
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILFlowControlCall> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processPasteInstr(arg);
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILInstruction> value)
{
    if (value->isA(InstructionTypePaste)) {
        auto snippet = this->getRootNode()->getSnippetNamed(value->getName());
        if (snippet) {
            auto & nbb = this->_nodeBuffer.back();
            auto snipBody = snippet->getBody();
            if (snipBody.size() > 0) {
                nbb.insert(nbb.end(), snipBody.begin(), snipBody.end());
            } else {
                return true;
            }
        }
    } else {
        auto arg = value->getArgument();
        if (arg) {
            this->_nodeBuffer.emplace_back();
            bool remove = this->processPasteInstr(arg);
            if (remove) {
                LILErrorMessage ei;
                ei.message =  "Evaluation of #paste leaves instruction without argument. Use an else block and provide an alternate value.";
                LILNode::SourceLocation sl = arg->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            } else if (this->_nodeBuffer.back().size() == 1) {
                this->_needsAnotherPass = true;
                value->setArgument(this->_nodeBuffer.back().back());
            }
            this->_nodeBuffer.pop_back();
        }
        return false;
    }
    return false;
}

bool LILPreprocessor::_processPasteInstrIfInstr(std::shared_ptr<LILIfInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(arg);
        if (remove) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #paste leaves instruction without argument. Use an else block and provide an alternate value.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() == 1) {
            this->_needsAnotherPass = true;
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    
    bool hasChangesThen = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesThen;
    for (auto node : value->getThen()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesThen.push_back(node);
        } else {
            hasChangesThen = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesThen.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesThen) {
        this->_needsAnotherPass = true;
        value->setThen(std::move(resultNodesThen));
    }
    
    bool hasChangesElse = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesElse;
    for (auto node : value->getElse()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesElse.push_back(node);
        } else {
            hasChangesElse = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesElse.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesElse) {
        this->_needsAnotherPass = true;
        value->setElse(std::move(resultNodesElse));
    }
    return false;
}

bool LILPreprocessor::_processPasteInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(arg);
        if (remove) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #paste leaves instruction without argument. Use an else block and provide an alternate value.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() == 1) {
            this->_needsAnotherPass = true;
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }

    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        this->_needsAnotherPass = true;
        value->setBody(std::move(resultNodes));
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILValueList> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        this->_needsAnotherPass = true;
        value->setValues(resultNodes);
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILIndexAccessor> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processPasteInstr(arg);
    }
    return false;
}

bool LILPreprocessor::_processPasteInstr(std::shared_ptr<LILConversionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processPasteInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        this->_needsAnotherPass = true;
        value->setBody(std::move(resultNodes));
    }
    return false;
}
