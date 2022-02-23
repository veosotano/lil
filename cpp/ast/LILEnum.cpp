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
 *      This file represents an enum
 *
 ********************************************************************/

#include "LILEnum.h"
#include "LILAssignment.h"
#include "LILPropertyName.h"

using namespace LIL;

LILEnum::LILEnum()
: LILTypedNode(NodeTypeEnum)
{
    
}

LILEnum::LILEnum(const LILEnum &other)
: LILTypedNode(other)
{
    
}

std::shared_ptr<LILEnum> LILEnum::clone() const
{
    return std::static_pointer_cast<LILEnum> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILEnum::cloneImpl() const
{
    std::shared_ptr<LILEnum> clone(new LILEnum(*this));
    LILNode::cloneChildNodes(clone);
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILEnum::~LILEnum()
{
    
}

const std::vector<std::shared_ptr<LILNode>> & LILEnum::getValues() const
{
    return this->getChildNodes();
}

void LILEnum::setValues(const std::vector<std::shared_ptr<LILNode>> & newNodes)
{
    this->clearChildNodes();
    for (const auto & node : newNodes) {
        this->addValue(node);
    }
}

void LILEnum::addValue(std::shared_ptr<LILNode> child)
{
    this->addNode(child);
}

std::shared_ptr<LILNode> LILEnum::getValueNamed(const LILString & name) const
{
    for (auto node : this->getChildNodes()) {
        if (!node->isA(NodeTypeAssignment)) {
            continue;
        }
        auto as = std::static_pointer_cast<LILAssignment>(node);
        auto subjNode = as->getSubject();
        if (subjNode->getNodeType() != NodeTypePropertyName) {
            continue;
        }
        if (std::static_pointer_cast<LILPropertyName>(subjNode)->getName() == name) {
            return as->getValue();
        }
    }
    return nullptr;
}

void LILEnum::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILEnum::getName() const
{
    return this->_name;
}

void LILEnum::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}
