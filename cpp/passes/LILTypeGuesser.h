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
 *      This file analyzes the code to determine types automatically
 *
 ********************************************************************/

#ifndef LILTYPEGUESSER_H
#define LILTYPEGUESSER_H

#include "LILVisitor.h"
#include "LILErrorMessage.h"
#include "LILNode.h"

#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCombinator.h"
#include "LILEnum.h"
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILIfInstruction.h"
#include "LILIndexAccessor.h"
#include "LILInstruction.h"
#include "LILNullLiteral.h"
#include "LILNumberLiteral.h"
#include "LILMultipleType.h"
#include "LILObjectDefinition.h"
#include "LILPercentageLiteral.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILRootNode.h"
#include "LILRule.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILType.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

namespace LIL
{
    class LILTypeGuesser : public LILVisitor
    {
    public:
        
        LILTypeGuesser();
        virtual ~LILTypeGuesser();

        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void nullsToNullables(std::shared_ptr<LILNode> node);
        void connectCallsWithDecls(std::shared_ptr<LILNode> node);
        void propagateStrongTypes(std::shared_ptr<LILNode> node);
        void _propagateStrongType(std::shared_ptr<LILNode> node, std::shared_ptr<LILType> ty);
        
        void searchForTypesFromInitVal(std::shared_ptr<LILNode> node);
        void searchForTypesFromAssignments(std::shared_ptr<LILNode> node);
        void searchForTypesForArguments(std::shared_ptr<LILNode> node);
        
        inline void processChildren(const std::vector<std::shared_ptr<LILNode>> & children);
        void process(LILNode * node);
        void _process(LILBoolLiteral * value);
        void _process(LILNumberLiteral * value);
        void _process(LILPercentageLiteral * value);
        void _processCast(LILExpression * value);
        void _process(LILExpression * value);
        void _process(LILUnaryExpression * value);
        void _process(LILStringLiteral * value);
        void _process(LILStringFunction * value);
        void _process(LILNullLiteral * value);
        void _process(LILVarDecl * value);
        void _process(LILEnum * value);
        void _process(LILClassDecl * value);
        void _process(LILObjectDefinition * value);
        void _process(LILAssignment * value);
        void _process(LILValuePath * value);
        void _process(LILPropertyName * value);
        void _process(LILVarName * value);
        void _process(LILRule * value);
        void _processRuleInner(LILRule * value);
        void _process(LILSimpleSelector * value);
        void _process(LILSelectorChain * value);
        void _process(LILSelector * value);
        void _process(LILCombinator * value);
        void _process(LILFilter * value);
        void _process(LILFlag * value);
        void _process(LILFunctionDecl * value);
        void _process(LILFunctionCall * value);
        void _process(LILFlowControl * value);
        void _process(LILFlowControlCall * value);
        void _process(LILInstruction * value);
        void _process(LILIfInstruction * value);
        void _process(LILValueList * value);
        void _process(LILIndexAccessor * value);

        std::shared_ptr<LILType> recursiveFindTypeFromAncestors(LILNode * value) const;
        std::shared_ptr<LILFunctionType> findFnTypeForFunctionCall(LILFunctionCall * fc) const;
        void setTypeOnAncestorIfNeeded(LILNode * value, std::shared_ptr<LILType> ty);
        std::shared_ptr<LILFunctionDecl> recursiveFindFunctionDecl(LILNode * node) const;
        std::shared_ptr<LILType> getNodeType(LILNode * node) const;
        std::shared_ptr<LILType> getExpType(LILExpression * exp) const;
        std::shared_ptr<LILType> getExpElementType(LILExpression * exp) const;
        std::shared_ptr<LILType> getFnType(LILFunctionDecl * fd) const;
        std::shared_ptr<LILType> getFnReturnType(const std::vector<std::shared_ptr<LILNode>> & nodes) const;
        std::shared_ptr<LILType> findTypeForArg(LILVarDecl * vd, LILFunctionDecl * fd, size_t argCount) const;
        void recursiveFindReturnTypes(std::vector<std::shared_ptr<LILType>> & returnTypes, std::shared_ptr<LILNode> eval) const;
        void addTypeToReturnTypes(std::vector<std::shared_ptr<LILType>> & returnTypes, std::shared_ptr<LILType> ty) const;
        std::shared_ptr<LILType> findReturnTypeForFunctionCall(LILFunctionCall * fc) const;
        std::shared_ptr<LILType> findTypeForVarName(LILVarName * name) const;
        std::shared_ptr<LILType> findTypeForValuePath(LILValuePath * vp) const;
        std::shared_ptr<LILType> findTypeForPropertyName(LILPropertyName * name) const;
        std::shared_ptr<LILType> findTypeForSelectorChain(LILSelectorChain * selCh) const;
        std::shared_ptr<LILType> findTypeFromAssignments(std::vector<std::shared_ptr<LILNode>> nodes, LILVarDecl * vd) const;
         std::shared_ptr<LILType> findTypeFromFunctionCalls(std::vector<std::shared_ptr<LILNode>> nodes, LILVarDecl * vd) const;
        std::shared_ptr<LILType> findTypeFromExpressions(std::vector<std::shared_ptr<LILNode>> nodes, LILVarDecl * vd) const;
        std::shared_ptr<LILType> findTypeFromCallers(const std::vector<std::shared_ptr<LILNode>> & nodes, LILVarDecl * vd, size_t argCount) const;
        std::shared_ptr<LILType> nullsToNullableTypes(std::shared_ptr<LILType> ty) const;
        std::shared_ptr<LILType> findTypeForEnum(LILEnum * value) const;
    };
}

#endif
