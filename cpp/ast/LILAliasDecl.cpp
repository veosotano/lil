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
: LILTypedNode(NodeTypeAliasDecl)
{
    
}

LILAliasDecl::LILAliasDecl(const LILAliasDecl &other)
: LILTypedNode(other)
{
    this->_name = other._name;
}

std::shared_ptr<LILAliasDecl> LILAliasDecl::clone() const
{
    return std::static_pointer_cast<LILAliasDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILAliasDecl::cloneImpl() const
{
    std::shared_ptr<LILAliasDecl> clone(new LILAliasDecl(*this));
    clone->clearChildNodes();
    
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }

    return clone;
}

LILAliasDecl::~LILAliasDecl()
{
    
}

LILString LILAliasDecl::getName() const
{
    return this->_name;
}

void LILAliasDecl::setName(LILString value)
{
    this->_name = value;
}

void LILAliasDecl::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}
