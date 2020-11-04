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
 *      This file represents function declarations
 *
 ********************************************************************/

#include "LILFunctionDecl.h"
#include "LILVarDecl.h"

using namespace LIL;

LILFunctionDecl::LILFunctionDecl()
: LILVarNode(NodeTypeFunctionDecl)
, _functionDeclType(FunctionDeclTypeFn)
, _name("")
, needsNameMangling(false)
, _hasReturn(false)
, _isConstructor(false)
, _hasOwnType(false)
{
    this->_receivesFunctionBody = false;
    this->_functionDeclType = FunctionDeclTypeNone;
}

LILFunctionDecl::LILFunctionDecl(const LILFunctionDecl &other)
: LILVarNode(other)
{
    this->_body = other._body;
    this->_receivesFunctionBody = other._receivesFunctionBody;
    this->_functionDeclType = other._functionDeclType;
    this->_name = other._name;
    this->needsNameMangling = other.needsNameMangling;
    this->_hasReturn = other._hasReturn;
    this->_isConstructor = other._isConstructor;
    this->_finally = other._finally;
    this->_hasOwnType = other._hasOwnType;
    this->_fnType = other._fnType;
}

bool LILFunctionDecl::isTypedNode() const
{
    return true;
}

void LILFunctionDecl::setType(std::shared_ptr<LILType> value)
{
    if (this->_hasOwnType) {
        if (value->isA(TypeTypeFunction)) {
            this->_fnType = std::static_pointer_cast<LILFunctionType>(value);
        }
    } else {
        auto parent = this->getParentNode();
        if (parent && parent->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(parent);
            if (vd) {
                vd->setType(value);
            }
        }
    }
}

std::shared_ptr<LILType> LILFunctionDecl::getType() const
{
    if (this->_hasOwnType) {
        return this->_fnType;
    } else {
        auto parent = this->getParentNode();
        if (parent && parent->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(parent);
            if (vd) {
                return vd->getType();
            }
        }
    }
    return nullptr;
}

std::shared_ptr<LILFunctionType> LILFunctionDecl::getFnType() const
{
    auto ty = this->getType();
    if (ty->isA(TypeTypeFunction)) {
        return std::static_pointer_cast<LILFunctionType>(ty);
    } else {
        return nullptr;
    }
}

std::shared_ptr<LILFunctionDecl> LILFunctionDecl::clone() const
{
    return std::static_pointer_cast<LILFunctionDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFunctionDecl::cloneImpl() const
{
    std::shared_ptr<LILFunctionDecl> clone(new LILFunctionDecl(*this));

    clone->_body.clear();
    for (auto it = this->_body.begin(); it != this->_body.end(); ++it)
    {
        clone->addEvaluable((*it)->clone());
    }
    if (this->_finally) {
        clone->setFinally(this->_finally->clone());
    }
    if (this->_fnType) {
        clone->_fnType = this->_fnType->clone();
    }
    return clone;
}

LILFunctionDecl::~LILFunctionDecl()
{
    
}

LILString LILFunctionDecl::stringRep()
{
    switch (this->getFunctionDeclType())
    {
        case FunctionDeclTypeNone: return "";
        case FunctionDeclTypeFn:
        {
            return "fn " + this->getName();
        }
        case FunctionDeclTypeInsert: return "insert";
        case FunctionDeclTypeOverride: return "override";
        case FunctionDeclTypeMacro: return "macro";
    }
}

void LILFunctionDecl::receiveNodeData(const LILString &data)
{
    if (data == "override")
    {
        this->setFunctionDeclType(FunctionDeclTypeOverride);
    }
    else if (data == "insert")
    {
        this->setFunctionDeclType(FunctionDeclTypeInsert);
    }
    else if (data == "macro")
    {
        this->setFunctionDeclType(FunctionDeclTypeMacro);
    }
}

void LILFunctionDecl::addEvaluable(std::shared_ptr<LILNode> evl)
{
    this->addNode(evl);
    this->_body.push_back(evl);
    
    if (evl->isA(FlowControlCallTypeReturn)) {
        this->_hasReturn = true;
    }
}

void LILFunctionDecl::prependEvaluable(std::shared_ptr<LILNode> evl)
{
    this->clearChildNodes();

    auto bodyBackup = this->_body;
    this->_body.clear();
    this->addNode(evl);
    this->_body.push_back(evl);
    for (auto existing : bodyBackup){
        this->addNode(existing);
        this->_body.push_back(existing);
    }
    if (evl->isA(FlowControlCallTypeReturn)) {
        this->_hasReturn = true;
    }
}

const std::vector<std::shared_ptr<LILNode>> & LILFunctionDecl::getBody() const
{
    return this->_body;
}

void LILFunctionDecl::setBody(std::vector<std::shared_ptr<LILNode>> newBody)
{
    for (auto node : newBody) {
        node->setParentNode(nullptr);
    }
    this->_body.clear();
    this->_body = newBody;
    for (auto node : this->_body) {
        this->addNode(node);
    }
}

void LILFunctionDecl::setReceivesFunctionBody(bool newValue)
{
    this->_receivesFunctionBody = newValue;
}

bool LILFunctionDecl::getReceivesFunctionBody() const
{
    return this->_receivesFunctionBody;
}

FunctionDeclType LILFunctionDecl::getFunctionDeclType() const
{
    return this->_functionDeclType;
}

void LILFunctionDecl::setFunctionDeclType(FunctionDeclType newType)
{
    this->_functionDeclType = newType;
}

void LILFunctionDecl::setReturnType(std::shared_ptr<LILType> type)
{
    auto ty = this->getType();
    if (!ty) {
        ty = std::make_shared<LILFunctionType>();
        this->setType(ty);
    }
    if (ty->isA(TypeTypeFunction)) {
        std::static_pointer_cast<LILFunctionType>(ty)->setReturnType(type);
    }
}

std::shared_ptr<LILType> LILFunctionDecl::getReturnType() const
{
    auto ty = this->getType();
    if (ty && ty->isA(TypeTypeFunction)) {
        return std::static_pointer_cast<LILFunctionType>(ty)->getReturnType();
    }
    return nullptr;
}

void LILFunctionDecl::setName(LILString value)
{
    this->_name = value;
}

LILString LILFunctionDecl::getName() const
{
    return this->_name;
}

bool LILFunctionDecl::hasReturn() const
{
    return this->_hasReturn;
}

bool LILFunctionDecl::getIsConstructor() const
{
    return this->_isConstructor;
}

void LILFunctionDecl::setIsConstructor(bool value)
{
    this->_isConstructor = value;
}

std::shared_ptr<LILNode> LILFunctionDecl::getFinally() const
{
    return this->_finally;
}

void LILFunctionDecl::setFinally(std::shared_ptr<LILNode> value)
{
    this->addNode(value);
    this->_finally = value;
}

void LILFunctionDecl::setHasOwnType(bool value)
{
    this->_hasOwnType = value;
}

bool LILFunctionDecl::getHasOwnType() const
{
    return this->_hasOwnType;
}
