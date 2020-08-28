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
 *      This file represents a selector chain
 *
 ********************************************************************/

#include "LILSelectorChain.h"

using namespace LIL;

LILSelectorChain::LILSelectorChain()
: LIL::LILNode(NodeTypeSelectorChain)
{
    
}

LILSelectorChain::LILSelectorChain(const LILSelectorChain &other)
: LILNode(other)
{
    
}

std::shared_ptr<LILSelectorChain> LILSelectorChain::clone() const
{
    return std::static_pointer_cast<LILSelectorChain> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILSelectorChain::cloneImpl() const
{
    std::shared_ptr<LILSelectorChain> clone(new LILSelectorChain(*this));
    return clone;
}

LILSelectorChain::~LILSelectorChain()
{
    
}

const std::vector<std::shared_ptr<LILNode>> & LILSelectorChain::getNodes() const
{
    return this->getChildNodes();
}
