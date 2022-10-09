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
 *	  This file encapsulates the name of a property
 *
 ********************************************************************/

#include "LILPropertyName.h"

using namespace LIL;

LILPropertyName::LILPropertyName()
: LIL::LILNode(NodeTypePropertyName)
{
	
}

LILPropertyName::LILPropertyName(const LILPropertyName &other)
: LILNode(other)
{
	this->_name = other._name;
}

std::shared_ptr<LILPropertyName> LILPropertyName::clone() const
{
	return std::static_pointer_cast<LILPropertyName> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILPropertyName::cloneImpl() const
{
	std::shared_ptr<LILPropertyName> clone(new LILPropertyName(*this));
	return clone;
}

LILPropertyName::~LILPropertyName()
{
	
}

void LILPropertyName::receiveNodeData(const LIL::LILString &data)
{
	this->setName(data);
}

bool LILPropertyName::equalTo(std::shared_ptr<LILNode> otherNode)
{
	if ( ! LILNode::equalTo(otherNode)) return false;
	auto castedNode = std::static_pointer_cast<LILPropertyName>(otherNode);
	if ( this->_name != castedNode->_name) return false;
	return true;
}

void LILPropertyName::setName(LILString newName)
{
	this->_name = newName;
}

const LILString LILPropertyName::getName() const
{
	return this->_name;
}
