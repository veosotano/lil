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
#include "LILTypeDecl.h"
#include "LILVarDecl.h"

using namespace LIL;

LILRootNode::LILRootNode()
: LILVarNode(NodeTypeRoot)
{
    this->_mainFunction = std::make_shared<LILFunctionDecl>();
    this->_mainFunction->setFunctionDeclType(FunctionDeclTypeFn);
    this->_mainFunction->setName("main");
    
    this->_mainFunctionVarDecl = std::make_shared<LILVarDecl>();
    this->_mainFunctionVarDecl->setName("main");
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
    this->_aliases = other._aliases;
    this->_types = other._types;
    this->_conversions = other._conversions;
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

const std::vector<std::shared_ptr<LILClassDecl>> & LILRootNode::getClasses() const
{
    return this->_classes;
}

void LILRootNode::addDependency(std::shared_ptr<LILInstruction> value)
{
    this->_dependencies.push_back(value);
}

const std::vector<std::shared_ptr<LILInstruction>> & LILRootNode::getDependencies() const
{
    return this->_dependencies;
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
    auto fromTyName = fromTy->stringRep();

    auto toTy = value->getType();
    if (!toTy) {
        std::cerr << "!!!!!! CONVERSION DECL HAD NO TARGET TYPE FAIL !!!!!!\n\n";
        return;
    }
    auto toTyName = toTy->stringRep();
    
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
