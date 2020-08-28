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
 *      This file is a filter in a selector chain
 *
 ********************************************************************/

#include "LILFilter.h"

using namespace LIL;

LILFilter::LILFilter()
: LIL::LILNode(NodeTypeFilter)
{
    
}

LILFilter::LILFilter(const LILFilter &other)
: LILNode(other)
{
    this->_name = other._name;
}

std::shared_ptr<LILFilter> LILFilter::clone() const
{
    return std::static_pointer_cast<LILFilter> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFilter::cloneImpl() const
{
    std::shared_ptr<LILFilter> clone(new LILFilter(*this));
    return clone;
}

LILFilter::~LILFilter()
{
    
}

void LILFilter::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

LILString LILFilter::stringRep()
{
    return this->_name;
}

void LILFilter::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILFilter::getName() const
{
    return this->_name;
}
