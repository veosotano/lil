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

#include "LILFlowControlCall.h"
#include "LILType.h"

using namespace LIL;

LILFlowControlCall::LILFlowControlCall()
: LIL::LILNode(NodeTypeFlowControlCall)
{
    this->_flowControlCallType = FlowControlCallTypeNone;
}

LILFlowControlCall::LILFlowControlCall(const LILFlowControlCall &other)
: LILNode(other)
{
    this->_flowControlCallType = other._flowControlCallType;
    this->_types = other._types;
}

std::shared_ptr<LILFlowControlCall> LILFlowControlCall::clone() const
{
    return std::static_pointer_cast<LILFlowControlCall> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFlowControlCall::cloneImpl() const
{
    std::shared_ptr<LILFlowControlCall> clone(new LILFlowControlCall(*this));
    clone->_types.clear();
    for (auto ty : this->_types) {
        clone->_types.push_back(ty->clone());
    }
    
    return clone;
}

LILFlowControlCall::~LILFlowControlCall()
{
    
}

void LILFlowControlCall::receiveNodeData(const LILString &data)
{
    if (data == "return")
    {
        this->setFlowControlCallType(FlowControlCallTypeReturn);
    }
    else if (data == "repeat")
    {
        this->setFlowControlCallType(FlowControlCallTypeRepeat);
    }
    else if (data == "continue")
    {
        this->setFlowControlCallType(FlowControlCallTypeContinue);
    }
    else if (data == "break")
    {
        this->setFlowControlCallType(FlowControlCallTypeBreak);
    }
}


LILString LILFlowControlCall::stringRep()
{
    switch (this->getFlowControlCallType())
    {
        case FlowControlCallTypeReturn:
        {
            return "return";
        }
        case FlowControlCallTypeRepeat:
        {
            return "repeat";
        }
        case FlowControlCallTypeContinue:
        {
            return "continue";
        }
        case FlowControlCallTypeBreak:
        {
            return "break";
        }
   
        default:
            break;
    }
    return "";
}

FlowControlCallType LILFlowControlCall::getFlowControlCallType() const
{
    return this->_flowControlCallType;
}

void LILFlowControlCall::setFlowControlCallType(FlowControlCallType newType)
{
    this->_flowControlCallType = newType;
}

bool LILFlowControlCall::isA(FlowControlCallType otherType) const
{
    return this->_flowControlCallType == otherType;
}

void LILFlowControlCall::setArgument(std::shared_ptr<LILNode> arg)
{
    this->addNode(arg);
}

std::shared_ptr<LILNode> LILFlowControlCall::getArgument() const
{
    return this->getChildNodes().front();
}

void LILFlowControlCall::setTypes(std::vector<std::shared_ptr<LILType>> types)
{
    this->_types = std::move(types);
}
