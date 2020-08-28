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
 *      This file represents if, else, for, swith and loop statements
 *
 ********************************************************************/


#ifndef LILFLOWCONTROL_H
#define LILFLOWCONTROL_H


#include "LILVarNode.h"

namespace LIL
{
    class LILFlowControl : public LILVarNode
    {
    public:
        LILFlowControl();
        LILFlowControl(const LILFlowControl &other);
        std::shared_ptr<LILFlowControl> clone() const;
        virtual ~LILFlowControl();
        LILString stringRep() override;
        void receiveNodeData(const LILString &data) override;

        void addArgument(std::shared_ptr<LILNode> arg);
        const std::vector<std::shared_ptr<LILNode>> & getArguments() const;
        void addThen(std::shared_ptr<LILNode> node);
        const std::vector<std::shared_ptr<LILNode>> & getThen() const;
        void setThen(std::vector<std::shared_ptr<LILNode>> newThen);
        void addElse(std::shared_ptr<LILNode> node);
        const std::vector<std::shared_ptr<LILNode>> & getElse() const;
        void setElse(std::vector<std::shared_ptr<LILNode>> newElse);
        
        void setReceivesFunctionBody(bool newValue);
        bool getReceivesFunctionBody() const;

        void setReceivesElse(bool newValue);
        bool getReceivesElse() const;

        bool isA(FlowControlType type) const override;
        FlowControlType getFlowControlType() const override;
        void setFlowControlType(FlowControlType newType);

        void setReturnType(std::shared_ptr<LILNode> type);
        std::shared_ptr<LILNode> getReturnType() const;
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        std::vector<std::shared_ptr<LILNode>> _arguments;
        std::vector<std::shared_ptr<LILNode>> _then;
        std::vector<std::shared_ptr<LILNode>> _else;
        bool _receivesFunctionBody;
        bool _receivesElse;
        FlowControlType _flowControlType;
        std::shared_ptr<LILNode> _returnType;
    };
}

#endif
