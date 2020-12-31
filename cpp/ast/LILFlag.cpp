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
 *      This file is a flag in a selector chain
 *
 ********************************************************************/

#include "LILFlag.h"

using namespace LIL;

LILFlag::LILFlag()
: LIL::LILNode(NodeTypeFlag)
{
    
}

LILFlag::LILFlag(const LILFlag &other)
: LILNode(other)
{
    this->_name = other._name;
}

std::shared_ptr<LILFlag> LILFlag::clone() const
{
    return std::static_pointer_cast<LILFlag> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFlag::cloneImpl() const
{
    std::shared_ptr<LILFlag> clone(new LILFlag(*this));
    return clone;
}

LILFlag::~LILFlag()
{
    
}

void LILFlag::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

void LILFlag::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILFlag::getName() const
{
    return this->_name;
}
