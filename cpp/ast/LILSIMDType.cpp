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
 *      This file is used to represent "vector types"
 *
 ********************************************************************/

#include "LILSIMDType.h"
#include "LILTypedNode.h"

using namespace LIL;

LILSIMDType::LILSIMDType()
: LILType(TypeTypeSIMD)
, _width(1)
{
    
}

LILSIMDType::LILSIMDType(const LILSIMDType &other)
: LILType(other)
, _width(other._width)
{

}

std::shared_ptr<LILSIMDType> LILSIMDType::clone() const
{
    return std::static_pointer_cast<LILSIMDType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILSIMDType::cloneImpl() const
{
    std::shared_ptr<LILSIMDType> clone(new LILSIMDType(*this));
    return clone;
}

LILSIMDType::~LILSIMDType()
{
    
}

bool LILSIMDType::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILType::equalTo(otherNode)) return false;
    std::shared_ptr<LILSIMDType> castedNode = std::static_pointer_cast<LILSIMDType>(otherNode);
    if (this->_width != castedNode->_width) return false;
    if (this->_type && !castedNode->_type) return false;
    if (!this->_type && castedNode->_type) return false;
    if ( this->_type && ! this->_type->equalTo(castedNode->_type) ) return false;
    return true;
}

void LILSIMDType::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

void LILSIMDType::setWidth(unsigned int value)
{
    this->_width = value;
}

unsigned int LILSIMDType::getWidth() const
{
    return this->_width;
}

void LILSIMDType::setType(std::shared_ptr<LILType> type)
{
    this->_type = type;
    this->_type->setParentNode(this->shared_from_this());
}

std::shared_ptr<LILType> LILSIMDType::getType() const
{
    return this->_type;
}
