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
 *      This file encapsulates a type declaration
 *
 ********************************************************************/

#include "LILTypeDecl.h"
#include "LILType.h"

using namespace LIL;

LILTypeDecl::LILTypeDecl()
: LILTypedNode(NodeTypeTypeDecl)
{
    
}

LILTypeDecl::LILTypeDecl(const LILTypeDecl &other)
: LILTypedNode(other)
{
    this->_name = other._name;
}

std::shared_ptr<LILTypeDecl> LILTypeDecl::clone() const
{
    return std::static_pointer_cast<LILTypeDecl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILTypeDecl::cloneImpl() const
{
    std::shared_ptr<LILTypeDecl> clone(new LILTypeDecl(*this));
    clone->clearChildNodes();
    
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    
    return clone;
}

LILTypeDecl::~LILTypeDecl()
{
    
}

LILString LILTypeDecl::getName() const
{
    return this->_name;
}

void LILTypeDecl::setName(LILString value)
{
    this->_name = value;
}

void LILTypeDecl::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}
