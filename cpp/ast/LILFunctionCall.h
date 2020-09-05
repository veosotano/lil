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


#ifndef LILFUNCTIONCALL_H
#define LILFUNCTIONCALL_H


#include "LILNode.h"

namespace LIL
{
    class LILType;
    class LILFunctionCall : public LILNode
    {
    public:
        LILFunctionCall();
        LILFunctionCall(const LILFunctionCall &other);
        std::shared_ptr<LILFunctionCall> clone() const;
        virtual ~LILFunctionCall();

        void receiveNodeData(const LILString &data) override;
        
        FunctionCallType getFunctionCallType() const override;
        void setFunctionCallType(FunctionCallType newType);
        bool isA(FunctionCallType otherType) const override;
        LILString stringRep() override;
        
        void addArgument(std::shared_ptr<LILNode> arg);
        void setArguments(std::vector<std::shared_ptr<LILNode>> args);
        std::vector<std::shared_ptr<LILNode>> getArguments() const;
        
        void setTypes(std::vector<std::shared_ptr<LILType>> types);
        std::vector<std::shared_ptr<LILType>> getTypes() const;
        
    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        FunctionCallType _functionCallType;
        std::vector<std::shared_ptr<LILType>> _types;
    };
}

#endif
