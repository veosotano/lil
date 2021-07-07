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
 *      This file represents a simple selector, which groups individual
 *      selectors as a basic group
 *
 ********************************************************************/

#include "LILSimpleSelector.h"

using namespace LIL;

LILSimpleSelector::LILSimpleSelector()
: LIL::LILNode(NodeTypeSimpleSelector)
{
    
}

LILSimpleSelector::LILSimpleSelector(const LILSimpleSelector &other)
: LILNode(other)
{

}

std::shared_ptr<LILSimpleSelector> LILSimpleSelector::clone() const
{
    return std::static_pointer_cast<LILSimpleSelector> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILSimpleSelector::cloneImpl() const
{
    std::shared_ptr<LILSimpleSelector> clone(new LILSimpleSelector(*this));
    return clone;
}

LILSimpleSelector::~LILSimpleSelector()
{
    
}

const std::vector<std::shared_ptr<LILNode>> & LILSimpleSelector::getNodes() const
{
    return this->getChildNodes();
}
