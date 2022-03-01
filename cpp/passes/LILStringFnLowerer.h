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
 *      This file converts color instructions into object definitions
 *
 ********************************************************************/

#ifndef LILSTRINGFNLOWERER_H
#define LILSTRINGFNLOWERER_H

#include "LILVisitor.h"

namespace LIL
{
    class LILAssignment;
    class LILConversionDecl;
    class LILEnum;
    class LILExpression;
    class LILFlowControl;
    class LILFlowControlCall;
    class LILFunctionCall;
    class LILFunctionDecl;
    class LILInstruction;
    class LILObjectDefinition;
    class LILRootNode;
    class LILStringFunction;
    class LILUnaryExpression;
    class LILValueList;
    class LILVarDecl;
    
    class LILStringFnLowerer : public LILVisitor
    {
    public:
        LILStringFnLowerer();
        virtual ~LILStringFnLowerer();
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        bool processStringFn(std::shared_ptr<LILNode> node);
        
    private:
        std::vector<std::vector<std::shared_ptr<LILNode>>> _nodeBuffer;
        size_t _count;
        
        bool _processStringFn(std::shared_ptr<LILStringFunction> value);
        bool _processStringFn(std::shared_ptr<LILExpression> value);
        bool _processStringFn(std::shared_ptr<LILUnaryExpression> value);
        bool _processStringFn(std::shared_ptr<LILVarDecl> value);
        bool _processStringFn(std::shared_ptr<LILObjectDefinition> value);
        bool _processStringFn(std::shared_ptr<LILAssignment> value);
        bool _processStringFn(std::shared_ptr<LILFunctionDecl> value);
        bool _processStringFn(std::shared_ptr<LILFunctionCall> value);
        bool _processStringFn(std::shared_ptr<LILFlowControl> value);
        bool _processStringFn(std::shared_ptr<LILFlowControlCall> value);
        bool _processStringFn(std::shared_ptr<LILInstruction> value);
        bool _processStringFn(std::shared_ptr<LILValueList> value);
        bool _processStringFn(std::shared_ptr<LILConversionDecl> value);
        bool _processStringFn(std::shared_ptr<LILEnum> value);
    };
}

#endif /* LILSTRINGFNLOWERER_H */
