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
{
    
}

LILVarDecl::LILVarDecl(const LILVarDecl & orig)
: LILTypedNode(orig)
{
    this->_name = orig._name;
    this->_isExtern = orig._isExtern;
    this->_isIVar = orig._isIVar;
    this->_isVVar = orig._isVVar;
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

LILString LILVarDecl::stringRep()
{
    LILString kw;
    if (this->getIsIVar()) {
        kw = "ivar";
    } else if (this->getIsVVar()){
        kw = "vvar";
    } else {
        kw = "var";
    }

    if (this->getInitVal())
    {
        return kw + this->getName() + " = " + this->getInitVal()->stringRep();
    }
    return kw + " " + this->getName();
}

void LILVarDecl::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILVarDecl::getName() const
{
    return this->_name;
}

void LILVarDecl::setInitVal(std::shared_ptr<LILNode> value)
{
    this->addNode(value);
}

std::shared_ptr<LILNode> LILVarDecl::getInitVal() const
{
    if (this->getChildNodes().size() > 0) {
        return this->getChildNodes().front();
    } else {
        return nullptr;
    }
}

const std::vector<std::shared_ptr<LILNode>> & LILVarDecl::getInitVals() const
{
    return this->getChildNodes();
}

void LILVarDecl::setIsExtern(bool value)
{
    this->_isExtern = value;
}

bool LILVarDecl::getIsExtern() const
{
    return this->_isExtern;
}

void LILVarDecl::setIsIVar(bool value)
{
    this->_isIVar = value;
}

bool LILVarDecl::getIsIVar() const
{
    return this->_isIVar;
}

void LILVarDecl::setIsVVar(bool value)
{
    this->_isVVar = value;
}

bool LILVarDecl::getIsVVar() const
{
    return this->_isVVar;
}
