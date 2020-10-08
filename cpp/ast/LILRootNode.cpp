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
#include "LILInstruction.h"
#include "LILVarDecl.h"

using namespace LIL;

LILRootNode::LILRootNode()
: LILVarNode(NodeTypeRoot)
{
    this->_mainFunction = std::make_shared<LILFunctionDecl>();
    this->_mainFunction->setFunctionDeclType(FunctionDeclTypeFn);
    this->_mainFunction->setName("main");
    
    this->_mainFunctionVarDecl = std::make_shared<LILVarDecl>();
    auto ty = LILFunctionType::make("i64");
    this->_mainFunctionVarDecl->setType(ty);
    this->_mainFunctionVarDecl->setInitVal(this->_mainFunction);
}

LILRootNode::LILRootNode(const LILRootNode & other)
: LILVarNode(other)
{
    this->_localVars = other._localVars;
    this->_mainFunction = other._mainFunction;
    this->_mainFunctionVarDecl = other._mainFunctionVarDecl;
    this->_classes = other._classes;
    this->_dependencies = other._dependencies;
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

std::shared_ptr<LILVarDecl> LILRootNode::getMainFnVarDecl() const
{
    return this->_mainFunctionVarDecl;
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

void LILRootNode::addDependency(std::shared_ptr<LILInstruction> value)
{
    this->_dependencies.push_back(value);
}

std::vector<std::shared_ptr<LILInstruction>> LILRootNode::getDependencies() const
{
    return this->_dependencies;
}
