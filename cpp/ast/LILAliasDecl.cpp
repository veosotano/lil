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
 *      This file encapsulates an alias declaration
 *
 ********************************************************************/

#include "LILAliasDecl.h"
#include "LILType.h"

using namespace LIL;

LILAliasDecl::LILAliasDecl()
: LILNode(NodeTypeAliasDecl)
{
    
}

LILAliasDecl::LILAliasDecl(const LILAliasDecl &other)
: LILNode(other)
, _srcTy(other._srcTy)
, _dstTy(other._dstTy)
{
}

std::shared_ptr<LILAliasDecl> LILAliasDecl::clone() const
{
    return std::static_pointer_cast<LILAliasDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILAliasDecl::cloneImpl() const
{
    std::shared_ptr<LILAliasDecl> clone(new LILAliasDecl(*this));
    clone->clearChildNodes();
    
    if (this->_srcTy) {
        clone->setSrcType(this->_srcTy->clone());
    }
    if (this->_dstTy) {
        clone->setDstType(this->_dstTy->clone());
    }

    return clone;
}

LILAliasDecl::~LILAliasDecl()
{
    
}

void LILAliasDecl::receiveNodeData(const LIL::LILString &data)
{
    //do nothing
}

const std::shared_ptr<LILType> & LILAliasDecl::getSrcType() const
{
    return this->_srcTy;
}

void LILAliasDecl::setSrcType(std::shared_ptr<LILType> value)
{
    this->addNode(value);
    this->_srcTy = value;
}

const std::shared_ptr<LILType> & LILAliasDecl::getDstType() const
{
    return this->_dstTy;
}

void LILAliasDecl::setDstType(std::shared_ptr<LILType> value)
{
    this->addNode(value);
    this->_dstTy = value;
}
