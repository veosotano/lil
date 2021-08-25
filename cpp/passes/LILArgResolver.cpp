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
 *      This file handles #arg instructions
 *
 ********************************************************************/


#include "LILArgResolver.h"
#include "LILASTBuilder.h"
#include "LILCodeParser.h"
#include "LILNodeToString.h"
#include "LILRootNode.h"
#include "LILStringLiteral.h"

using namespace LIL;

LILArgResolver::LILArgResolver()
{
}

LILArgResolver::~LILArgResolver()
{
}

void LILArgResolver::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  RESOLVING ARGS  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILArgResolver::visit(LILNode *node)
{
    
}

void LILArgResolver::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    for (auto node : nodes) {
        this->processArgInstr(node);
    }
}

bool LILArgResolver::processArgInstr(std::shared_ptr<LILNode> node)
{
    if (this->getDebug()) {
        std::cerr << "##  processing #arg instructions in " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
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
            return this->_processArgInstr(value);
        }
        case NodeTypeUnaryExpression:
        {
            auto value = std::static_pointer_cast<LILUnaryExpression>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeStringFunction:
        {
            auto value = std::static_pointer_cast<LILStringFunction>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeVarDecl:
        case NodeTypeConstDecl:
        {
            auto value = std::static_pointer_cast<LILVarDecl>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeConversionDecl:
        {
            auto value = std::static_pointer_cast<LILConversionDecl>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeClassDecl:
        {
            auto value = std::static_pointer_cast<LILClassDecl>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeObjectDefinition:
        {
            auto value = std::static_pointer_cast<LILObjectDefinition>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeAssignment:
        {
            auto value = std::static_pointer_cast<LILAssignment>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeRule:
        {
            auto value = std::static_pointer_cast<LILRule>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeSelectorChain:
        {
            auto value = std::static_pointer_cast<LILSelectorChain>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeFunctionDecl:
        {
            auto value = std::static_pointer_cast<LILFunctionDecl>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeFunctionCall:
        {
            auto value = std::static_pointer_cast<LILFunctionCall>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeFlowControl:
        {
            auto value = std::static_pointer_cast<LILFlowControl>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeFlowControlCall:
        {
            auto value = std::static_pointer_cast<LILFlowControlCall>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeInstruction:
        {
            auto value = std::static_pointer_cast<LILInstruction>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeIfInstruction:
        {
            auto value = std::static_pointer_cast<LILIfInstruction>(node);
            return this->_processArgInstrIfInstr(value);
        }
        case NodeTypeSnippetInstruction:
        {
            auto value = std::static_pointer_cast<LILSnippetInstruction>(node);
            return this->_processArgInstrSnippetInstr(value);
        }
        case NodeTypeValueList:
        {
            auto value = std::static_pointer_cast<LILValueList>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeIndexAccessor:
        {
            auto value = std::static_pointer_cast<LILIndexAccessor>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeValuePath:
        {
            auto value = std::static_pointer_cast<LILValuePath>(node);
            return this->_processArgInstr(value);
        }
        case NodeTypeSimpleSelector:
        {
            auto value = std::static_pointer_cast<LILSimpleSelector>(node);
            return this->_processArgInstr(value);
        }
    }
    return false;
}

bool LILArgResolver::hasCustomArg(const LILString & name) const
{
    return this->_customArgs.count(name) > 0;
}

void LILArgResolver::setCustomArgs(std::vector<LILString> & args)
{
    auto astBuilder = std::make_unique<LILASTBuilder>();
    astBuilder->setBuildFlatList(true);
    auto rootNode = astBuilder->getRootNode();
    auto parser = std::make_unique<LILCodeParser>(astBuilder.get());
    for (LILString argStr : args) {
        const auto & arg = argStr.data();
        auto colonPos = arg.find(':');
        std::string name;
        if (colonPos != std::string::npos) {
            name = arg.substr(2, colonPos-2);
            auto value = arg.substr(colonPos+1, arg.length()-colonPos-1);
            parser->parseString(value);
        }
        else
        {
            name = arg.substr(2);
            auto value = "true";
            parser->parseString(value);
        }
        if (rootNode->getChildNodes().size() > 0) {
            this->_customArgs[name] = rootNode->getChildNodes().back();
            rootNode->clearNodes();
        }
    }
}

std::shared_ptr<LILNode> LILArgResolver::getCustomArg(const LILString & name)
{
    if (this->_customArgs.count(name)) {
        return this->_customArgs[name];
    } else {
        return nullptr;
    }
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILExpression> value)
{
    auto left = value->getLeft();
    if (!left) {
        std::cerr << "EXPRESSION HAD NO LEFT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeLeft = this->processArgInstr(left);
    if (removeLeft && this->_nodeBuffer.back().size() == 0) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #arg leaves expression without left side value. Please provide a default value.";
        LILNode::SourceLocation sl = left->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 0) {
        value->setLeft(this->_nodeBuffer.back().back());
    }
    this->_nodeBuffer.pop_back();
    
    auto right = value->getRight();
    if (!right) {
        std::cerr << "EXPRESSION HAD NO RIGHT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeRight = this->processArgInstr(right);
    if (removeRight && this->_nodeBuffer.back().size() == 0) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #arg leaves expression without right side value. Please provide a default value.";
        LILNode::SourceLocation sl = right->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 0) {
        value->setRight(this->_nodeBuffer.back().back());
    }
    this->_nodeBuffer.pop_back();
    
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILUnaryExpression> value)
{
    auto val = value->getValue();
    if (val && val->isA(InstructionTypeIf)) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(val);
        if (remove && this->_nodeBuffer.back().size() == 0) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #arg leaves unary expression without value. Please provide a default value.";
            LILNode::SourceLocation sl = val->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() > 0) {
            value->setValue(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILStringFunction> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILVarDecl> value)
{
    auto initVal = value->getInitVal();
    if (initVal) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->processArgInstr(initVal);
        if (this->_nodeBuffer.back().size() > 0) {
            value->setInitVals(this->_nodeBuffer.back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILClassDecl> value)
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
        this->processArgInstr(node);
        for (auto newNode : this->_nodeBuffer.back()) {
            resultNodes.push_back(newNode);
            newNodes.push_back(newNode);
        }
        this->_nodeBuffer.pop_back();
    }
    if (newNodes.size() > 0) {
        for (auto newNode : newNodes) {
            value->addMethod(newNode);
        }
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILObjectDefinition> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILAssignment> value)
{
    auto val = value->getValue();
    if (val) {
        if (val->isA(InstructionTypeArg))
        {
            this->_nodeBuffer.emplace_back();
            bool remove = this->processArgInstr(val);
            if (remove) {
                value->clearValue();
            }
            if (this->_nodeBuffer.back().size() > 0) {
                value->setValue(this->_nodeBuffer.back().back());
            }
            this->_nodeBuffer.pop_back();
        }
        else
        {
            this->processArgInstr(val);
        }
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILValuePath> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setNodes(resultNodes);
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILRule> value)
{
    bool hasChangesSCh = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesSCh;
    for (auto node : value->getSelectorChains()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        this->processArgInstr(rule);
    }
    
    bool hasChangesVal = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesVal;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILSelectorChain> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILSimpleSelector> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILFunctionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setBody(resultNodes);
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILFunctionCall> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setArguments(std::move(resultNodesArgs));
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILFlowControl> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setArguments(std::move(resultNodesArgs));
    }
    
    bool hasChangesThen = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesThen;
    for (auto node : value->getThen()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setThen(std::move(resultNodesThen));
    }
    
    bool hasChangesElse = false;
    
    std::vector<std::shared_ptr<LILNode>> resultNodesElse;
    for (auto node : value->getElse()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setElse(std::move(resultNodesElse));
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILFlowControlCall> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processArgInstr(arg);
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILInstruction> value)
{
    if (value->isA(InstructionTypeArg)) {
        const auto & children = value->getChildNodes();
        const auto & nameNode = children[0];
        const auto & nameAsgmt = std::static_pointer_cast<LILAssignment>(nameNode);
        const auto & nameStr = std::static_pointer_cast<LILStringLiteral>(nameAsgmt->getValue());
        auto name = nameStr->getValue().stripQuotes();
        if (this->hasCustomArg(name)) {
            auto customArg = this->getCustomArg(name);
            this->_nodeBuffer.back().push_back(customArg);
        } else {
            const auto & defaultNode = children[1];
            const auto & defaultAsgmt = std::static_pointer_cast<LILAssignment>(defaultNode);
            this->_nodeBuffer.back().push_back(defaultAsgmt->getValue());
        }
        return true;
        
    } else {
        auto arg = value->getArgument();
        if (arg) {
            this->_nodeBuffer.emplace_back();
            bool remove = this->processArgInstr(arg);
            if (remove && this->_nodeBuffer.back().size() == 0) {
                LILErrorMessage ei;
                ei.message =  "Evaluation of #arg leaves instruction without argument. Please provide a default value.";
                LILNode::SourceLocation sl = arg->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            } else if (this->_nodeBuffer.back().size() > 0) {
                value->setArgument(this->_nodeBuffer.back().back());
            }
            this->_nodeBuffer.pop_back();
        }
        
        bool hasChanges = false;
        std::vector<std::shared_ptr<LILNode>> resultNodes;
        for (auto node : value->getChildNodes()) {
            this->_nodeBuffer.emplace_back();
            bool remove = this->processArgInstr(node);
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
            value->setChildNodes(std::move(resultNodes));
        }
        return false;
    }
    return false;
}

bool LILArgResolver::_processArgInstrIfInstr(std::shared_ptr<LILIfInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(arg);
        if (remove && this->_nodeBuffer.back().size() == 0) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #arg leaves instruction without argument. Please provide a default value.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() > 0) {
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    
    bool hasChangesThen = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesThen;
    for (auto node : value->getThen()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setThen(std::move(resultNodesThen));
    }
    
    bool hasChangesElse = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesElse;
    for (auto node : value->getElse()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setElse(std::move(resultNodesElse));
    }
    return false;
}

bool LILArgResolver::_processArgInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(arg);
        if (remove && this->_nodeBuffer.back().size() == 0) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #arg leaves instruction without argument. Please provide a default value.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() > 0) {
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setBody(std::move(resultNodes));
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILValueList> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setValues(resultNodes);
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILIndexAccessor> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processArgInstr(arg);
    }
    return false;
}

bool LILArgResolver::_processArgInstr(std::shared_ptr<LILConversionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processArgInstr(node);
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
        value->setBody(std::move(resultNodes));
    }
    return false;
}
