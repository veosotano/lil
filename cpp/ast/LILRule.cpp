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
: LILTypedNode(NodeTypeRule)
{
    
}

LILRule::LILRule(const LILRule &other)
: LILTypedNode(other)
{
    this->_selectorChain = other._selectorChain;
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

    if (this->_selectorChain) {
        clone->setSelectorChain(this->_selectorChain->clone());
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
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILRule::~LILRule()
{
    
}

const std::shared_ptr<LILNode> & LILRule::getSelectorChain() const
{
    return this->_selectorChain;
}

void LILRule::setSelectorChain(std::shared_ptr<LILNode> value)
{
    this->addNode(value);
    this->_selectorChain = value;
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
    if (this->_selectorChain) {
        const auto & sChainNodes = this->_selectorChain->getChildNodes();
        if (sChainNodes.size() > 0) {
            const auto & firstSimpleSel = sChainNodes.front();
            const auto & firstSimpleSelNodes = firstSimpleSel->getChildNodes();
            if (firstSimpleSelNodes.size() > 0) {
                const auto & firstSel = firstSimpleSelNodes.front();
                return firstSel;
            }
        }
    }
    return nullptr;
}
