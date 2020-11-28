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
 *      This file holds the node used to access into an array
 *
 ********************************************************************/

#include "LILIndexAccessor.h"

using namespace LIL;

LILIndexAccessor::LILIndexAccessor()
: LILTypedNode(NodeTypeIndexAccessor)
{
    
}

LILIndexAccessor::LILIndexAccessor(const LILIndexAccessor &other)
: LILTypedNode(other)
, _argument(other._argument)
{
    
}

std::shared_ptr<LILIndexAccessor> LILIndexAccessor::clone() const
{
    return std::static_pointer_cast<LILIndexAccessor> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILIndexAccessor::cloneImpl() const
{
    std::shared_ptr<LILIndexAccessor> clone(new LILIndexAccessor(*this));
    LILNode::cloneChildNodes(clone);
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    if (this->_argument) {
        clone->setArgument(this->_argument->clone());
    }
    return clone;
}

LILIndexAccessor::~LILIndexAccessor()
{
    
}

const std::shared_ptr<LILNode> & LILIndexAccessor::getArgument() const
{
    return this->_argument;
}

void LILIndexAccessor::setArgument(std::shared_ptr<LILNode> newValue)
{
    this->addNode(newValue);
    this->_argument = newValue;
}
