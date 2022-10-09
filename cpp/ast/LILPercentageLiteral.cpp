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
 *	  This file represents a percentage written in the source code
 *
 ********************************************************************/

#include "LILPercentageLiteral.h"

using namespace LIL;

LILPercentageLiteral::LILPercentageLiteral()
: LILTypedNode( NodeTypePercentage )
{

}

LILPercentageLiteral::LILPercentageLiteral(const LILPercentageLiteral & other)
: LILTypedNode(other)
{
	this->_value = other._value;
}

std::shared_ptr<LILPercentageLiteral> LILPercentageLiteral::clone() const
{
	return std::static_pointer_cast<LILPercentageLiteral> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILPercentageLiteral::cloneImpl() const
{
	std::shared_ptr<LILPercentageLiteral> clone(new LILPercentageLiteral(*this));
	//clone LILTypedNode
	if (this->_type) {
		clone->setType(this->_type->clone());
	}
	return clone;
}

LILPercentageLiteral::~LILPercentageLiteral()
{
	
}

void LILPercentageLiteral::receiveNodeData(const LIL::LILString &data)
{
	this->setValue(data);
}

bool LILPercentageLiteral::equalTo(std::shared_ptr<LILNode> otherNode)
{
	if ( ! LILTypedNode::equalTo(otherNode)) return false;
	std::shared_ptr<LILPercentageLiteral> castedNode = std::static_pointer_cast<LILPercentageLiteral>(otherNode);
	if ( this->_value != castedNode->_value ) return false;
	return true;
}

LILString LILPercentageLiteral::getValue() const
{
	return this->_value;
}

void LILPercentageLiteral::setValue(LILString newValue)
{
	this->_value = newValue;
}
