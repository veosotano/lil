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
 *      This file contains the abstract base class for all nodes
 *
 ********************************************************************/

#include "LILShared.h"
#include "LILNode.h"
#include "LILVarNode.h"
#include "LILVisitor.h"

using namespace LIL;

LILString LILNode::nodeTypeToString(NodeType nodeType)
{
    switch (nodeType) {
        case NodeTypeRoot:
            return "root";
        case NodeTypeNull:
            return "null";
        case NodeTypeNegation:
            return "negation";
        case NodeTypeBoolLiteral:
            return "bool";
        case NodeTypeNumberLiteral:
            return "number literal";
        case NodeTypeStringLiteral:
            return "string";
        case NodeTypeStringFunction:
            return "string function";
        case NodeTypeCStringLiteral:
            return "C string";
        case NodeTypePercentage:
            return "percentage";
        case NodeTypeExpression:
            return "expression";
        case NodeTypeUnaryExpression:
            return "unary expression";
        case NodeTypeVarName:
            return "var name";
        case NodeTypeType:
            return "type";
        case NodeTypeVarDecl:
            return "var declaration";
        case NodeTypeAssignment:
            return "assignment";
        case NodeTypeValuePath:
            return "value path";
        case NodeTypePropertyName:
            return "propertyName";
        case NodeTypeSelector:
            return "selector";
        case NodeTypeRootSelector:
            return "@root";
        case NodeTypeCombinator:
            return "combinator";
        case NodeTypeFilter:
            return "filter";
        case NodeTypeFlag:
            return "flag";
        case NodeTypeSimpleSelector:
            return "simple selector";
        case NodeTypeSelectorChain:
            return "selector chain";
        case NodeTypeRule:
            return "rule";
        case NodeTypeClassDecl:
            return "class declaration";
        case NodeTypeObjectDefinition:
            return "object definition";
        case NodeTypeComment:
            return "comment";
        case NodeTypeInstruction:
            return "instruction";
        case NodeTypeFunctionDecl:
            return "function declaration";
        case NodeTypeFunctionCall:
            return "function call";
        case NodeTypeArgument:
            return "argument";
        case NodeTypeArray:
            return "array";
        case NodeTypeIndexAccessor:
            return "index accessor";
        case NodeTypeWhitespaceNode:
            return "whitespace";
        case NodeTypeFlowControl:
            return "flow control";
        case NodeTypeFlowControlCall:
            return "flow control call";

        default:
            return "ERROR: unknown node type";
    }
}

LILNode::LILNode(NodeType type)
{
    this->nodeType = type;
    this->_specificity = 1;
    this->document = nullptr;
}

LILNode::LILNode(const LILNode &orig)
{
    this->nodeType = orig.nodeType;
    this->_specificity = orig._specificity;
    this->document = orig.document;
    this->_sourceLocation = orig._sourceLocation;
    this->_childNodes = orig._childNodes;
    this->_parentNode = orig._parentNode;
}
LILNode::~LILNode()
{

}

void LILNode::accept(LILVisitor *visitor)
{
    visitor->visit(this);
}

bool LILNode::isContainerNode(NodeType nodeType)
{
    switch (nodeType) {
        case NodeTypeStringFunction:
        case NodeTypeValuePath:
        case NodeTypeRule:
        case NodeTypeType:
        case NodeTypeMultipleType:
        case NodeTypeFunctionType:
        case NodeTypePointerType:
        case NodeTypeObjectType:
        case NodeTypeVarDecl:
        case NodeTypeAssignment:
        case NodeTypeExpression:
        case NodeTypeUnaryExpression:
        case NodeTypeClassDecl:
        case NodeTypeObjectDefinition:
        case NodeTypeFunctionCall:
        case NodeTypeFunctionDecl:
        case NodeTypeFlowControl:
        case NodeTypeFlowControlCall:
        case NodeTypeSimpleSelector:
        case NodeTypeSelectorChain:
        case NodeTypeInstruction:
            return true;
        default:
            return false;
    }
    return false;

}

void LILNode::receiveNodeData(const LIL::LILString &data)
{

}

bool LILNode::isVarNode() const
{
    return false;
}

std::shared_ptr<LILVarNode> LILNode::getClosestVarNode() const
{
    std::shared_ptr<LILVarNode> ret;
    std::shared_ptr<LILNode> parent = this->getParentNode();
    if (!parent) {
        return ret;
    }
    ret = std::dynamic_pointer_cast<LILVarNode>(parent);
    if (ret)
        return ret;
    
    bool done = false;
    while (!done) {
        done = true;
        
        parent = parent->getParentNode();
        if (parent)
        {
            done = false;
        }
        else
        {
            return ret;
        }
        ret = std::dynamic_pointer_cast<LILVarNode>(parent);
        if (ret)
            return ret;
    }
    return ret;
}

bool LILNode::isRootNode() const
{
    return false;
}

bool LILNode::isTypedNode() const
{
    return false;
}

std::shared_ptr<LILType> LILNode::getType() const
{
    const auto & parent = this->getParentNode();
    if (parent) {
        return parent->getType();
    } else {
        return nullptr;
    }
}

void LILNode::setSourceLocation(LILNode::SourceLocation loc)
{
    this->_sourceLocation = loc;
}

LILNode::SourceLocation LILNode::getSourceLocation() const
{
    return this->_sourceLocation;
}

