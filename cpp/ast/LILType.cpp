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
 *      This file encapsulates the type of a variable
 *
 ********************************************************************/

#include "LILType.h"
#include "LILMultipleType.h"

using namespace LIL;

std::shared_ptr<LILType> LILType::merge(std::shared_ptr<LILType> typeA, std::shared_ptr<LILType> typeB)
{
    if (!typeA && !typeB) {
        return nullptr;
    }
    if (typeA && !typeB) {
        return typeA;
    }
    if (typeB && !typeA) {
        return typeB;
    }
    if (typeA->equalTo(typeB)) {
        return typeA;
    }

    auto multiA = std::dynamic_pointer_cast<LILMultipleType>(typeA);
    auto multiB = std::dynamic_pointer_cast<LILMultipleType>(typeB);
    
    if(multiB && !multiA){
        if (multiB->getIsWeakType()) {
            bool found = false;
            for (const auto & tyB : multiB->getTypes()) {
                if (typeA->equalTo(tyB)) {
                    found = true;
                    break;
                }
            }
            if (found){
                return typeA;
            }
        }
        multiA = std::make_shared<LILMultipleType>();
        multiA->addType(typeA);
    }
    if(multiA && !multiB){
        if (multiA->getIsWeakType()) {
            bool found = false;
            for (const auto & tyA : multiA->getTypes()) {
                if (typeB->equalTo(tyA)) {
                    found = true;
                    break;
                }
            }
            if (found){
                return typeB;
            } else {
                return nullptr;
            }
        }
    }
    
    if (multiA && multiB) {
        bool aIsWeak = multiA->getIsWeakType();
        bool bIsWeak = multiB->getIsWeakType();
        if (aIsWeak == bIsWeak) {
            for (const auto & tyB : multiB->getTypes()) {
                bool found = false;
                for (const auto & tyA : multiA->getTypes()) {
                    if (tyA->equalTo(tyB)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    multiA->addType(tyB);
                }
            }
            return multiA;
        } else if (aIsWeak){
            //a is weak, but b isn't
            std::shared_ptr<LILType> ret = nullptr;
            bool foundOne = false;
            for (const auto & tyB : multiB->getTypes()) {
                for (const auto & tyA : multiA->getTypes()) {
                    if (tyA->equalTo(tyB)) {
                        if (foundOne) {
                            //more than one match -- cant merge
                            return nullptr;
                        }
                        ret = tyB;
                        foundOne = true;
                    }
                }
                
            }
            return ret;
            
        } else {
            //b is weak, but a isn't
            for (const auto & tyA : multiA->getTypes()) {
                for (const auto & tyB : multiB->getTypes()) {
                    if (tyB->equalTo(tyA)) {
                        return tyA;
                    }
                }
            }
            //if no type from b is found in a, we can't merge
            return nullptr;
        }
    } else if (multiA) {
        bool found = false;
        for (const auto & tyA : multiA->getTypes()) {
            if (tyA->equalTo(typeB)) {
                found = true;
                break;
            }
        }
        if (!found) {
            multiA->addType(typeB);
        }
        return multiA;
    }
    
    multiA = std::make_shared<LILMultipleType>();
    multiA->addType(typeA);
    multiA->addType(typeB);
    return multiA;
}

std::shared_ptr<LILType> LILType::make(LILString name)
{
    auto ret = std::make_shared<LILType>();
    ret->setTypeType(TypeTypeSingle);
    ret->setName(name);
    return ret;
}

bool LILType::isBuiltInType(LILString name)
{
    if (
        name == "i8"
        || name == "i16"
        || name == "i32"
        || name == "i64"
        || name == "i128"
        || name == "f32"
        || name == "f64"
        || name == "bool"
        || name == "null"
        || name == "i8%"
        || name == "i16%"
        || name == "i32%"
        || name == "i64%"
        || name == "f32%"
        || name == "f64%"
        ) {
        return true;
    }
    return false;
}

bool LILType::isNumberType(LILString name)
{
    if (
        name == "i8"
        || name == "i16"
        || name == "i32"
        || name == "i64"
        || name == "i128"
        || name == "f32"
        || name == "f64"
    ) {
        return true;
    }
    return false;
}

LILType::LILType()
: LIL::LILNode(NodeTypeType)
, _typeType(TypeTypeSingle)
, _isNullable(false)
{
    
}

LILType::LILType(TypeType type)
: LIL::LILNode(NodeTypeType)
, _typeType(type)
, _isNullable(false)
{
    
}

LILType::LILType(const LILType &other)
: LILNode(other)
{
    this->_name = other._name;
    this->_typeType = other._typeType;
    this->_isNullable = other._isNullable;
}

std::shared_ptr<LILType> LILType::clone() const
{
    return std::static_pointer_cast<LILType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILType::cloneImpl() const
{
    std::shared_ptr<LILType> clone(new LILType(*this));
    return clone;
}

LILType::~LILType()
{
    
}

bool LILType::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    std::shared_ptr<LILType> castedNode = std::static_pointer_cast<LILType>(otherNode);
    if ( this->_name != castedNode->_name ) return false;
    if ( this->_typeType != castedNode->_typeType ) return false;
    if ( this->_isNullable != castedNode->_isNullable )  return false;
    return true;
}

void LILType::receiveNodeData(const LIL::LILString &data)
{
    auto currentName = this->getName();
    if (currentName.length() == 0) {
        this->setName(data);
    } else {
        LILString newStr = currentName + data;
        this->setName(newStr);
    }
}

LILString LILType::stringRep()
{
    auto name = this->getName();
    if (this->getIsNullable()) {
        return name + "?";
    } else {
        return name;
    }
}

void LILType::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILType::getName() const
{
    return this->_name;
}

bool LILType::getIsWeakType() const
{
    return false;
}

TypeType LILType::getTypeType() const
{
    return this->_typeType;
}

void LILType::setTypeType(TypeType newType)
{
    this->_typeType = newType;
}

bool LILType::isA(TypeType otherType) const
{
    return this->_typeType == otherType;
}

bool LILType::getIsNullable() const
{
    return this->_isNullable;
}

void LILType::setIsNullable(bool newValue)
{
    this->_isNullable = newValue;
}
