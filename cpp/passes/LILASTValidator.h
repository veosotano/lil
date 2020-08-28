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
        void validate(LILBoolLiteral value);
        void validate(LILNumberLiteral value);
        void validate(LILPercentageLiteral value);
        void validate(LILExpression value);
        void validate(LILStringLiteral value);
        void validate(LILStringFunction value);
        void validate(LILNullLiteral value);
        void validate(LILType value);
        void validate(LILVarDecl value);
        void validate(LILClassDecl value);
        void validate(LILObjectDefinition value);
        void validate(LILAssignment value);
        void validate(LILValuePath value);
        void validate(LILPropertyName value);
        void validate(LILVarName value);
        void validate(LILRule value);
        void validate(LILSimpleSelector value);
        void validate(LILSelectorChain value);
        void validate(LILSelector value);
        void validate(LILCombinator value);
        void validate(LILFilter value);
        void validate(LILFlag value);
        void validate(LILFunctionDecl value);
        void validate(LILFunctionCall value);
        void validate(LILFlowControl value);
        void validate(LILInstruction value);
        inline void validateChildren(const std::vector<std::shared_ptr<LILNode>> & children);
        
        void setDebug(bool value);
    private:
        bool _debug;
    };
}

#endif
