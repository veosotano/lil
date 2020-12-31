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
 *      This file represents a number written in the source code
 *
 ********************************************************************/

#include "LILNumberLiteral.h"

using namespace LIL;

LILNumberLiteral::LILNumberLiteral()
: LILTypedNode(NodeTypeNumberLiteral)
{
    
}

LILNumberLiteral::LILNumberLiteral(const LILNumberLiteral & other)
: LILTypedNode(other)
{
    this->_value = other._value;
}

std::shared_ptr<LILNumberLiteral> LILNumberLiteral::clone() const
{
    return std::static_pointer_cast<LILNumberLiteral> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILNumberLiteral::cloneImpl() const
{
    std::shared_ptr<LILNumberLiteral> clone(new LILNumberLiteral(*this));
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILNumberLiteral::~LILNumberLiteral()
{

}

void LILNumberLiteral::receiveNodeData(const LIL::LILString &data)
{
    this->setValue(data);
}

bool LILNumberLiteral::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILTypedNode::equalTo(otherNode)) return false;
    std::shared_ptr<LILNumberLiteral> castedNode = std::static_pointer_cast<LILNumberLiteral>(otherNode);
    if ( this->_value != castedNode->_value ) return false;
    return true;
}

LILString LILNumberLiteral::getValue() const
{
    return this->_value;
}

void LILNumberLiteral::setValue(LILString newValue)
{
    this->_value = newValue;
}
