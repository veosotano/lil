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
 *      This file encapsulates the type for a function
 *
 ********************************************************************/

#include "LILPointerType.h"

using namespace LIL;

std::shared_ptr<LILPointerType> LILPointerType::make(LILString typeName)
{
    auto ret = std::make_shared<LILPointerType>();
    ret->setName("ptr");
    auto argTy = std::make_shared<LILType>();
    argTy->setName(typeName);
    ret->setArgument(argTy);
    return ret;
}

LILPointerType::LILPointerType()
: LILType(TypeTypePointer)
{
    
}

LILPointerType::LILPointerType(const LILPointerType &other)
: LILType(other)
{
    this->_argument = other._argument;
}

std::shared_ptr<LILPointerType> LILPointerType::clone() const
{
    return std::static_pointer_cast<LILPointerType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILPointerType::cloneImpl() const
{
    std::shared_ptr<LILPointerType> clone(new LILPointerType(*this));
    if (this->_argument) {
        clone->setArgument(this->_argument->clone());
    }
    return clone;
}

LILPointerType::~LILPointerType()
{
    
}

bool LILPointerType::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILType::equalTo(otherNode)) return false;
    std::shared_ptr<LILPointerType> castedNode = std::static_pointer_cast<LILPointerType>(otherNode);
    if (this->_argument && !castedNode->_argument) return false;
    if (!this->_argument && castedNode->_argument) return false;
    if ( this->_argument && ! this->_argument->equalTo(castedNode->_argument) ) return false;
    return true;
}

void LILPointerType::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

LILString LILPointerType::stringRep()
{
    LILString name = this->getName();
    
    name += "(";
    auto arg = this->getArgument();
    if (arg) {
        name += arg->stringRep();
    }
    name += ")";
    if (this->getIsNullable()) {
        name += "?";
    }
    return name;
}

void LILPointerType::setArgument(std::shared_ptr<LILType> node)
{
    this->addNode(node);
    this->_argument = node;
}

std::shared_ptr<LILType> LILPointerType::getArgument() const
{
    return this->_argument;
}
