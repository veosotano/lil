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
 *      This file encapsulates the type for an object
 *
 ********************************************************************/

#include "LILObjectType.h"

using namespace LIL;

LILObjectType::LILObjectType()
: LILType(TypeTypeObject)
{
    
}

LILObjectType::LILObjectType(const LILObjectType &other)
: LILType(other)
{

}

std::shared_ptr<LILObjectType> LILObjectType::clone() const
{
    return std::static_pointer_cast<LILObjectType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILObjectType::cloneImpl() const
{
    std::shared_ptr<LILObjectType> clone(new LILObjectType(*this));
    return clone;
}

LILObjectType::~LILObjectType()
{
    
}

bool LILObjectType::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILType::equalTo(otherNode)) return false;
    std::shared_ptr<LILObjectType> castedNode = std::static_pointer_cast<LILObjectType>(otherNode);
    
    return true;
}

void LILObjectType::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

LILString LILObjectType::stringRep()
{
    return "@" + this->getName();
}
