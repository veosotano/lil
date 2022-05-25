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
 *      This file prints nodes as text in a hierarchical tree
 *
 ********************************************************************/

#ifndef LILTOSTRINGVISITOR_H
#define LILTOSTRINGVISITOR_H

#include "LILVisitor.h"
#include "LILNode.h"
#include "LILAliasDecl.h"
#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCombinator.h"
#include "LILConversionDecl.h"
#include "LILDocumentation.h"
#include "LILEnum.h"
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILForeignLang.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILIndexAccessor.h"
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
#include "LILTypeDecl.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

namespace LIL
{
    class LILToStrInfo;
    class LILToStringVisitor : public LILVisitor
    {
    public:
        LILToStringVisitor();
        virtual ~LILToStringVisitor();
        void initializeVisit() override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void visit(LILNode * node) override;
        void printInfo(LILToStrInfo info, size_t indents, std::vector<size_t> moreItems);
        LILToStrInfo stringify(LILNode * node);
        LILToStrInfo _stringify(LILBoolLiteral * value);
        LILToStrInfo _stringify(LILNumberLiteral * value);
        LILToStrInfo _stringify(LILPercentageLiteral * value);
        LILToStrInfo _stringify(LILExpression * value);
        LILToStrInfo _stringify(LILUnaryExpression * value);
        LILToStrInfo _stringify(LILStringLiteral * value);
        LILToStrInfo _stringify(LILStringFunction * value);
        LILToStrInfo _stringify(LILNullLiteral * value);
        LILToStrInfo _stringify(LILType * value);
        LILToStrInfo _stringify(LILVarDecl * value);
        LILToStrInfo _stringify(LILAliasDecl * value);
        LILToStrInfo _stringify(LILTypeDecl * value);
        LILToStrInfo _stringify(LILConversionDecl * value);
        LILToStrInfo _stringify(LILEnum * value);
        LILToStrInfo _stringify(LILClassDecl * value);
        LILToStrInfo _stringify(LILObjectDefinition * value);
        LILToStrInfo _stringify(LILAssignment * value);
        LILToStrInfo _stringify(LILValuePath * value);
        LILToStrInfo _stringify(LILPropertyName * value);
        LILToStrInfo _stringify(LILVarName * value);
        LILToStrInfo _stringify(LILRule * value);
        LILToStrInfo _stringify(LILSimpleSelector * value);
        LILToStrInfo _stringify(LILSelectorChain * value);
        LILToStrInfo _stringify(LILSelector * value);
        LILToStrInfo _stringify(LILCombinator * value);
        LILToStrInfo _stringify(LILFilter * value);
        LILToStrInfo _stringify(LILFlag * value);
        LILToStrInfo _stringify(LILFunctionDecl * value);
        LILToStrInfo _stringify(LILFunctionCall * value);
        LILToStrInfo _stringify(LILFlowControl * value);
        LILToStrInfo _stringify(LILFlowControlCall * value);
        LILToStrInfo _stringify(LILInstruction * value);
        LILToStrInfo _stringify(LILForeignLang * value);
        LILToStrInfo _stringify(LILDocumentation * value);
        LILToStrInfo _stringify(LILValueList * value);
        LILToStrInfo _stringify(LILIndexAccessor * value);
        LILString stringForIndent(size_t indents, std::vector<size_t> moreItems);
        inline void stringifyChildren(const std::vector<std::shared_ptr<LILNode>> & children, LILToStrInfo & info);
    };
}

#endif
