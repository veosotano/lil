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
    this->_instruction = other._instruction;
}

std::shared_ptr<LILRule> LILRule::clone() const
{
    return std::static_pointer_cast<LILRule> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILRule::cloneImpl() const
{
    std::shared_ptr<LILRule> clone(new LILRule(*this));
    clone->clearChildNodes();

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
    if (this->_instruction) {
        clone->setInstruction(this->_instruction->clone());
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

const std::vector<std::shared_ptr<LILNode>> & LILRule::getSelectorChains() const
{
    return this->_selectorChains;
}

void LILRule::setSelectorChains(std::vector<std::shared_ptr<LILNode>> && nodes)
{
    this->_selectorChains = std::move(nodes);
}

void LILRule::addValue(std::shared_ptr<LILNode> newVal)
{
    this->addNode(newVal);
    this->_values.push_back(newVal);
}

const std::vector<std::shared_ptr<LILNode>> & LILRule::getValues() const
{
    return this->_values;
}

void LILRule::setValues(std::vector<std::shared_ptr<LILNode>> && nodes)
{
    this->_values = std::move(nodes);
}

void LILRule::addChildRule(std::shared_ptr<LILRule> rule)
{
    this->addNode(rule);
    this->_childRules.push_back(rule);
}
const std::vector<std::shared_ptr<LILRule>> & LILRule::getChildRules() const
{
    return this->_childRules;
}

void LILRule::setChildRules(std::vector<std::shared_ptr<LILRule>> && rules)
{
    this->_childRules = std::move(rules);
}

void LILRule::setInstruction(std::shared_ptr<LILNode> instruction)
{
    this->_instruction = instruction;
}

const std::shared_ptr<LILNode> LILRule::getInstruction() const
{
    return this->_instruction;
}

const std::vector<std::shared_ptr<LILNode>> & LILRule::getNodes() const
{
    return this->getChildNodes();
}

std::shared_ptr<LILNode> LILRule::getFirstSelector() const
{
    if (this->_selectorChains.size() > 0) {
        const auto & firstChain = this->_selectorChains.front();
        const auto & firstChainNodes = firstChain->getChildNodes();
        if (firstChainNodes.size() > 0) {
            const auto & firstSimpleSel = firstChainNodes.front();
            const auto & firstSimpleSelNodes = firstSimpleSel->getChildNodes();
            if (firstSimpleSelNodes.size() > 0) {
                const auto & firstSel = firstSimpleSelNodes.front();
                return firstSel;
            }
        }
    }
    return nullptr;
}
