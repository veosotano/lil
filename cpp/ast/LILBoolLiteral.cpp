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
 *      This file represents a bool written in the source code
 *
 ********************************************************************/

#include "LILBoolLiteral.h"

using namespace LIL;

LILBoolLiteral::LILBoolLiteral()
: LILTypedNode(NodeTypeBoolLiteral)
{

}

LILBoolLiteral::LILBoolLiteral(const LILBoolLiteral & other)
: LILTypedNode(other)
{
    this->_value = other._value;
    this->_originalRep = other._originalRep;
}

std::shared_ptr<LILBoolLiteral> LILBoolLiteral::clone() const
{
    auto clone = std::static_pointer_cast<LILBoolLiteral> (this->cloneImpl());
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

std::shared_ptr<LILClonable> LILBoolLiteral::cloneImpl() const
{
    return std::shared_ptr<LILBoolLiteral>(new LILBoolLiteral(*this));
}

LILBoolLiteral::~LILBoolLiteral()
{
    
}

void LILBoolLiteral::receiveNodeData(const LIL::LILString &data)
{
    if (data == "true") {
        this->setValue(true);
    } else if (data == "false"){
        this->setValue(false);
    }
}

void LILBoolLiteral::setValue(bool newValue)
{
    this->_value = newValue;
}

void LILBoolLiteral::setOriginalStringRep(LILString string)
{
    this->_originalRep = string;
}

LILString LILBoolLiteral::originalStringRep()
{
    return this->_originalRep;
}

bool LILBoolLiteral::getValue()
{
    return this->_value;
}

bool LILBoolLiteral::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILTypedNode::equalTo(otherNode)) return false;
    std::shared_ptr<LILBoolLiteral> castedNode = std::static_pointer_cast<LILBoolLiteral>(otherNode);
    if ( this->_value != castedNode->_value ) return false;
    return true;
}