std::shared_ptr<LILNode> LILNode::clone() const
{
    return std::static_pointer_cast<LILNode> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILNode::cloneImpl() const
{
    return std::shared_ptr<LILNode>(new LILNode(*this));
}

void LILNode::cloneChildNodes(std::shared_ptr<LILNode> clone) const
{
    clone->clearChildNodes();
    for (auto child : this->getChildNodes()) {
        clone->addNode(child->clone());
    }
}

LILString LILNode::stringRep()
{
    return "Generic node - you forgot to override stringRep in your subclass or somehow using LILNode directly";
}

std::string LILNode::stdStringRep()
{
    LILString tempstr = this->stringRep();
    return tempstr.data();
}

bool LILNode::equalTo(std::shared_ptr<LILNode> otherNode)
{
    //check wether pointers are the same
    if (this == otherNode.get()) return true;
    //check wether of same type
    if (otherNode->nodeType != this->nodeType) return false;
    //check wether the same amount of child nodes
    size_t nodesSize = this->_childNodes.size();
    if (nodesSize != otherNode->_childNodes.size()) return false;
    //compare the child nodes
    for (size_t i = 0; i<nodesSize; ++i)
    {
        if ( ! this->_childNodes[i]->equalTo(otherNode->_childNodes[i])) return false;
    }
    return true;
}

bool LILNode::isA(NodeType otherType) const
{
    return otherType == this->nodeType;
}

NodeType LILNode::getNodeType() const
{
    return this->nodeType;
}

std::shared_ptr<LILNode> LILNode::getParentNode() const
{
    if (!this->_parentNode.expired())
    {
        std::shared_ptr<LILNode> parent = this->_parentNode.lock();
        return parent;
    }
    else
    {
        return std::shared_ptr<LILNode>();
    }
}

void LILNode::setParentNode(std::shared_ptr<LILNode> newParent)
{
    this->_parentNode = newParent;
}

void LILNode::removeFromParentNode()
{
    std::shared_ptr<LILNode> parentNode = this->getParentNode();
    if (parentNode) parentNode->removeNode(this->shared_from_this());
}

void LILNode::addNode(std::shared_ptr<LILNode> child)
{
    child->setParentNode(this->shared_from_this());
    this->_childNodes.push_back(child);
}

void LILNode::removeNode(std::shared_ptr<LILNode> child)
{
    auto it = find(this->_childNodes.begin(), this->_childNodes.end(), child);
    if (it != this->_childNodes.end())
    {
        this->_childNodes.erase(it);
    }
}

void LILNode::prependNode(std::shared_ptr<LILNode> child)
{
    child->setParentNode(this->shared_from_this());
    std::vector<std::shared_ptr<LILNode>> newVector;
    newVector.push_back(child);
    for (auto existing : this->_childNodes) {
        newVector.push_back(existing);
    }
    this->_childNodes = newVector;
}

const std::vector<std::shared_ptr<LILNode> > & LILNode::getChildNodes() const
{
    return this->_childNodes;
}

void LILNode::setChildNodes(std::vector<std::shared_ptr<LILNode>> nodes)
{
    this->_childNodes = std::move(nodes);
    for (auto node : this->_childNodes) {
        node->setParentNode(this->shared_from_this());
    }
}

void LILNode::clearChildNodes()
{
    this->_childNodes.clear();
}

bool LILNode::isA(ExpressionType otherType) const
{
    return false;
}

ExpressionType LILNode::getExpressionType() const
{
    return ExpressionTypeNone;
}

bool LILNode::isA(InstructionType otherType) const
{
    return false;
}

InstructionType LILNode::getInstructionType() const
{
    return InstructionTypeNone;
}

bool LILNode::isA(SelectorType otherType) const
{
    return false;
}

SelectorType LILNode::getSelectorType() const
{
    return SelectorTypeNone;
}

bool LILNode::isA(CombinatorType otherType) const
{
    return false;
}

CombinatorType LILNode::getCombinatorType() const
{
    return CombinatorTypeNone;
}

bool LILNode::isA(FlagFunctionType otherType) const
{
    return false;
}

FlagFunctionType LILNode::getFlagFunctionType() const
{
    return FlagFunctionTypeNone;
}

bool LILNode::isA(EventType otherType) const
{
    return false;
}

EventType LILNode::getEventType() const
{
    return EventTypeNone;
}

bool LILNode::isA(SelectionType otherType) const
{
    return false;
}

SelectionType LILNode::getSelectionType() const
{
    return SelectionTypeNone;
}

bool LILNode::isA(FunctionCallType otherType) const
{
    return false;
}

FunctionCallType LILNode::getFunctionCallType() const
{
    return FunctionCallTypeNone;
}

bool LILNode::isA(FlowControlType otherType) const
{
    return false;
}

FlowControlType LILNode::getFlowControlType() const
{
    return FlowControlTypeNone;
}

bool LILNode::isA(FlowControlCallType otherType) const
{
    return false;
}

FlowControlCallType LILNode::getFlowControlCallType() const
{
    return FlowControlCallTypeNone;
}

bool LILNode::isA(TypeType otherType) const
{
    return false;
}

TypeType LILNode::getTypeType() const
{
    return TypeTypeNone;
}

LILDocument * LILNode::getDocument() const
{
    if (document != nullptr) {
        return document;
    }
    std::shared_ptr<LILNode> parent = this->getParentNode();
    if (parent)
    {
        return parent->getDocument();
    }
    return nullptr;
}

void LILNode::setDocument(LILDocument * newDoc)
{
    this->document = newDoc;
}

LILString LILNode::getHostProperty() const
{
    return this->_hostProperty;
}

void LILNode::setHostProperty(LILString newValue)
{
    this->_hostProperty = newValue;
}
