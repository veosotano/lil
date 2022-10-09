/********************************************************************
 *
 *	  LIL Is a Language
 *
 *	  AUTHORS: Miro Keller
 *
 *	  COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *	  LICENSE: see LICENSE file
 *
 *	  This file is used to declare implicit conversion between types
 *
 ********************************************************************/

#include "LILConversionDecl.h"
#include "LILNodeToString.h"
#include "LILType.h"
#include "LILVarDecl.h"

using namespace LIL;

LILConversionDecl::LILConversionDecl()
: LILTypedNode(NodeTypeConversionDecl)
{
	
}

LILConversionDecl::LILConversionDecl(const LILConversionDecl &other)
: LILTypedNode(other)
{
	this->_varDecl = other._varDecl;
	this->_body = other._body;
	this->_srcTyName = other._srcTyName;
}

std::shared_ptr<LILConversionDecl> LILConversionDecl::clone() const
{
	return std::static_pointer_cast<LILConversionDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILConversionDecl::cloneImpl() const
{
	std::shared_ptr<LILConversionDecl> clone(new LILConversionDecl(*this));
	clone->clearChildNodes();
	
	if (this->_varDecl) {
		clone->setVarDecl(this->_varDecl->clone());
	}

	//clone LILTypedNode
	if (this->_type) {
		clone->setType(this->_type->clone());
	}
	
	clone->_body.clear();
	for (auto node : this->_body) {
		clone->addEvaluable(node->clone());
	}
	return clone;
}

LILConversionDecl::~LILConversionDecl()
{
	
}

void LILConversionDecl::setType(std::shared_ptr<LILType> value)
{
	LILTypedNode::setType(value);
	if (this->_destTyName.length() == 0) {
		this->_destTyName = LILNodeToString::stringify(value.get());
	}
}

std::shared_ptr<LILVarDecl> LILConversionDecl::getVarDecl() const
{
	return this->_varDecl;
}

void LILConversionDecl::setVarDecl(std::shared_ptr<LILVarDecl> value)
{
	this->_varDecl = value;
	auto ty = value->getType();
	if (ty) {
		this->_srcTyName = LILNodeToString::stringify(ty.get());
	}
	this->addNode(value);
}

void LILConversionDecl::receiveNodeData(const LIL::LILString &data)
{
	//do nothing
}

void LILConversionDecl::addEvaluable(std::shared_ptr<LILNode> node)
{
	this->addNode(node);
	this->_body.push_back(node);
}

const std::vector<std::shared_ptr<LILNode>> & LILConversionDecl::getBody() const
{
	return this->_body;
}

void LILConversionDecl::setBody(std::vector<std::shared_ptr<LILNode>> newBody)
{
	for (auto node : newBody) {
		node->setParentNode(nullptr);
	}
	this->clearBody();
	for (const auto & node : newBody) {
		this->addEvaluable(node);
	}
}

void LILConversionDecl::clearBody()
{
	for (auto it = this->_body.rbegin(); it != this->_body.rend(); ++it){
		this->removeNode(*it);
	}
	this->_body.clear();
}

LILString LILConversionDecl::encodedName()
{
	return this->_srcTyName + "_to_" + this->_destTyName;
}
