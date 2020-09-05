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

using namespace LIL;

LILFunctionCall::LILFunctionCall()
: LIL::LILNode(NodeTypeFunctionCall)
{
    this->_functionCallType = FunctionCallTypeNone;
}

LILFunctionCall::LILFunctionCall(const LILFunctionCall &other)
: LILNode(other)
{
    this->_functionCallType = other._functionCallType;
    this->_types = other._types;
}

std::shared_ptr<LILFunctionCall> LILFunctionCall::clone() const
{
    return std::static_pointer_cast<LILFunctionCall> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFunctionCall::cloneImpl() const
{
    std::shared_ptr<LILFunctionCall> clone(new LILFunctionCall(*this));
    clone->_types.clear();
    for (auto ty : this->_types) {
        clone->_types.push_back(ty->clone());
    }
    
    return clone;
}

LILFunctionCall::~LILFunctionCall()
{

}

void LILFunctionCall::receiveNodeData(const LILString &data)
{
    if (data == "has")
    {
        this->setFunctionCallType(FunctionCallTypeHas);
    }
    else if (data == "atLeast")
    {
        this->setFunctionCallType(FunctionCallTypeAtLeast);
    }
    else if (data == "atMost")
    {
        this->setFunctionCallType(FunctionCallTypeAtMost);
    }
    else if (data == "clamp")
    {
        this->setFunctionCallType(FunctionCallTypeClamp);
    }
    else if (data == "floor")
    {
        this->setFunctionCallType(FunctionCallTypeFloor);
    }
    else if (data == "ceil")
    {
        this->setFunctionCallType(FunctionCallTypeCeil);
    }
    else if (data == "round")
    {
        this->setFunctionCallType(FunctionCallTypeRound);
    }
    else if (data == "ref")
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
    else if (data == "print")
    {
        this->setFunctionCallType(FunctionCallTypePrint);
    }
    else if (data == "startTimer")
    {
        this->setFunctionCallType(FunctionCallTypeStartTimer);
    }
    else if (data == "stopTimer")
    {
        this->setFunctionCallType(FunctionCallTypeStopTimer);
    }
    else if (data == "toggleTimer")
    {
        this->setFunctionCallType(FunctionCallTypeToggleTimer);
    }
}


LILString LILFunctionCall::stringRep()
{
    switch (this->getFunctionCallType())
    {
        case FunctionCallTypeHas:
        {
            return "has";
        }
        case FunctionCallTypeAtLeast:
        {
            return "atLeast";
        }
        case FunctionCallTypeAtMost:
        {
            return "atMost";
        }
        case FunctionCallTypeClamp:
        {
            return "clamp";
        }
        case FunctionCallTypeFloor:
        {
            return "floor";
        }
        case FunctionCallTypeCeil:
        {
            return "ceil";
        }
        case FunctionCallTypeRound:
        {
            return "round";
        }
        case FunctionCallTypeRef:
        {
            return "ref";
        }
        case FunctionCallTypeSel:
        {
            return "sel";
        }
        case FunctionCallTypeFlag:
        {
            return "flag";
        }
        case FunctionCallTypeUnflag:
        {
            return "unflag";
        }
        case FunctionCallTypeAddFlag:
        {
            return "addFlag";
        }
        case FunctionCallTypeTakeFlag:
        {
            return "takeFlag";
        }
        case FunctionCallTypeReplaceFlag:
        {
            return "replaceFlag";
        }
        case FunctionCallTypePrint:
        {
            return "print";
        }
        case FunctionCallTypeStartTimer:
        {
            return "startTimer";
        }
        case FunctionCallTypeStopTimer:
        {
            return "stopTimer";
        }
        case FunctionCallTypeToggleTimer:
        {
            return "toggleTimer";
        }

        default:
            break;
    }
    return "";
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

std::vector<std::shared_ptr<LILNode>> LILFunctionCall::getArguments() const
{
    return this->getChildNodes();
}

void LILFunctionCall::setTypes(std::vector<std::shared_ptr<LILType>> types)
{
    this->_types = std::move(types);
}

std::vector<std::shared_ptr<LILType>> LILFunctionCall::getTypes() const
{
    return this->_types;
}
