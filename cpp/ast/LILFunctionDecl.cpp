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

#include "LILDocumentation.h"
#include "LILFunctionDecl.h"
#include "LILVarDecl.h"

using namespace LIL;

LILFunctionDecl::LILFunctionDecl()
: LILVarNode(NodeTypeFunctionDecl)
, _functionDeclType(FunctionDeclTypeFn)
, _name("")
, _hasReturn(false)
, _isConstructor(false)
, _isExtern(false)
, _hasMultipleImpls(false)
{
    this->_receivesFunctionBody = false;
    this->_functionDeclType = FunctionDeclTypeNone;
}

LILFunctionDecl::LILFunctionDecl(const LILFunctionDecl &other)
: LILVarNode(other)
, _body(other._body)
, _impls(other._impls)
, _docs(other._docs)
, _fnlly(other._fnlly)
, _fnType(other._fnType)
, _receivesFunctionBody(other._receivesFunctionBody)
, _functionDeclType(other._functionDeclType)
, _name(other._name)
, _hasReturn(other._hasReturn)
, _isConstructor(other._isConstructor)
, _isExtern(other._isExtern)
, _hasMultipleImpls(other._hasMultipleImpls)
{
}

std::shared_ptr<LILType> LILFunctionDecl::getType() const
{
    return this->_fnType;
}

void LILFunctionDecl::setType(std::shared_ptr<LILType> value)
{
    if (value->isA(TypeTypeFunction)) {
        this->_fnType = std::static_pointer_cast<LILFunctionType>(value);
        this->_fnType->setParentNode(this->shared_from_this());
    }
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
    clone->clearChildNodes();

    clone->clearLocalVars();

    clone->_body.clear();
    for (auto it = this->_body.begin(); it != this->_body.end(); ++it)
    {
        clone->addEvaluable((*it)->clone());
    }
    if (this->_fnlly) {
        clone->setFinally(this->_fnlly->clone());
    }
    if (this->_fnType) {
        clone->_fnType = this->_fnType->clone();
        for (auto arg : clone->_fnType->getArguments()) {
            if (arg->isA(NodeTypeVarDecl)) {
                auto vd = std::static_pointer_cast<LILVarDecl>(arg);
                clone->setLocalVariable(vd->getName(), vd);
            }
        }
    }
    clone->_impls.clear();
    for (auto it = this->_impls.begin(); it != this->_impls.end(); ++it)
    {
        clone->addImpl((*it)->clone());
    }
    clone->_docs.clear();
    for (auto doc : this->_docs) {
        clone->addDoc(doc->clone());
    }
    return clone;
}

LILFunctionDecl::~LILFunctionDecl()
{
    
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
    
    if (evl->isA(NodeTypeVarDecl)) {
        auto vd = std::static_pointer_cast<LILVarDecl>(evl);
        this->setLocalVariable(vd->getName(), vd);
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
    if (evl->isA(NodeTypeVarDecl)) {
        auto vd = std::static_pointer_cast<LILVarDecl>(evl);
        this->setLocalVariable(vd->getName(), vd);
    }
}

const std::vector<std::shared_ptr<LILNode>> & LILFunctionDecl::getBody() const
{
    return this->_body;
}

void LILFunctionDecl::setBody(const std::vector<std::shared_ptr<LILNode>> & newBody)
{
    this->clearBody();
    for (const auto & node : newBody) {
        this->addEvaluable(node);
    }
}

void LILFunctionDecl::clearBody()
{
    for (auto it = this->_body.rbegin(); it != this->_body.rend(); ++it){
        auto evl = *it;
        if (evl->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(evl);
            this->unsetLocalVariable(vd->getName());
        }
        this->removeNode(evl);
    }
    this->_body.clear();
}

bool LILFunctionDecl::getReceivesFunctionBody() const
{
    return this->_receivesFunctionBody;
}

void LILFunctionDecl::setReceivesFunctionBody(bool newValue)
{
    this->_receivesFunctionBody = newValue;
}

FunctionDeclType LILFunctionDecl::getFunctionDeclType() const
{
    return this->_functionDeclType;
}

void LILFunctionDecl::setFunctionDeclType(FunctionDeclType newType)
{
    this->_functionDeclType = newType;
}

std::shared_ptr<LILType> LILFunctionDecl::getReturnType() const
{
    auto ty = this->getType();
    if (ty && ty->isA(TypeTypeFunction)) {
        return std::static_pointer_cast<LILFunctionType>(ty)->getReturnType();
    }
    return nullptr;
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

LILString LILFunctionDecl::getName() const
{
    return this->_name;
}

void LILFunctionDecl::setName(LILString value)
{
    this->_name = value;
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
    return this->_fnlly;
}

void LILFunctionDecl::setFinally(std::shared_ptr<LILNode> value)
{
    if (this->_fnlly) {
        this->removeNode(this->_fnlly);
    }
    this->addNode(value);
    this->_fnlly = value;
}

bool LILFunctionDecl::getIsExtern() const
{
    return this->_isExtern;
}

void LILFunctionDecl::setIsExtern(bool value)
{
    this->_isExtern = value;
}

bool LILFunctionDecl::getHasMultipleImpls() const
{
    return this->_hasMultipleImpls;
}

void LILFunctionDecl::setHasMultipleImpls(bool value)
{
    this->_hasMultipleImpls = value;
}

const std::vector<std::shared_ptr<LILFunctionDecl>> & LILFunctionDecl::getImpls() const
{
    return this->_impls;
}

void LILFunctionDecl::addImpl(std::shared_ptr<LILFunctionDecl> fd)
{
    this->addNode(fd);
    this->_impls.push_back(fd);
}

void LILFunctionDecl::setImpls(const std::vector<std::shared_ptr<LILFunctionDecl>> & impls)
{
    this->clearImpls();
    for (const auto & impl : impls) {
        this->addImpl(impl);
    }
}

void LILFunctionDecl::clearImpls()
{
    for (auto it = this->_impls.rbegin(); it != this->_impls.rend(); ++it){
        auto impl = *it;
        this->removeNode(impl);
    }
    this->_impls.clear();
}

void LILFunctionDecl::addDoc(std::shared_ptr<LILDocumentation> value)
{
    this->addNode(value);
    this->_docs.push_back(value);
}

const std::vector<std::shared_ptr<LILDocumentation>> & LILFunctionDecl::getDocs() const
{
    return this->_docs;
}
