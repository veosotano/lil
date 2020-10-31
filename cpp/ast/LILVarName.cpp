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
 *      This file encapsulates the name of a property
 *
 ********************************************************************/

#include "LILVarName.h"
#include "LILVarNode.h"

using namespace LIL;

LILVarName::LILVarName()
: LIL::LILNode(NodeTypeVarName)
{
    
}

LILVarName::LILVarName(const LILVarName &other)
: LILNode(other)
{
    this->_name = other._name;
}

std::shared_ptr<LILVarName> LILVarName::clone() const
{
    return std::static_pointer_cast<LILVarName> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILVarName::cloneImpl() const
{
    std::shared_ptr<LILVarName> clone(new LILVarName(*this));
    return clone;
}

LILVarName::~LILVarName()
{
    
}

void LILVarName::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

LILString LILVarName::stringRep()
{
    return this->_name;
}

void LILVarName::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILVarName::getName() const
{
    return this->_name;
}

std::shared_ptr<LILType> LILVarName::getType() const
{
    auto varNode = this->getClosestVarNode();
    if (varNode) {
        auto remoteNode = varNode->getVar(this->getName());
        if (remoteNode) {
            return remoteNode->getType();
        }
    }
    return nullptr;
}
