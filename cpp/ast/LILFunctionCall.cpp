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
 *      This file represents a function call
 *
 ********************************************************************/

#include "LILFunctionCall.h"
#include "LILType.h"
#include "LILValuePath.h"

using namespace LIL;

LILFunctionCall::LILFunctionCall()
: LIL::LILNode(NodeTypeFunctionCall)
{
    this->_functionCallType = FunctionCallTypeNone;
    this->_returnType = nullptr;
}

LILFunctionCall::LILFunctionCall(const LILFunctionCall &other)
: LILNode(other)
{
    this->_functionCallType = other._functionCallType;
    this->_argumentTypes = other._argumentTypes;
    this->_returnType = other._returnType;
    this->_name = other._name;
}

std::shared_ptr<LILFunctionCall> LILFunctionCall::clone() const
{
    return std::static_pointer_cast<LILFunctionCall> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFunctionCall::cloneImpl() const
{
    std::shared_ptr<LILFunctionCall> clone(new LILFunctionCall(*this));
    LILNode::cloneChildNodes(clone);

    clone->_argumentTypes.clear();
    for (auto ty : this->_argumentTypes) {
        clone->_argumentTypes.push_back(ty->clone());
    }
    if (this->_returnType) {
        clone->setReturnType(this->_returnType->clone());
    }

    return clone;
}

LILFunctionCall::~LILFunctionCall()
{

}

void LILFunctionCall::receiveNodeData(const LILString &data)
{
    if (data == "ref")
    {
        this->setFunctionCallType(FunctionCallTypeRef);
    }
    else if (data == "sel")
    {
        this->setFunctionCallType(FunctionCallTypeSel);
    }
    else if (data == "flag")
    {
        this->setFunctionCallType(FunctionCallTypeFlag);
    }
    else if (data == "unflag")
    {
        this->setFunctionCallType(FunctionCallTypeUnflag);
    }
    else if (data == "addFlag")
    {
        this->setFunctionCallType(FunctionCallTypeAddFlag);
    }
    else if (data == "takeFlag")
    {
        this->setFunctionCallType(FunctionCallTypeTakeFlag);
    }
    else if (data == "replaceFlag")
    {
        this->setFunctionCallType(FunctionCallTypeReplaceFlag);
    }
    else if (data == "pointerTo")
    {
        this->setFunctionCallType(FunctionCallTypePointerTo);
    }
    else if (data == "valueOf")
    {
        this->setFunctionCallType(FunctionCallTypeValueOf);
    }
    else if (data == "set")
    {
        this->setFunctionCallType(FunctionCallTypeSet);
    }
    else if (data == "sizeOf")
    {
        this->setFunctionCallType(FunctionCallTypeSizeOf);
    }
    this->_name = data;
}

FunctionCallType LILFunctionCall::getFunctionCallType() const
{
    return this->_functionCallType;
}

void LILFunctionCall::setFunctionCallType(FunctionCallType newType)
{
    this->_functionCallType = newType;
}

bool LILFunctionCall::isA(FunctionCallType otherType) const
{
    return this->_functionCallType == otherType;
}

void LILFunctionCall::addArgument(std::shared_ptr<LILNode> arg)
{
    this->addNode(arg);
}

void LILFunctionCall::setArguments(std::vector<std::shared_ptr<LILNode>> args)
{
    this->clearChildNodes();
    for (auto arg : args) {
        this->addArgument(arg);
    }
}

void LILFunctionCall::clearArguments()
{
    this->clearChildNodes();
}

std::vector<std::shared_ptr<LILNode>> LILFunctionCall::getArguments() const
{
    return this->getChildNodes();
}

void LILFunctionCall::setArgumentTypes(std::vector<std::shared_ptr<LILType>> types)
{
    this->_argumentTypes = types;
}

std::vector<std::shared_ptr<LILType>> LILFunctionCall::getArgumentTypes() const
{
    return this->_argumentTypes;
}

void LILFunctionCall::setReturnType(std::shared_ptr<LILType> retTy)
{
    this->_returnType = retTy;
}

std::shared_ptr<LILType> LILFunctionCall::getReturnType() const
{
    return this->_returnType;
}

std::shared_ptr<LILType> LILFunctionCall::getType() const
{
    return this->getReturnType();
}

std::shared_ptr<LILValuePath> LILFunctionCall::getSubject() const
{
    switch (this->_functionCallType) {
        case FunctionCallTypeValuePath:
        {
            auto vp = std::static_pointer_cast<LILValuePath>(this->getParentNode());
            if (vp) {
                auto newVp = std::make_shared<LILValuePath>();
                auto nodes = vp->getNodes();
                for (auto node : nodes) {
                    if (node.get() == this) {
                        break;
                    }
                    newVp->addChild(node->clone());
                }
                newVp->setParentNode(vp->getParentNode());
                return newVp;
            }
            break;
        }
        case FunctionCallTypePointerTo:
        {
            auto firstArg = this->getArguments().front();
            if (firstArg->isA(NodeTypeValuePath)) {
                return std::static_pointer_cast<LILValuePath>(firstArg);
            }
            break;
        }
            
        default:
            break;
    }
    return nullptr;
}


void LILFunctionCall::setName(LILString newName)
{
    this->_name = newName;
}

const LILString LILFunctionCall::getName() const
{
    return this->_name;
}
