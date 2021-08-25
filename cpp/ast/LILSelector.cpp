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
 *      This file is the selector in a selector chain
 *
 ********************************************************************/

#include "LILSelector.h"

using namespace LIL;

LILSelector::LILSelector()
: LIL::LILNode(NodeTypeSelector)
{
    this->_selectorType = SelectorTypeNone;
    this->_name = "";
}

LILSelector::LILSelector(const LILSelector &other)
: LILNode(other)
{
    this->_selectorType = other._selectorType;
    this->_name = other._name;
}

std::shared_ptr<LILSelector> LILSelector::clone() const
{
    return std::static_pointer_cast<LILSelector> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILSelector::cloneImpl() const
{
    std::shared_ptr<LILSelector> clone(new LILSelector(*this));
    return clone;
}

LILSelector::~LILSelector()
{
    
}

void LILSelector::receiveNodeData(const LIL::LILString &data)
{
    if (data == "*")
    {
        this->setSelectorType(SelectorTypeUniversalSelector);
    }
    else if (data == "@this")
    {
        this->setSelectorType(SelectorTypeThisSelector);
    }
    else if (data == "@root")
    {
        this->setSelectorType(SelectorTypeRootSelector);
    }
    else if (data == "@parent")
    {
        this->setSelectorType(SelectorTypeParentSelector);
    }
    else if (data == "@self")
    {
        this->setSelectorType(SelectorTypeSelfSelector);
    }
    else if (data == "@super")
    {
        this->setSelectorType(SelectorTypeSuperSelector);
    }
    else if (data == "@mainMenu")
    {
        this->setSelectorType(SelectorTypeMainMenu);
    }
    else
    {
        this->setSelectorType(SelectorTypeNameSelector);
    }
    this->_name = data;
}

bool LILSelector::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    //other checks
    if ( this->_selectorType != otherNode->getSelectorType()) return false;
    //compare the child nodes
    size_t nodesSize = this->_childNodes.size();
    const auto otherChildNodes = otherNode->getChildNodes();
    for (size_t i = 0; i<nodesSize; ++i)
    {
        if ( ! this->_childNodes[i]->equalTo(otherChildNodes[i])) return false;
    }
    return true;
    
}

SelectorType LILSelector::getSelectorType() const
{
    return this->_selectorType;
}

bool LILSelector::isA(SelectorType otherType) const
{
    return this->_selectorType == otherType;
}

void LILSelector::setSelectorType(SelectorType newType)
{
    this->_selectorType = newType;
}

LILString LILSelector::getName() const
{
    return this->_name;
}
void LILSelector::setName(LILString newName)
{
    this->_name = newName;
}
