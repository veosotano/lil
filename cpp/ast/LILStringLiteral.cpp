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
 *      This file represents a string written in the source code
 *
 ********************************************************************/

#include "LILStringLiteral.h"
#include "LILObjectType.h"
#include "LILPointerType.h"

using namespace LIL;

LILStringLiteral::LILStringLiteral()
: LILNode(NodeTypeStringLiteral)
, _isCStr(false)
{

}

LILStringLiteral::LILStringLiteral(const LILStringLiteral & other)
: LILNode(other)
{
    this->_value = other._value;
    this->_isCStr = other._isCStr;
}

std::shared_ptr<LILStringLiteral> LILStringLiteral::clone() const
{
    return std::static_pointer_cast<LILStringLiteral> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILStringLiteral::cloneImpl() const
{
    return std::shared_ptr<LILStringLiteral>(new LILStringLiteral(*this));
}

LILStringLiteral::~LILStringLiteral()
{
    
}

void LILStringLiteral::receiveNodeData(const LIL::LILString &data)
{
    this->setValue(data);
}

void LILStringLiteral::setValue(LILString newValue)
{
    this->_value = newValue;
}

LILString LILStringLiteral::getValue()
{
    return this->_value;
}

bool LILStringLiteral::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    std::shared_ptr<LILStringLiteral> castedNode = std::static_pointer_cast<LILStringLiteral>(otherNode);
    if ( this->_value != castedNode->_value ) return false;
    if ( this->_isCStr != castedNode->_isCStr ) return false;
    return true;
}

void LILStringLiteral::setIsCString(bool value)
{
    this->_isCStr = value;
}

bool LILStringLiteral::getIsCString() const
{
    return this->_isCStr;
}

std::shared_ptr<LILType> LILStringLiteral::getType() const
{
    if (this->getIsCString())
    {
        static std::shared_ptr<LILPointerType> cStrTy;
        if (!cStrTy) {
            cStrTy = std::make_shared<LILPointerType>();
            cStrTy->setName("ptr");
            auto charTy = std::make_shared<LILType>();
            charTy->setName("i8");
            cStrTy->setArgument(charTy);
        }
        return cStrTy;
    }
    else
    {
        static std::shared_ptr<LILObjectType> strTy;
        if (!strTy) {
            strTy = std::make_shared<LILObjectType>();
            strTy->setName("string");
        }
        return strTy;
    }
}
