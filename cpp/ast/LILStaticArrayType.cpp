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
 *	  This file is used to express C-style arrays
 *
 ********************************************************************/

#include "LILStaticArrayType.h"
#include "LILTypedNode.h"

using namespace LIL;

LILStaticArrayType::LILStaticArrayType()
: LILType(TypeTypeStaticArray)
, _receivesType(false)
{

}

LILStaticArrayType::LILStaticArrayType(const LILStaticArrayType &other)
: LILType(other)
, _argument(other._argument)
, _type(other._type)
, _receivesType(other._receivesType)
{

}

std::shared_ptr<LILStaticArrayType> LILStaticArrayType::clone() const
{
	return std::static_pointer_cast<LILStaticArrayType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILStaticArrayType::cloneImpl() const
{
	std::shared_ptr<LILStaticArrayType> clone(new LILStaticArrayType(*this));
	if (this->_argument) {
		clone->setArgument(this->_argument->clone());
	}
	if (this->_type) {
		clone->setType(this->_type->clone());
	}
	return clone;
}

LILStaticArrayType::~LILStaticArrayType()
{
	
}

bool LILStaticArrayType::equalTo(std::shared_ptr<LILNode> otherNode)
{
	if ( ! LILType::equalTo(otherNode)) return false;
	std::shared_ptr<LILStaticArrayType> castedNode = std::static_pointer_cast<LILStaticArrayType>(otherNode);
	if (this->_argument && !castedNode->_argument) return false;
	if (!this->_argument && castedNode->_argument) return false;
	if ( this->_argument && ! this->_argument->equalTo(castedNode->_argument) ) return false;
	if (this->_type && !castedNode->_type) return false;
	if (!this->_type && castedNode->_type) return false;
	if ( this->_type && ! this->_type->equalTo(castedNode->_type) ) return false;
	if (this->_receivesType != castedNode->_receivesType) return false;
	return true;
}

void LILStaticArrayType::receiveNodeData(const LIL::LILString &data)
{
	this->setName(data);
}

void LILStaticArrayType::setArgument(std::shared_ptr<LILNode> node)
{
	if (node->isTypedNode()) {
		auto tyNode = std::static_pointer_cast<LILTypedNode>(node);
		auto ty = tyNode->getType();
		if (ty && ty->getIsWeakType()) {
			tyNode->setType(ty->getDefaultType());
		}
	}
	this->_argument = node;
	
	this->_argument->setParentNode(this->shared_from_this());
}

std::shared_ptr<LILNode> LILStaticArrayType::getArgument() const
{
	return this->_argument;
}

void LILStaticArrayType::setType(std::shared_ptr<LILType> type)
{
	this->_type = type;
	this->_type->setParentNode(this->shared_from_this());
}

std::shared_ptr<LILType> LILStaticArrayType::getType() const
{
	return this->_type;
}

void LILStaticArrayType::setReceivesType(bool value)
{
	this->_receivesType = value;
}

bool LILStaticArrayType::getReceivesType() const
{
	return this->_receivesType;
}
