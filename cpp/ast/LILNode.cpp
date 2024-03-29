/********************************************************************
 *
 *	  LIL Is a Language
 *
 *	  AUTHORS: Miro Keller
 *
 *	  COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *	  LICENSE: see LICENSE file
 *
 *	  This file contains the abstract base class for all nodes
 *
 ********************************************************************/

#include "LILShared.h"
#include "LILNode.h"
#include "LILNodeTypeToString.h"
#include "LILVarNode.h"
#include "LILVisitor.h"

using namespace LIL;

LILString LILNode::nodeTypeToString(NodeType nodeType)
{
	return LILNodeTypeToString(nodeType);
}

LILNode::LILNode(NodeType type)
: nodeType(type)
, _specificity(1)
, hidden(false)
, _isExported(false)
{
	
}

LILNode::LILNode(const LILNode &orig)
: nodeType(orig.nodeType)
, _specificity(orig._specificity)
, _sourceLocation(orig._sourceLocation)
, _childNodes(orig._childNodes)
, _parentNode(orig._parentNode)
, hidden(orig.hidden)
, _isExported(orig._isExported)
{
	
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
		case NodeTypeNumberLiteral:
		case NodeTypeStringFunction:
		case NodeTypeValuePath:
		case NodeTypeRule:
		case NodeTypeType:
		case NodeTypeMultipleType:
		case NodeTypeFunctionType:
		case NodeTypePointerType:
		case NodeTypeObjectType:
		case NodeTypeStaticArrayType:
		case NodeTypeSIMDType:
		case NodeTypeVarDecl:
		case NodeTypeConstDecl:
		case NodeTypeAliasDecl:
		case NodeTypeTypeDecl:
		case NodeTypeConversionDecl:
		case NodeTypeEnum:
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
		case NodeTypeIfInstruction:
		case NodeTypeSnippetInstruction:
		case NodeTypeValueList:
		case NodeTypeIndexAccessor:
		case NodeTypeDocumentation:
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
	return nullptr;
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

bool LILNode::equalTo(std::shared_ptr<LILNode> otherNode)
{
	//check wether pointers are the same
	if (this == otherNode.get()) return true;
	//check wether of same type
	if (otherNode->nodeType != this->nodeType) return false;
	//check wether the same amount of child nodes
	size_t nodesSize = this->_childNodes.size();
	if (nodesSize != otherNode->_childNodes.size()) return false;
	//compare hidden flag
	if (otherNode->hidden != this->hidden) return false;
	//compare isExported flag
	if (otherNode->_isExported != this->_isExported) return false;
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

void LILNode::setChildNodes(const std::vector<std::shared_ptr<LILNode>> && nodes)
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

bool LILNode::isA(UnaryExpressionType otherType) const
{
	return false;
}

UnaryExpressionType LILNode::getUnaryExpressionType() const
{
	return UnaryExpressionTypeNone;
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

LILString LILNode::getHostProperty() const
{
	return this->_hostProperty;
}

void LILNode::setHostProperty(LILString newValue)
{
	this->_hostProperty = newValue;
}

void LILNode::setIsExported(bool value)
{
	this->_isExported = value;
}

bool LILNode::getIsExported() const
{
	return this->_isExported;
}
