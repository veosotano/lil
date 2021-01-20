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
 *      This file implements object definitions
 *
 ********************************************************************/

#include "LILClassDecl.h"
#include "LILAliasDecl.h"
#include "LILVarDecl.h"

using namespace LIL;

LILClassDecl::LILClassDecl()
: LILTypedNode(NodeTypeClassDecl)
, _isExtern(false)
, _receivesInherits(false)
, _receivesBody(false)
{
}

LILClassDecl::LILClassDecl(const LILClassDecl &other)
: LILTypedNode(other)
, _isExtern(other._isExtern)
, _inheritType(other._inheritType)
, _receivesInherits(other._receivesInherits)
, _receivesBody(other._receivesBody)
, _fields(other._fields)
, _methods(other._methods)
, _aliases(other._aliases)
{
}

std::shared_ptr<LILClassDecl> LILClassDecl::clone() const
{
    return std::static_pointer_cast<LILClassDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILClassDecl::cloneImpl() const
{
    std::shared_ptr<LILClassDecl> clone(new LILClassDecl(*this));
    clone->clearChildNodes();

    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    if (this->_inheritType) {
        clone->setInheritType(this->_inheritType->clone());
    }
    
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    clone->_fields.clear();
    for (auto field : this->_fields) {
        clone->addField(field->clone());
    }
    clone->_methods.clear();
    for (auto method : this->_methods) {
        clone->addMethod(method->clone());
    }
    clone->_aliases.clear();
    for (auto alias : this->_aliases) {
        clone->addAlias(alias->clone());
    }
    return clone;
}

LILClassDecl::~LILClassDecl()
{
    
}

std::shared_ptr<LILNode> LILClassDecl::getInheritType() const
{
    return this->_inheritType;
}

void LILClassDecl::setInheritType(std::shared_ptr<LILNode> newType)
{
    this->_inheritType = newType;
}

bool LILClassDecl::getReceivesInherits() const
{
    return this->_receivesInherits;
}

void LILClassDecl::setReceivesInherits(bool value)
{
    this->_receivesInherits = value;
}

bool LILClassDecl::getReceivesBody() const
{
    return this->_receivesBody;
}

void LILClassDecl::setReceivesBody(bool value)
{
    this->_receivesBody = value;
}

void LILClassDecl::addField(std::shared_ptr<LILNode> value)
{
    this->addNode(value);
    this->_fields.push_back(value);
}

const std::vector<std::shared_ptr<LILNode>> & LILClassDecl::getFields() const
{
    return this->_fields;
}

void LILClassDecl::addMethod(std::shared_ptr<LILNode> value)
{
    this->addNode(value);
    this->_methods.push_back(value);
}

const std::vector<std::shared_ptr<LILNode>> & LILClassDecl::getMethods() const
{
    return this->_methods;
}

LILString LILClassDecl::getName() const
{
    return this->getType()->getName();
}

std::shared_ptr<LILNode> LILClassDecl::getFieldNamed(const LILString & name) const
{
    for (auto field : this->_fields) {
        if (!field->isA(NodeTypeVarDecl)) {
            continue;
        }
        auto vd = std::static_pointer_cast<LILVarDecl>(field);
        if (vd->getName() == name) {
            return vd;
        }
    }
    return nullptr;
}

std::shared_ptr<LILNode> LILClassDecl::getMethodNamed(const LILString & name) const
{
    for (auto method : this->_methods) {
        if (!method->isA(NodeTypeVarDecl)) {
            continue;
        }
        auto vd = std::static_pointer_cast<LILVarDecl>(method);
        if (vd->getName() == name) {
            return vd;
        }
    }
    return nullptr;
}

bool LILClassDecl::getIsExtern() const
{
    return this->_isExtern;
}

void LILClassDecl::setIsExtern(bool value)
{
    this->_isExtern = value;
}

void LILClassDecl::addAlias(std::shared_ptr<LILAliasDecl> value)
{
    this->_aliases.push_back(value);
}

const std::vector<std::shared_ptr<LILAliasDecl>> & LILClassDecl::getAliases() const
{
    return this->_aliases;
}
