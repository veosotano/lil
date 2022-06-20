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
#include "LILFunctionType.h"
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
}

std::shared_ptr<LILFlowControlCall> LILFlowControlCall::clone() const
{
    return std::static_pointer_cast<LILFlowControlCall> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFlowControlCall::cloneImpl() const
{
    std::shared_ptr<LILFlowControlCall> clone(new LILFlowControlCall(*this));
    LILNode::cloneChildNodes(clone);
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
    auto childNodes = this->getChildNodes();
    if (childNodes.size() > 0) {
        return childNodes.front();;
    }
    return nullptr;
}

std::shared_ptr<LILType> LILFlowControlCall::getType() const
{
    auto parent = this->getParentNode();
    while (parent) {
        if (parent->isA(NodeTypeFunctionDecl)) {
            auto parentTy = parent->getType();
            if (parentTy && parentTy->isA(TypeTypeFunction)) {
                auto fnTy = std::static_pointer_cast<LILFunctionType>(parentTy);
                return fnTy->getReturnType();
            }
            break;
        } else {
            parent = parent->getParentNode();
        }
    }
    return nullptr;
}
