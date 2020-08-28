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
 *      This file encapsulates multiple types for one variable
 *
 ********************************************************************/

#include "LILMultipleType.h"

using namespace LIL;

LILMultipleType::LILMultipleType()
: LIL::LILType(TypeTypeMultiple)
, _isWeakType(false)
{
    
}

LILMultipleType::LILMultipleType(const LILMultipleType &other)
: LILType(other)
{
    this->_types = other._types;
    this->_isWeakType = other._isWeakType;
}

std::shared_ptr<LILMultipleType> LILMultipleType::clone() const
{
    return std::static_pointer_cast<LILMultipleType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILMultipleType::cloneImpl() const
{
    std::shared_ptr<LILMultipleType> clone(new LILMultipleType(*this));
    clone->_types.clear();
    for (const auto & ty : this->_types) {
        clone->addType(ty->clone());
    }
    return clone;
}

LILMultipleType::~LILMultipleType()
{
    
}

bool LILMultipleType::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILType::equalTo(otherNode)) return false;
    std::shared_ptr<LILMultipleType> castedNode = std::static_pointer_cast<LILMultipleType>(otherNode);
    if (this->_types.size() != castedNode->_types.size()) return false;
    for (size_t i=0, j=this->_types.size(); i<j; ++i) {
        if (!this->_types[i]->equalTo(castedNode->_types[i])) return false;
    }
    return true;
}

void LILMultipleType::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

LILString LILMultipleType::stringRep()
{
    LILString tempstr;
    for (size_t i=0, j=this->_types.size(); i<j; ++i) {
        tempstr += this->_types[i]->stringRep();
        if (i<j-1) {
            tempstr += "|";
        }
    }
    if (this->getIsWeakType()) {
        return "<"+tempstr+">";
    } else {
        return tempstr;
    }
}

void LILMultipleType::addType(std::shared_ptr<LILType> ty)
{
    this->_types.push_back(ty);
}

std::vector<std::shared_ptr<LILType>> LILMultipleType::getTypes() const
{
    return this->_types;
}

bool LILMultipleType::getIsWeakType() const
{
    return this->_isWeakType;
}

void LILMultipleType::setIsWeakType(bool value)
{
    this->_isWeakType = value;
}
