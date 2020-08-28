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
 *      This file represents a percentage written in the source code
 *
 ********************************************************************/

#include "LILPercentageLiteral.h"

using namespace LIL;

LILPercentageLiteral::LILPercentageLiteral()
: LIL::LILNode( NodeTypePercentage )
{
    this->_value = 0.;
}

LILPercentageLiteral::LILPercentageLiteral(const LILPercentageLiteral & other)
: LILNode(other)
{
    this->_value = other._value;
}

std::shared_ptr<LILPercentageLiteral> LILPercentageLiteral::clone() const
{
    return std::static_pointer_cast<LILPercentageLiteral> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILPercentageLiteral::cloneImpl() const
{
    return std::shared_ptr<LILPercentageLiteral>(new LILPercentageLiteral(*this));
}

LILPercentageLiteral::~LILPercentageLiteral()
{
    
}

LILString LILPercentageLiteral::stringRep()
{
    return LILString::number(this->_value) + "%";
}

void LILPercentageLiteral::receiveNodeData(const LIL::LILString &data)
{
    this->setValue(data.toDouble());
}

void LILPercentageLiteral::setValue(LILUnitF64 newValue)
{
    this->_value = newValue;
}

LILUnitF64 LILPercentageLiteral::getValue() const
{
    return this->_value;
}
