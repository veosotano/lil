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
 *      This file implements object definitions
 *
 ********************************************************************/

#include "LILObjectDefinition.h"
#include "LILObjectType.h"

using namespace LIL;

LILObjectDefinition::LILObjectDefinition()
: LILTypedNode(NodeTypeObjectDefinition)
{
}

LILObjectDefinition::LILObjectDefinition(const LILObjectDefinition &other)
: LILTypedNode(other)
{
}

std::shared_ptr<LILObjectDefinition> LILObjectDefinition::clone() const
{
    return std::static_pointer_cast<LILObjectDefinition> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILObjectDefinition::cloneImpl() const
{
    std::shared_ptr<LILObjectDefinition> clone(new LILObjectDefinition(*this));
    clone->clearChildNodes();

    for (auto it = this->getChildNodes().begin(); it!=this->getChildNodes().end(); ++it)
    {
        clone->addNode((*it)->clone());
    }
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILObjectDefinition::~LILObjectDefinition()
{
}

void LILObjectDefinition::receiveNodeData(const LIL::LILString &data)
{
    if (data == "@") {
        return;
    }
    auto newType = std::make_shared<LILObjectType>();
    newType->setName(data);
    this->setType(newType);
}

const std::vector<std::shared_ptr<LILNode>> & LILObjectDefinition::getNodes() const
{
    return this->getChildNodes();
}

void LILObjectDefinition::setNodes(const std::vector<std::shared_ptr<LILNode>> && nodes)
{
    this->setChildNodes(std::move(nodes));
}
