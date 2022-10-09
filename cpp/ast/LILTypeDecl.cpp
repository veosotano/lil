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
 *	  This file encapsulates a type declaration
 *
 ********************************************************************/

#include "LILTypeDecl.h"
#include "LILType.h"

using namespace LIL;

LILTypeDecl::LILTypeDecl()
: LILNode(NodeTypeTypeDecl)
{
	
}

LILTypeDecl::LILTypeDecl(const LILTypeDecl &other)
: LILNode(other)
, _srcTy(other._srcTy)
, _dstTy(other._dstTy)
{
}

std::shared_ptr<LILTypeDecl> LILTypeDecl::clone() const
{
	return std::static_pointer_cast<LILTypeDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILTypeDecl::cloneImpl() const
{
	std::shared_ptr<LILTypeDecl> clone(new LILTypeDecl(*this));
	clone->clearChildNodes();

	if (this->_srcTy) {
		clone->setSrcType(this->_srcTy->clone());
	}
	if (this->_dstTy) {
		clone->setDstType(this->_dstTy->clone());
	}
	return clone;
}

LILTypeDecl::~LILTypeDecl()
{
	
}

void LILTypeDecl::receiveNodeData(const LIL::LILString &data)
{
	//do nothing
}

const std::shared_ptr<LILType> & LILTypeDecl::getSrcType() const
{
	return this->_srcTy;
}

void LILTypeDecl::setSrcType(std::shared_ptr<LILType> value)
{
	this->addNode(value);
	this->_srcTy = value;
}

const std::shared_ptr<LILType> & LILTypeDecl::getDstType() const
{
	return this->_dstTy;
}

void LILTypeDecl::setDstType(std::shared_ptr<LILType> value)
{
	this->addNode(value);
	this->_dstTy = value;
}
