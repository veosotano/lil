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

#include "LILRootNode.h"
#include "LILClassDecl.h"
#include "LILFunctionDecl.h"

using namespace LIL;

LILRootNode::LILRootNode()
: LILVarNode(NodeTypeRoot)
{
    this->_mainFunction = std::make_shared<LILFunctionDecl>();
    this->_mainFunction->setFunctionDeclType(FunctionDeclTypeFn);
    auto ty = LILFunctionType::make("i64");
    this->_mainFunction->setType(ty);
    this->_mainFunction->setName("main");
}

LILRootNode::LILRootNode(const LILRootNode & other)
: LILVarNode(other)
{
    this->_localVars = other._localVars;
}

LILRootNode::~LILRootNode()
{
    
}

LILString LILRootNode::stringRep()
{
    return "root node";
}

bool LILRootNode::isRootNode() const
{
    return true;
}

std::shared_ptr<LILFunctionDecl> LILRootNode::getMainFn() const
{
    return this->_mainFunction;
}

const std::vector<std::shared_ptr<LILNode>> & LILRootNode::getNodes() const
{
    return this->getChildNodes();
}

void LILRootNode::addClass(std::shared_ptr<LILClassDecl> value)
{
    this->_classes.push_back(value);
}

std::vector<std::shared_ptr<LILClassDecl>> LILRootNode::getClasses() const
{
    return this->_classes;
}
