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


#ifndef LILFLOWCONTROLCALL_H
#define LILFLOWCONTROLCALL_H


#include "LILNode.h"

namespace LIL
{
    class LILType;
    class LILFlowControlCall : public LILNode
    {
    public:
        LILFlowControlCall();
        LILFlowControlCall(const LILFlowControlCall &other);
        std::shared_ptr<LILFlowControlCall> clone() const;
        virtual ~LILFlowControlCall();
        
        void receiveNodeData(const LILString &data) override;
        
        FlowControlCallType getFlowControlCallType() const override;
        void setFlowControlCallType(FlowControlCallType newType);
        bool isA(FlowControlCallType otherType) const override;
        
        void setArgument(std::shared_ptr<LILNode> arg);
        std::shared_ptr<LILNode> getArgument() const;
        
        std::shared_ptr<LILType> getType() const override;
        
    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        FlowControlCallType _flowControlCallType;
    };
}

#endif
