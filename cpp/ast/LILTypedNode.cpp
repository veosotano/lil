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
 *	  This is a node that participates in local variables
 *
 ********************************************************************/

#include "LILTypedNode.h"

using namespace LIL;

LILTypedNode::LILTypedNode(NodeType type)
: LILNode(type)
{
	
}

LILTypedNode::LILTypedNode(const LILTypedNode & other)
: LILNode(other)
{
	this->_type = other._type;
}

LILTypedNode::~LILTypedNode()
{
	
}

bool LILTypedNode::isTypedNode() const
{
	return true;
}

bool LILTypedNode::equalTo(std::shared_ptr<LILNode> otherNode)
{
	if ( ! LILNode::equalTo(otherNode)) return false;
	std::shared_ptr<LILTypedNode> castedNode = std::static_pointer_cast<LILTypedNode>(otherNode);
	if ( (this->_type && castedNode->_type) && ( ! this->_type->equalTo(castedNode->_type) ) ) return false;
	return true;
}

void LILTypedNode::setType(std::shared_ptr<LILType> value)
{
	this->_type = value;
	this->_type->setParentNode(shared_from_this());
}

std::shared_ptr<LILType> LILTypedNode::getType() const
{
	return this->_type;
}
