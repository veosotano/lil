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
 *      This file represents a value path
 *
 ********************************************************************/

#include "LILValuePath.h"

using namespace LIL;

LILValuePath::LILValuePath()
: LILTypedNode(NodeTypeValuePath)
, _preventEmitCallToIVar(false)
{

}

LILValuePath::LILValuePath(const LILValuePath &other)
: LILTypedNode(other)
, _preventEmitCallToIVar(other._preventEmitCallToIVar)
{

}

std::shared_ptr<LILValuePath> LILValuePath::clone() const
{
    return std::static_pointer_cast<LILValuePath> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILValuePath::cloneImpl() const
{
    std::shared_ptr<LILValuePath> clone(new LILValuePath(*this));
    LILNode::cloneChildNodes(clone);
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILValuePath::~LILValuePath()
{

}

LILString LILValuePath::stringRep()
{
    LILString ret;
    auto nodes = this->getNodes();
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        auto node = *it;
        ret += node->stringRep();
        if (it+1 != nodes.end()) {
            ret += ".";
        }
    }
    return ret;
}

const std::vector<std::shared_ptr<LILNode>> & LILValuePath::getNodes() const
{
    return this->getChildNodes();
}

void LILValuePath::setNodes(const std::vector<std::shared_ptr<LILNode>> & newNodes)
{
    this->clearChildNodes();
    for (const auto & node : newNodes) {
        this->addChild(node);
    }
}

void LILValuePath::addChild(std::shared_ptr<LILNode> child)
{
    this->addNode(child);
}

void LILValuePath::setPreventEmitCallToIVar(bool newValue)
{
    this->_preventEmitCallToIVar = newValue;
}

bool LILValuePath::getPreventEmitCallToIVar() const
{
    return this->_preventEmitCallToIVar;
}
