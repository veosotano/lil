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
 *      This file represents var declaration
 *
 ********************************************************************/

#include "LILVarDecl.h"
#include "LILExpression.h"
#include "LILObjectDefinition.h"
#include "LILType.h"

using namespace LIL;

LILVarDecl::LILVarDecl()
: LILTypedNode(NodeTypeVarDecl)
, _isExtern(false)
, _isIVar(false)
, _isVVar(false)
, _isConst(false)
, _receivesReturnType(false)
{
    
}

LILVarDecl::LILVarDecl(const LILVarDecl & orig)
: LILTypedNode(orig)
, _returnType(orig._returnType)
, _name(orig._name)
, _isExtern(orig._isExtern)
, _isIVar(orig._isIVar)
, _isVVar(orig._isVVar)
, _isConst(orig._isConst)
, _receivesReturnType(orig._receivesReturnType)
{

}

std::shared_ptr<LILVarDecl> LILVarDecl::clone() const
{
    return std::static_pointer_cast<LILVarDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILVarDecl::cloneImpl() const
{
    std::shared_ptr<LILVarDecl> clone(new LILVarDecl(*this));

    LILNode::cloneChildNodes(clone);

    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILVarDecl::~LILVarDecl()
{

}

void LILVarDecl::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

bool LILVarDecl::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    auto castedNode = std::static_pointer_cast<LILVarDecl>(otherNode);
    if ( this->_name != castedNode->_name) return false;
    if ( this->_isExtern != castedNode->_isExtern) return false;
    if ( this->_isIVar != castedNode->_isIVar) return false;
    if ( this->_isVVar != castedNode->_isVVar) return false;
    if ( this->_isConst != castedNode->_isConst) return false;
    if ( this->_receivesReturnType != castedNode->_receivesReturnType) return false;
    if ( this->_returnType && !castedNode->_returnType) return false;
    if ( this->_returnType && (!this->_returnType->equalTo(castedNode->_returnType))) return false;
    return true;
}

const LILString LILVarDecl::getName() const
{
    return this->_name;
}

void LILVarDecl::setName(LILString newName)
{
    this->_name = newName;
}

std::shared_ptr<LILNode> LILVarDecl::getInitVal() const
{
    if (this->getChildNodes().size() > 0) {
        return this->getChildNodes().front();
    } else {
        return nullptr;
    }
}

void LILVarDecl::setInitVal(std::shared_ptr<LILNode> value)
{
    this->clearChildNodes();
    this->addNode(value);
}

void LILVarDecl::setInitVals(const std::vector<std::shared_ptr<LILNode>> & values)
{
    this->clearChildNodes();
    for (auto value : values) {
        this->addNode(value);
    }
}

bool LILVarDecl::getIsExtern() const
{
    return this->_isExtern;
}

void LILVarDecl::setIsExtern(bool value)
{
    this->_isExtern = value;
}

bool LILVarDecl::getIsIVar() const
{
    return this->_isIVar;
}

void LILVarDecl::setIsIVar(bool value)
{
    this->_isIVar = value;
}

bool LILVarDecl::getIsVVar() const
{
    return this->_isVVar;
}

void LILVarDecl::setIsVVar(bool value)
{
    this->_isVVar = value;
}

bool LILVarDecl::getIsConst() const
{
    return this->_isConst;
}

void LILVarDecl::setIsConst(bool value)
{
    this->_isConst = value;
}
bool LILVarDecl::getReceivesReturnType() const
{
    return this->_receivesReturnType;
}

void LILVarDecl::setReceivesReturnType(bool value)
{
    this->_receivesReturnType = value;
}

std::shared_ptr<LILType> LILVarDecl::getReturnType() const
{
    return this->_returnType;
}

void LILVarDecl::setReturnType(std::shared_ptr<LILType> value)
{
    this->_returnType = value;
}
