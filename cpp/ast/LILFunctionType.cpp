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
 *      This file encapsulates the type for a function
 *
 ********************************************************************/

#include "LILFunctionType.h"

using namespace LIL;

std::shared_ptr<LILFunctionType> LILFunctionType::make(LILString returnTypeName)
{
    auto ret = std::make_shared<LILFunctionType>();
    ret->setName("fn");
    auto returnTy = std::make_shared<LILType>();
    returnTy->setName(returnTypeName);
    ret->setReturnType(returnTy);
    return ret;
}

LILFunctionType::LILFunctionType()
: LILType(TypeTypeFunction)
, _receivesReturnType(false)
, _isVariadic(false)
{
    
}

LILFunctionType::LILFunctionType(const LILFunctionType &other)
: LILType(other)
{
    this->_arguments = other._arguments;
    this->_returnType = other._returnType;
    this->_receivesReturnType = other._receivesReturnType;
    this->_isVariadic = other._isVariadic;
}

std::shared_ptr<LILFunctionType> LILFunctionType::clone() const
{
    return std::static_pointer_cast<LILFunctionType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFunctionType::cloneImpl() const
{
    std::shared_ptr<LILFunctionType> clone(new LILFunctionType(*this));
    for (const auto & node : this->_arguments) {
        clone->addArgument(node->clone());
    }
    clone->setReturnType(this->getReturnType()->clone());
    return clone;
}

LILFunctionType::~LILFunctionType()
{
    
}

bool LILFunctionType::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILType::equalTo(otherNode)) return false;
    std::shared_ptr<LILFunctionType> castedNode = std::static_pointer_cast<LILFunctionType>(otherNode);
    if ( this->_receivesReturnType != castedNode->_receivesReturnType ) return false;
    for (size_t i = 0, j = this->_arguments.size(); i<j; ++i) {
        if (!this->_arguments[i]->equalTo(castedNode->_arguments[i])) return false;
    }
    if (this->_returnType && !castedNode->_returnType) return false;
    if (!this->_returnType && castedNode->_returnType) return false;
    if ( this->_returnType && ! this->_returnType->equalTo(castedNode->_returnType) ) return false;
    return true;
}

void LILFunctionType::receiveNodeData(const LIL::LILString &data)
{
    this->setName(data);
}

LILString LILFunctionType::stringRep()
{
    LILString name = this->getName();
    
    name += "(";
    auto args = this->getArguments();
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
    if (this->getIsVariadic()) {
        name += "...";
    }
    name += ")";
    std::shared_ptr<LILFunctionType> retTy = std::static_pointer_cast<LILFunctionType>(this->getReturnType());
    if (retTy) {
        name += "=>";
        name += retTy->getName();
    }

    return name;
}

void LILFunctionType::addArgument(std::shared_ptr<LILType> node)
{
    this->addNode(node);
    this->_arguments.push_back(node);
}

void LILFunctionType::prependArgument(std::shared_ptr<LILType> node)
{
    this->addNode(node);

    std::vector<std::shared_ptr<LILType>> newVector;
    newVector.push_back(node);
    for (auto existing : this->_arguments) {
        newVector.push_back(existing);
    }
    this->_arguments = newVector;
}

std::vector<std::shared_ptr<LILType>> LILFunctionType::getArguments() const
{
    return this->_arguments;
}

void LILFunctionType::setReturnType(std::shared_ptr<LILType> node)
{
    this->addNode(node);
    this->_returnType = node;
}

std::shared_ptr<LILType> LILFunctionType::getReturnType() const
{
    return this->_returnType;
}

void LILFunctionType::setReceivesReturnType(bool value)
{
    this->_receivesReturnType = value;
}

bool LILFunctionType::getReceivesReturnType() const
{
    return this->_receivesReturnType;
}

void LILFunctionType::setIsVariadic(bool value)
{
    this->_isVariadic = value;
}

bool LILFunctionType::getIsVariadic() const
{
    return this->_isVariadic;
}
