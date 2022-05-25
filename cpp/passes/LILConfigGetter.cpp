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


#include "LILConfigGetter.h"
#include "LILASTBuilder.h"
#include "LILCodeParser.h"
#include "LILConfiguration.h"
#include "LILNodeToString.h"
#include "LILRootNode.h"
#include "LILStringLiteral.h"
#include "LILVarName.h"

using namespace LIL;

LILConfigGetter::LILConfigGetter()
: _config(nullptr)
{
}

LILConfigGetter::~LILConfigGetter()
{
}

void LILConfigGetter::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  GETTING CONFIG  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILConfigGetter::visit(LILNode *node)
{
    
}

void LILConfigGetter::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    for (auto node : nodes) {
        this->processGetConfigInstr(node);
    }
    for (auto node : rootNode->getInitializers()) {
        this->processGetConfigInstr(node);
    }
}

bool LILConfigGetter::processGetConfigInstr(std::shared_ptr<LILNode> node)
{
    if (this->getDebug()) {
        std::cerr << "##  processing #getConfig instructions in " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }
    bool ret = false;
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
        case NodeTypeFlag:
        case NodeTypeFilter:
        case NodeTypeSelector:
        case NodeTypeCombinator:
        case NodeTypeForeignLang:
        case NodeTypeComment:
        case NodeTypeInvalid:
        case NodeTypeDocumentation:
        case NodeTypeSIMDType:
            //do nothing
            break;
            
        case NodeTypeNegation:
            std::cerr << "UNIMPLEMENTED FAIL !!!!!\n\n";
            ret = false;
            break;
            
        case NodeTypeExpression:
        {
            auto value = std::static_pointer_cast<LILExpression>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeUnaryExpression:
        {
            auto value = std::static_pointer_cast<LILUnaryExpression>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeStringFunction:
        {
            auto value = std::static_pointer_cast<LILStringFunction>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeVarDecl:
        case NodeTypeConstDecl:
        {
            auto value = std::static_pointer_cast<LILVarDecl>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeConversionDecl:
        {
            auto value = std::static_pointer_cast<LILConversionDecl>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeClassDecl:
        {
            auto value = std::static_pointer_cast<LILClassDecl>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeObjectDefinition:
        {
            auto value = std::static_pointer_cast<LILObjectDefinition>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeAssignment:
        {
            auto value = std::static_pointer_cast<LILAssignment>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeRule:
        {
            auto value = std::static_pointer_cast<LILRule>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeSelectorChain:
        {
            auto value = std::static_pointer_cast<LILSelectorChain>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            auto value = std::static_pointer_cast<LILFunctionDecl>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeFunctionCall:
        {
            auto value = std::static_pointer_cast<LILFunctionCall>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeFlowControl:
        {
            auto value = std::static_pointer_cast<LILFlowControl>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeFlowControlCall:
        {
            auto value = std::static_pointer_cast<LILFlowControlCall>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeInstruction:
        {
            auto value = std::static_pointer_cast<LILInstruction>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeIfInstruction:
        {
            auto value = std::static_pointer_cast<LILIfInstruction>(node);
            ret = this->_processGetConfigInstrIfInstr(value);
            break;
        }
        case NodeTypeSnippetInstruction:
        {
            auto value = std::static_pointer_cast<LILSnippetInstruction>(node);
            ret = this->_processGetConfigInstrSnippetInstr(value);
            break;
        }
        case NodeTypeValueList:
        {
            auto value = std::static_pointer_cast<LILValueList>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeIndexAccessor:
        {
            auto value = std::static_pointer_cast<LILIndexAccessor>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeValuePath:
        {
            auto value = std::static_pointer_cast<LILValuePath>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeSimpleSelector:
        {
            auto value = std::static_pointer_cast<LILSimpleSelector>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
        case NodeTypeType:
        case NodeTypeMultipleType:
        case NodeTypeFunctionType:
        case NodeTypeObjectType:
        case NodeTypePointerType:
        case NodeTypeStaticArrayType:
        {
            auto ty = std::static_pointer_cast<LILType>(node);
            this->_processGetConfigInstr(ty);
            
            switch (node->getTypeType()) {
                case TypeTypeStaticArray:
                {
                    auto value = std::static_pointer_cast<LILStaticArrayType>(node);
                    ret = this->_processGetConfigInstr(value);
                    break;
                }
                case TypeTypeMultiple:
                {
                    auto value = std::static_pointer_cast<LILMultipleType>(node);
                    ret = this->_processGetConfigInstr(value);
                    break;
                }
                case TypeTypePointer:
                {
                    auto value = std::static_pointer_cast<LILPointerType>(node);
                    ret = this->_processGetConfigInstr(value);
                    break;
                }
                case TypeTypeFunction:
                {
                    auto value = std::static_pointer_cast<LILFunctionType>(node);
                    ret = this->_processGetConfigInstr(value);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case NodeTypeEnum:
        {
            auto value = std::static_pointer_cast<LILEnum>(node);
            ret = this->_processGetConfigInstr(value);
            break;
        }
    }
    if (node->isTypedNode()) {
        auto tyNode = std::static_pointer_cast<LILTypedNode>(node);
        auto ty = tyNode->getType();
        if (ty) {
            this->processGetConfigInstr(ty);
        }
    }
    return ret;
}

void LILConfigGetter::setConfiguration(LILConfiguration * value)
{
    this->_config = value;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILExpression> value)
{
    auto left = value->getLeft();
    if (!left) {
        std::cerr << "EXPRESSION HAD NO LEFT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeLeft = this->processGetConfigInstr(left);
    if (removeLeft && this->_nodeBuffer.back().size() > 0) {
        value->setLeft(this->_nodeBuffer.back().back());
    }
    this->_nodeBuffer.pop_back();
    
    auto right = value->getRight();
    if (!right) {
        std::cerr << "EXPRESSION HAD NO RIGHT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeRight = this->processGetConfigInstr(right);
    if (removeRight && this->_nodeBuffer.back().size() > 0) {
        value->setRight(this->_nodeBuffer.back().back());
    }
    this->_nodeBuffer.pop_back();
    
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILUnaryExpression> value)
{
    auto val = value->getValue();
    if (val && val->isA(InstructionTypeIf)) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(val);
        if (remove && this->_nodeBuffer.back().size() > 0) {
            value->setValue(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILStringFunction> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILVarDecl> value)
{
    auto initVal = value->getInitVal();
    if (initVal) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->processGetConfigInstr(initVal);
        if (this->_nodeBuffer.back().size() > 0) {
            value->setInitVals(this->_nodeBuffer.back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILClassDecl> value)
{
    if (value->getIsExtern()) {
        return false;
    }
    
    std::vector<std::shared_ptr<LILNode>> newNodes;

    for (const auto & methodPair : value->getMethods()) {
        this->processGetConfigInstr(methodPair.second);
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILObjectDefinition> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILAssignment> value)
{
    auto val = value->getValue();
    if (val) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(val);
        if (remove) {
            value->clearValue();
        }
        if (this->_nodeBuffer.back().size() > 0) {
            value->setValue(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILValuePath> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILRule> value)
{
    const auto & node = value->getSelectorChain();
    if (node) {
        this->_nodeBuffer.emplace_back();
        this->processGetConfigInstr(node);
        if (this->_nodeBuffer.back().size() > 0) {
            value->setSelectorChain(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }

    for (auto rule : value->getChildRules()) {
        this->processGetConfigInstr(rule);
    }
    
    bool hasChangesVal = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesVal;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILSelectorChain> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILSimpleSelector> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILFunctionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILFunctionCall> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILFlowControl> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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
        bool remove = this->processGetConfigInstr(node);
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
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILFlowControlCall> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processGetConfigInstr(arg);
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILInstruction> value)
{
    if (value->isA(InstructionTypeGetConfig)) {
        const auto & arg = value->getArgument();
        if (arg && arg->isA(NodeTypeVarName))
        {
            if (this->_config) {
                auto vn = std::static_pointer_cast<LILVarName>(arg);
                auto name = vn->getName().data();
                auto & configVals = this->_config->getConfigItems(name);
                if (configVals.size() > 0) {
                    this->_nodeBuffer.back().push_back(configVals.back());
                    return true;
                }
            }
        }
        else
        {
            LILErrorMessage ei;
            ei.message =  "Unkonwn node as argument of #getConfig instruction.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        }
        return false;
        
    } else {
        auto arg = value->getArgument();
        if (arg) {
            this->_nodeBuffer.emplace_back();
            bool remove = this->processGetConfigInstr(arg);
            if (remove && this->_nodeBuffer.back().size() > 0) {
                value->setArgument(this->_nodeBuffer.back().back());
            }
            this->_nodeBuffer.pop_back();
        }
        
        bool hasChanges = false;
        std::vector<std::shared_ptr<LILNode>> resultNodes;
        for (auto node : value->getChildNodes()) {
            this->_nodeBuffer.emplace_back();
            bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstrIfInstr(std::shared_ptr<LILIfInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(arg);
        if (remove && this->_nodeBuffer.back().size() > 0) {
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    
    bool hasChangesThen = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesThen;
    for (auto node : value->getThen()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(arg);
        if (remove && this->_nodeBuffer.back().size() > 0) {
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILValueList> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILIndexAccessor> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processGetConfigInstr(arg);
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILConversionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILType> value)
{
    bool hasChangesTypes = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getTmplParams()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesTypes = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesTypes) {
        value->setTmplParams(std::move(resultNodes));
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILStaticArrayType> value)
{
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(arg);
        if (remove && this->_nodeBuffer.back().size() > 0) {
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILMultipleType> value)
{
    bool hasChangesTypes = false;
    std::vector<std::shared_ptr<LILType>> resultNodes;
    for (auto node : value->getTypes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesTypes = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                if (newNode->isA(NodeTypeType)) {
                    resultNodes.push_back(std::static_pointer_cast<LILType>(newNode));
                }
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesTypes) {
        value->setTypes(std::move(resultNodes));
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILPointerType> value)
{
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(arg);
        if (remove && this->_nodeBuffer.back().size() > 0) {
            auto tyNode = this->_nodeBuffer.back().back();
            if (tyNode->isA(NodeTypeType)) {
                auto ty = std::static_pointer_cast<LILType>(tyNode);
                value->setArgument(ty);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILFunctionType> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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
        value->setArguments(resultNodesArgs);
    }

    auto returnTy = value->getReturnType();
    if (returnTy) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(returnTy);
        if (remove && this->_nodeBuffer.back().size() > 0) {
            auto tyNode = this->_nodeBuffer.back().back();
            if (tyNode->isA(NodeTypeType)) {
                auto ty = std::static_pointer_cast<LILType>(tyNode);
                value->setReturnType(ty);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILConfigGetter::_processGetConfigInstr(std::shared_ptr<LILEnum> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processGetConfigInstr(node);
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
