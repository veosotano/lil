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
#include "LILDocumentation.h"
#include "LILFunctionDecl.h"
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
, _docs(other._docs)
, _other(other._other)
, _tmplParams(other._tmplParams)
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
    for (auto methodPair : this->_methods) {
        clone->addMethod(methodPair.first, methodPair.second->clone());
    }
    clone->_aliases.clear();
    for (auto alias : this->_aliases) {
        clone->addAlias(alias->clone());
    }
    clone->_docs.clear();
    for (auto doc : this->_docs) {
        clone->addDoc(doc->clone());
    }
    clone->_other.clear();
    for (auto other : this->_other) {
        clone->addOther(other->clone());
    }
    clone->_tmplParams.clear();
    for (auto tmplParam : this->_tmplParams) {
        clone->addTmplParam(tmplParam->clone());
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

size_t LILClassDecl::getIndexOfField(std::shared_ptr<LILNode> field, bool & found) const
{
    auto fields = this->getFields();
    size_t theIndex = 0;
    for (const auto & fld : fields) {
        if (fld->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(fld);
            if (vd->getIsVVar()) {
                continue;
            }
            if (field->equalTo(fld)) {
                found = true;
                return theIndex;
            }
            theIndex += 1;
        }
    }
    found = false;
    return theIndex;
}

void LILClassDecl::addMethod(std::string name, std::shared_ptr<LILNode> value)
{
    this->addNode(value);
    this->_methods[name] = value;
}

const std::unordered_map<std::string, std::shared_ptr<LILNode>> & LILClassDecl::getMethods() const
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
    auto namestr = name.data();
    if (_methods.count(namestr)) {
        return this->_methods.at(namestr);
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

void LILClassDecl::addDoc(std::shared_ptr<LILDocumentation> value)
{
    this->addNode(value);
    this->_docs.push_back(value);
}

const std::vector<std::shared_ptr<LILDocumentation>> & LILClassDecl::getDocs() const
{
    return this->_docs;
}

void LILClassDecl::add(std::shared_ptr<LILNode> node)
{
    switch (node->getNodeType())
    {
        case NodeTypeType:
        {
            if (this->getReceivesInherits()) {
                this->setInheritType(node);
            } else {
                this->setType(std::static_pointer_cast<LILType>(node));
            }
            break;
        }
        case NodeTypeVarDecl:
        {
            this->addField(node);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
            auto fnName = fd->getName();
            this->addMethod(fnName.data(), fd);
            //install arguments as local variables
            for ( auto arg : fd->getFnType()->getArguments()) {
                if (arg->isA(NodeTypeVarDecl)) {
                    auto argVd = std::static_pointer_cast<LILVarDecl>(arg);
                    fd->setLocalVariable(argVd->getName(), argVd);
                }
            }
            break;
        }
        case NodeTypeTypeDecl:
        {
            if (this->getReceivesBody()) {
                //fixme: not just aliases on classes
                //this->addType(std::static_pointer_cast<LILTypeDecl>(node));
            } else {
                this->addTmplParam(node);
            }
            break;
        }
        case NodeTypeAliasDecl:
        {
            if (this->getReceivesBody()) {
                auto ad = std::static_pointer_cast<LILAliasDecl>(node);
                this->addAlias(ad);
            }
            break;
        }
        case NodeTypeDocumentation:
        {
            this->addDoc(std::static_pointer_cast<LILDocumentation>(node));
            break;
        }
        case NodeTypeInstruction:
        {
            auto instrType = node->getInstructionType();
            if (
                instrType == InstructionTypeExpand
                || instrType == InstructionTypeResource
            ) {
                for (auto child : node->getChildNodes()) {
                    this->add(child);
                }
            } else {
                this->addOther(node);
            }
            break;
        }
        default:
        {
            this->addOther(node);
            break;
        }
    }
}

void LILClassDecl::addOther(std::shared_ptr<LILNode> value)
{
    this->addNode(value);
    this->_other.push_back(value);
}

const std::vector<std::shared_ptr<LILNode>> & LILClassDecl::getOther() const
{
    return this->_other;
}

void LILClassDecl::clearOther()
{
    this->_other.clear();
}

void LILClassDecl::setOther(const std::vector<std::shared_ptr<LILNode>> && other)
{
    this->_other = std::move(other);
}

bool LILClassDecl::isTemplate() const
{
    return this->_tmplParams.size() > 0;
}

const std::vector<std::shared_ptr<LILNode>> & LILClassDecl::getTmplParams() const
{
    return this->_tmplParams;
}

void LILClassDecl::addTmplParam(std::shared_ptr<LILNode> value)
{
    value->setParentNode(this->shared_from_this());
    this->_tmplParams.push_back(value);
}

void LILClassDecl::setTmplParams(const std::vector<std::shared_ptr<LILNode>> && values)
{
    this->_tmplParams = std::move(values);
}

void LILClassDecl::clearTmplParams()
{
    this->_tmplParams.clear();
}
