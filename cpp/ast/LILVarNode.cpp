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

LILString LILVarNode::stringRep()
{
    return "var node";
}

bool LILVarNode::isVarNode() const
{
    return true;
}

std::shared_ptr<LILNode> LILVarNode::getVar(LILString name)
{
    //first look at its own locals
    std::shared_ptr<LILNode> localVar = this->getVariable(name);
    if (localVar)
        return localVar;

    //not found, look at the parent
    std::shared_ptr<LILVarNode> parentNode = this->getClosestVarNode();
    if (parentNode)
    {
        std::shared_ptr<LILNode> parentVar = parentNode->getVar(name);
        if (parentVar)
            return parentVar;
    }
    return std::shared_ptr<LILNode>();
}

std::shared_ptr<LILNode> LILVarNode::getVariable(LILString name)
{
    return this->_localVars[name];
}

bool LILVarNode::hasLocalVar(LILString name)
{
    //first look at its own locals
    bool hasLocal = this->hasLocalVariable(name);
    if (hasLocal)
        return hasLocal;

    //not found, look at the parent
    std::shared_ptr<LILVarNode> parentNode = this->getClosestVarNode();
    if (parentNode)
    {
        bool hasParent = parentNode->hasLocalVar(name);
        if (hasParent)
            return hasParent;
    }
    return false;
}

bool LILVarNode::hasLocalVariable(LILString name)
{
    if (this->_localVars[name]) {
        return true;
    }
    return false;
}

void LILVarNode::setLocalVariable(LILString name, std::shared_ptr<LILNode> value)
{
    this->_localVars[name] = value;
}

std::shared_ptr<LILVarNode> LILVarNode::getClosestVarNode()
{
    std::shared_ptr<LILVarNode> ret;
    std::shared_ptr<LILNode> parent = this->getParentNode();
    if (!parent) {
        return ret;
    }
    ret = std::dynamic_pointer_cast<LILVarNode>(parent);
    if (ret)
        return ret;

    bool done = false;
    while (!done) {
        done = true;

        parent = parent->getParentNode();
        if (parent)
        {
            done = false;
        }
        else
        {
            return ret;
        }
        ret = std::dynamic_pointer_cast<LILVarNode>(parent);
        if (ret)
            return ret;
    }
    return ret;
}

