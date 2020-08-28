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
    this->_fields = other._fields;
}

std::shared_ptr<LILObjectType> LILObjectType::clone() const
{
    return std::static_pointer_cast<LILObjectType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILObjectType::cloneImpl() const
{
    std::shared_ptr<LILObjectType> clone(new LILObjectType(*this));
    for (const auto & node : this->_fields) {
        clone->addField(node->clone());
    }
    return clone;
}

LILObjectType::~LILObjectType()
{
    
}

bool LILObjectType::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILType::equalTo(otherNode)) return false;
    std::shared_ptr<LILObjectType> castedNode = std::static_pointer_cast<LILObjectType>(otherNode);
    
    for (size_t i = 0, j = this->_fields.size(); i<j; ++i) {
        if (!this->_fields[i]->equalTo(castedNode->_fields[i])) return false;
    }

    return true;
}

void LILObjectType::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

LILString LILObjectType::stringRep()
{
    LILString name = "@" + this->getName();
    
    name += "(";
    auto args = this->getFields();
    for (size_t i=0, j=args.size(); i<j; ++i) {
        std::shared_ptr<LILNode> arg = args[i];
        if (arg && arg->isA(NodeTypeType)) {
            std::shared_ptr<LILType> ty = std::static_pointer_cast<LILType>(arg);
            name += ty->stringRep();
            if ((i+1)<j) {
                name += ",";
            }
        }
    }
    name += ")";

    return name;
}

void LILObjectType::addField(std::shared_ptr<LILType> node)
{
    this->addNode(node);
    this->_fields.push_back(node);
}

std::vector<std::shared_ptr<LILType>> LILObjectType::getFields() const
{
    return this->_fields;
}
