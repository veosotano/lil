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
 *      This file is used to express C-style arrays
 *
 ********************************************************************/

#include "LILStaticArrayType.h"

using namespace LIL;

LILStaticArrayType::LILStaticArrayType()
: LILType(TypeTypeStaticArray)
{
    
}

LILStaticArrayType::LILStaticArrayType(const LILStaticArrayType &other)
: LILType(other)
{
    this->_argument = other._argument;
    this->_type = other._type;
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
    return true;
}

void LILStaticArrayType::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

void LILStaticArrayType::setArgument(std::shared_ptr<LILNode> node)
{
    this->_argument = node;
}

std::shared_ptr<LILNode> LILStaticArrayType::getArgument() const
{
    return this->_argument;
}

void LILStaticArrayType::setType(std::shared_ptr<LILType> type)
{
    this->_type = type;
}

std::shared_ptr<LILType> LILStaticArrayType::getType() const
{
    return this->_type;
}
