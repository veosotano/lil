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
 *      This file represents a rule, which has selector chains and properties
 *
 ********************************************************************/

#include "LILRule.h"

using namespace LIL;

LILRule::LILRule()
: LIL::LILNode(NodeTypeRule)
{
    
}

LILRule::LILRule(const LILRule &other)
: LILNode(other)
{
    this->_selectorChains = other._selectorChains;
    this->_values = other._values;
    this->_childRules = other._childRules;
}

std::shared_ptr<LILRule> LILRule::clone() const
{
    return std::static_pointer_cast<LILRule> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILRule::cloneImpl() const
{
    std::shared_ptr<LILRule> clone(new LILRule(*this));
    for (auto it = this->_selectorChains.begin(); it!=this->_selectorChains.end(); ++it)
    {
        clone->addSelectorChain((*it)->clone());
    }
    for (auto it = this->_values.begin(); it!=this->_values.end(); ++it)
    {
        clone->addValue((*it)->clone());
    }
    for (auto it = this->_childRules.begin(); it!=this->_childRules.end(); ++it)
    {
        clone->addChildRule((*it)->clone());
    }
    return clone;
}

LILRule::~LILRule()
{
    
}

void LILRule::addSelectorChain(std::shared_ptr<LILNode> newSc)
{
    this->addNode(newSc);
    this->_selectorChains.push_back(newSc);
}

void LILRule::addValue(std::shared_ptr<LILNode> newVal)
{
    this->addNode(newVal);
    this->_values.push_back(newVal);
}

void LILRule::addChildRule(std::shared_ptr<LILRule> rule)
{
    this->addNode(rule);
    this->_childRules.push_back(rule);
}

const std::vector<std::shared_ptr<LILNode>> & LILRule::getNodes() const
{
    return this->getChildNodes();
}
