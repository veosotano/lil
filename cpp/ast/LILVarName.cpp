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
: LILTypedNode(NodeTypeVarName)
{
    
}

LILVarName::LILVarName(const LILVarName &other)
: LILTypedNode(other)
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
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILVarName::~LILVarName()
{
    
}

void LILVarName::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

bool LILVarName::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    auto castedNode = std::static_pointer_cast<LILVarName>(otherNode);
    if ( this->_name != castedNode->_name) return false;
    return true;
}

void LILVarName::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILVarName::getName() const
{
    return this->_name;
}
