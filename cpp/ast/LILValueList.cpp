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
 *      This file holds multiple values separated by commas
 *
 ********************************************************************/

#include "LILValueList.h"
#include "LILVarNode.h"

using namespace LIL;

LILValueList::LILValueList()
: LILNode(NodeTypeValueList)
{
    
}

LILValueList::LILValueList(const LILValueList &other)
: LILNode(other)
{
    
}

std::shared_ptr<LILValueList> LILValueList::clone() const
{
    return std::static_pointer_cast<LILValueList> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILValueList::cloneImpl() const
{
    std::shared_ptr<LILValueList> clone(new LILValueList(*this));
    for (auto node : this->getChildNodes()) {
        clone->addValue(node->clone());
    }
    return clone;
}

LILValueList::~LILValueList()
{
    
}

void LILValueList::receiveNodeData(const LIL::LILString &data)
{

}

LILString LILValueList::stringRep()
{
    LILString tempstr;
    auto children = this->getChildNodes();
    for (size_t i = 0, j = children.size(); i<j; i+=1) {
        tempstr += children[i]->stringRep();
        if (i<j-1) {
            tempstr += ", ";
        }
    }
    return tempstr;
}

void LILValueList::addValue(std::shared_ptr<LILNode> arg)
{
    this->addNode(arg);
}

void LILValueList::setValues(std::vector<std::shared_ptr<LILNode>> vals)
{
    this->clearChildNodes();
    for (auto val : vals) {
        this->addValue(val);
    }
}

void LILValueList::clearValues()
{
    this->clearChildNodes();
}

std::vector<std::shared_ptr<LILNode>> LILValueList::getValues() const
{
    return this->getChildNodes();
}

std::shared_ptr<LILType> LILValueList::getType() const
{
    auto parent = this->getParentNode();
    if (parent) {
        return parent->getType();
    }
    return nullptr;
}

