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
 *	  This file represents a null value in the source code
 *
 ********************************************************************/

#include "LILNullLiteral.h"

using namespace LIL;

LILNullLiteral::LILNullLiteral()
: LILTypedNode(NodeTypeNull)
{
	
}

LILNullLiteral::LILNullLiteral(const LILNullLiteral & other)
: LILTypedNode(other)
{

}

std::shared_ptr<LILNullLiteral> LILNullLiteral::clone() const
{
	return std::static_pointer_cast<LILNullLiteral> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILNullLiteral::cloneImpl() const
{
	std::shared_ptr<LILNullLiteral> clone(new LILNullLiteral(*this));
	//clone LILTypedNode
	if (this->_type) {
		clone->setType(this->_type->clone());
	}
	return clone;
}

LILNullLiteral::~LILNullLiteral()
{
	
}

void LILNullLiteral::receiveNodeData(const LIL::LILString &data)
{
	
}

bool LILNullLiteral::equalTo(std::shared_ptr<LILNode> otherNode)
{
	if ( ! LILTypedNode::equalTo(otherNode)) return false;
	std::shared_ptr<LILNullLiteral> castedNode = std::static_pointer_cast<LILNullLiteral>(otherNode);
	return true;
}
