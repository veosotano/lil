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
, _name("")
, needsNameMangling(false)
, _hasReturn(false)
, _isConstructor(false)
{
    this->_receivesFunctionBody = false;
    this->_functionDeclType = FunctionDeclTypeNone;
}

LILFunctionDecl::LILFunctionDecl(const LILFunctionDecl &other)
: LILVarNode(other)
{
    this->_arguments = other._arguments;
    this->_body = other._body;
    this->_receivesFunctionBody = other._receivesFunctionBody;
    this->_functionDeclType = other._functionDeclType;
    this->_type = other._type;
    this->_name = other._name;
    this->needsNameMangling = other.needsNameMangling;
    this->_hasReturn = other._hasReturn;
    this->_isConstructor = other._isConstructor;
    this->_finally = other._finally;
}

bool LILFunctionDecl::isTypedNode() const
{
    return true;
}

void LILFunctionDecl::setType(std::shared_ptr<LILType> value)
{
    auto ty = std::dynamic_pointer_cast<LILFunctionType>(value);
    if (ty) {
        this->_type = ty;
    }
}

std::shared_ptr<LILType> LILFunctionDecl::getType() const
{
    return this->_type;
}

std::shared_ptr<LILFunctionType> LILFunctionDecl::getFnType() const
{
    if (this->_type->isA(TypeTypeFunction)) {
        return std::static_pointer_cast<LILFunctionType>(this->_type);
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
    clone->_arguments.clear();
    for (auto it = this->_arguments.begin(); it != this->_arguments.end(); ++it)
    {
        clone->addArgument((*it)->clone());
    }
    clone->_body.clear();
    for (auto it = this->_body.begin(); it != this->_body.end(); ++it)
    {
        clone->addEvaluable((*it)->clone());
    }
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    if (this->_finally) {
        clone->setFinally(this->_finally->clone());
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
        case FunctionDeclTypeFn: return "fn";
        case FunctionDeclTypeInsert: return "insert";
        case FunctionDeclTypeOverride: return "override";
        case FunctionDeclTypeMacro: return "macro";
    }
}

void LILFunctionDecl::receiveNodeData(const LILString &data)
{
    if (data == "fn")
    {
        this->setFunctionDeclType(FunctionDeclTypeFn);
    }
    else if (data == "override")
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

void LILFunctionDecl::addArgument(std::shared_ptr<LILNode> arg)
{
    this->addNode(arg);
    this->_arguments.push_back(arg);

    if (!arg->isA(NodeTypeVarDecl))
        return;
    std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(arg);
    this->setLocalVariable(vd->getName(), vd);
}

const std::vector<std::shared_ptr<LILNode>> & LILFunctionDecl::getArguments() const
{
    return this->_arguments;
}

void LILFunctionDecl::addEvaluable(std::shared_ptr<LILNode> evl)
{
    this->addNode(evl);
    this->_body.push_back(evl);
    
    if (evl->isA(FunctionCallTypeReturn)) {
        this->_hasReturn = true;
    }
}

void LILFunctionDecl::prependEvaluable(std::shared_ptr<LILNode> evl)
{
    this->clearChildNodes();
    
    for (auto arg : this->_arguments) {
        this->addNode(arg);
    }
    auto bodyBackup = this->_body;
    this->_body.clear();
    this->addNode(evl);
    this->_body.push_back(evl);
    for (auto existing : bodyBackup){
        this->addNode(existing);
        this->_body.push_back(existing);
    }
    if (evl->isA(FunctionCallTypeReturn)) {
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
    if (!this->_type) {
        this->_type = std::make_shared<LILFunctionType>();
    }
    this->_type->setReturnType(type);
}

std::shared_ptr<LILType> LILFunctionDecl::getReturnType() const
{
    if (this->_type) {
        return this->_type->getReturnType();
    } else {
        return nullptr;
    }
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
