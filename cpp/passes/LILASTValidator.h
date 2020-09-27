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
 *      This file checks the AST for logical coherence
 *
 ********************************************************************/

#ifndef LILASTVALIDATOR_H
#define LILASTVALIDATOR_H

#include "LILVisitor.h"
#include "LILErrorMessage.h"
#include "LILNode.h"

#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCombinator.h"
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILInstruction.h"
#include "LILNullLiteral.h"
#include "LILNumberLiteral.h"
#include "LILObjectDefinition.h"
#include "LILPercentageLiteral.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILRule.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILType.h"
#include "LILVarDecl.h"
#include "LILVarName.h"


namespace LIL
{
    class LILToStrInfo;
    class LILASTValidator : public LILVisitor
    {
    public:
        LILASTValidator();
        virtual ~LILASTValidator();
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void illegalNodeType(LILNode* illegalNode, LILNode * container);

        void validate(LILNode * node);
        void _validate(LILBoolLiteral * value);
        void _validate(LILNumberLiteral * value);
        void _validate(LILPercentageLiteral * value);
        void _validate(LILExpression * value);
        void _validate(LILStringLiteral * value);
        void _validate(LILStringFunction * value);
        void _validate(LILNullLiteral * value);
        void _validate(LILType * value);
        void _validate(LILVarDecl * value);
        void _validate(LILClassDecl * value);
        void _validate(LILObjectDefinition * value);
        void _validate(LILAssignment * value);
        void _validate(LILValuePath * value);
        void _validate(LILPropertyName * value);
        void _validate(LILVarName * value);
        void _validate(LILRule * value);
        void _validate(LILSimpleSelector * value);
        void _validate(LILSelectorChain * value);
        void _validate(LILSelector * value);
        void _validate(LILCombinator * value);
        void _validate(LILFilter * value);
        void _validate(LILFlag * value);
        void _validate(LILFunctionDecl * value);
        void _validate(LILFunctionCall * value);
        void _validate(LILFlowControl * value);
        void _validate(LILFlowControlCall * value);
        void _validate(LILInstruction * value);
        inline void validateChildren(const std::vector<std::shared_ptr<LILNode>> & children);
        
        void setDebug(bool value);
    private:
        bool _debug;
    };
}

#endif
