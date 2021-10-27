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
    class LILValuePath;
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

        void addArgument(std::shared_ptr<LILNode> arg);
        void setArguments(std::vector<std::shared_ptr<LILNode>> args);
        std::vector<std::shared_ptr<LILNode>> getArguments() const;
        void clearArguments();

        void setArgumentTypes(std::vector<std::shared_ptr<LILType>> types);
        void addArgumentType(std::shared_ptr<LILType> argTy);
        std::vector<std::shared_ptr<LILType>> getArgumentTypes() const;

        void setReturnType(std::shared_ptr<LILType> retTy);
        std::shared_ptr<LILType> getReturnType() const;

        std::shared_ptr<LILType> getType() const override;

        std::shared_ptr<LILValuePath> getSubject() const;
        void setName(LILString newName);
        const LILString getName() const;

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;

    private:
        FunctionCallType _functionCallType;
        std::vector<std::shared_ptr<LILType>> _argumentTypes;
        std::shared_ptr<LILType> _returnType;
        LILString _name;
    };
}

#endif
