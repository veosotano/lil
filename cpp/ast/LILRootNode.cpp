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
#include "LILAliasDecl.h"
#include "LILClassDecl.h"
#include "LILConversionDecl.h"
#include "LILFunctionDecl.h"
#include "LILInstruction.h"
#include "LILNodeToString.h"
#include "LILSnippetInstruction.h"
#include "LILTypeDecl.h"
#include "LILVarDecl.h"

using namespace LIL;

LILRootNode::LILRootNode()
: LILVarNode(NodeTypeRoot)
{
    this->_mainFunction = std::make_shared<LILFunctionDecl>();
    this->_mainFunction->setFunctionDeclType(FunctionDeclTypeFn);
    this->_mainFunction->setName("main");
    this->_mainFunction->setHasOwnType(true);
    auto ty = LILFunctionType::make("i64");
    this->_mainFunction->setType(ty);
}

LILRootNode::LILRootNode(const LILRootNode & other)
: LILVarNode(other)
, _localVars(other._localVars)
, _classes(other._classes)
, _aliases(other._aliases)
, _types(other._types)
, _conversions(other._conversions)
, _constants(other._constants)
, _snippets(other._snippets)
{

}

LILRootNode::~LILRootNode()
{
    
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

void LILRootNode::clearNodes()
{
    this->clearChildNodes();
}

void LILRootNode::appendNodes(const std::vector<std::shared_ptr<LILNode>> & nodes)
{
    this->_childNodes.insert(this->_childNodes.begin() + this->_childNodes.size(), nodes.begin(), nodes.end());
}

void LILRootNode::addClass(std::shared_ptr<LILClassDecl> value)
{
    this->_classes.push_back(value);
}

const std::vector<std::shared_ptr<LILClassDecl>> & LILRootNode::getClasses() const
{
    return this->_classes;
}

void LILRootNode::removeClass(std::shared_ptr<LILClassDecl> value)
{
    auto it = std::find(this->_classes.begin(), this->_classes.end(), value);
    if (it != this->_classes.end()) {
        this->_classes.erase(it);
    }
}

void LILRootNode::addAlias(std::shared_ptr<LILAliasDecl> value)
{
    this->_aliases.push_back(value);
}

const std::vector<std::shared_ptr<LILAliasDecl>> & LILRootNode::getAliases() const
{
    return this->_aliases;
}

void LILRootNode::addType(std::shared_ptr<LILTypeDecl> value)
{
    this->_types.push_back(value);
}

const std::vector<std::shared_ptr<LILTypeDecl>> & LILRootNode::getTypes() const
{
    return this->_types;
}

void LILRootNode::addConversion(std::shared_ptr<LILConversionDecl> value)
{
    auto vd = value->getVarDecl();
    if (!vd) {
        std::cerr << "!!!!!! CONVERSION DECL HAD NO VAR DECL FAIL !!!!!!\n\n";
        return;
    }
    auto fromTy = vd->getType();
    if (!fromTy) {
        std::cerr << "!!!!!! VAR DECL OF CONVERSION DECL HAD NO TYPE FAIL !!!!!!\n\n";
        return;
    }
    auto fromTyName = LILNodeToString::stringify(fromTy.get());

    auto toTy = value->getType();
    if (!toTy) {
        std::cerr << "!!!!!! CONVERSION DECL HAD NO TARGET TYPE FAIL !!!!!!\n\n";
        return;
    }
    auto toTyName = LILNodeToString::stringify(toTy.get());
    
    this->_conversions[fromTyName+"_to_"+toTyName] = value;
}

const std::map<LILString, std::shared_ptr<LILConversionDecl>> & LILRootNode::getConversions() const
{
    return this->_conversions;
}

std::shared_ptr<LILConversionDecl> LILRootNode::getConversionNamed(LILString name)
{
    if (this->_conversions.count(name)) {
        return this->_conversions[name];
    }
    return nullptr;

}

const std::vector<std::shared_ptr<LILVarDecl>> & LILRootNode::getConstants() const
{
    return this->_constants;
}

void LILRootNode::addConstant(std::shared_ptr<LILVarDecl> value)
{
    this->_constants.push_back(value);
}

const std::map<LILString, std::shared_ptr<LILSnippetInstruction>> & LILRootNode::getSnippets() const
{
    return this->_snippets;
}

std::shared_ptr<LILSnippetInstruction> LILRootNode::getSnippetNamed(LILString key)
{
    if (this->_snippets.count(key)) {
        return this->_snippets.at(key);
    }
    return nullptr;
}

void LILRootNode::addSnippet(std::shared_ptr<LILSnippetInstruction> snippet)
{
    this->_snippets[snippet->getName()] = snippet;
}

void LILRootNode::addEvaluable(std::shared_ptr<LILNode> node)
{
    this->_initializers.push_back(node);
}

