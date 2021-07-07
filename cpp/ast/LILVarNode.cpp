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
 *      This is a node that participates in local variables
 *
 ********************************************************************/

#include "LILVarNode.h"

using namespace LIL;

LILVarNode::LILVarNode(NodeType type)
: LILNode(type)
{
    
}

LILVarNode::LILVarNode(const LILVarNode & other)
: LILNode(other)
{
    this->_localVars = other._localVars;
}

LILVarNode::~LILVarNode()
{
    
}

bool LILVarNode::isVarNode() const
{
    return true;
}

std::shared_ptr<LILNode> LILVarNode::getLocalVariable(LILString name)
{
    if (this->_localVars.count(name)) {
        return this->_localVars.at(name);
    }
    return nullptr;
}

const std::map<LILString, std::shared_ptr<LILNode>> & LILVarNode::getLocalVariables()
{
    return this->_localVars;
}

void LILVarNode::setLocalVariable(LILString name, std::shared_ptr<LILNode> value)
{
    this->_localVars[name] = value;
}

void LILVarNode::unsetLocalVariable(LILString name)
{
    this->_localVars.erase(name);
}

void LILVarNode::clearLocalVars()
{
    this->_localVars.clear();
}
