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
 *      This file represents an assignment
 *
 ********************************************************************/

#include "LILAssignment.h"

using namespace LIL;

LILAssignment::LILAssignment()
: LILTypedNode(NodeTypeAssignment)
{
    
}

LILAssignment::LILAssignment(const LILAssignment &other)
: LILTypedNode(other)
{
    this->_subject = other._subject;
    this->_value = other._value;
}

std::shared_ptr<LILAssignment> LILAssignment::clone() const
{
    return std::static_pointer_cast<LILAssignment> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILAssignment::cloneImpl() const
{
    std::shared_ptr<LILAssignment> clone(new LILAssignment(*this));
    clone->clearChildNodes();
    if (this->_subject) {
        clone->setSubject(this->_subject->clone());
    }
    if (this->_value) {
        clone->_value = this->_value->clone();
    }

    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }

    return clone;
}

LILAssignment::~LILAssignment()
{
    
}

void LILAssignment::setSubject(std::shared_ptr<LILNode> pp)
{
    this->addNode(pp);
    this->_subject = pp;
}

std::shared_ptr<LILNode> LILAssignment::getSubject() const
{
    return this->_subject;
}

void LILAssignment::setValue(std::shared_ptr<LILNode> val)
{
    this->addNode(val);
    this->_value = val;
}

void LILAssignment::clearValue()
{
    this->_value.reset();
}

std::shared_ptr<LILNode> LILAssignment::getValue() const
{
    return this->_value;
}

const std::vector<std::shared_ptr<LILNode>> & LILAssignment::getNodes() const
{
    return this->getChildNodes();
}
