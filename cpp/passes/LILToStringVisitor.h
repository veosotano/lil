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
    class LILToStringVisitor : public LILVisitor
    {
    public:
        LILToStringVisitor();
        virtual ~LILToStringVisitor();
        void initializeVisit();
        void visit(LILNode * node);
        void printInfo(LILToStrInfo info, size_t indents, std::vector<size_t> moreItems);
        LILToStrInfo stringify(LILNode * node);
        LILToStrInfo stringify(LILBoolLiteral value);
        LILToStrInfo stringify(LILNumberLiteral value);
        LILToStrInfo stringify(LILPercentageLiteral value);
        LILToStrInfo stringify(LILExpression value);
        LILToStrInfo stringify(LILStringLiteral value);
        LILToStrInfo stringify(LILStringFunction value);
        LILToStrInfo stringify(LILNullLiteral value);
        LILToStrInfo stringify(LILType value);
        LILToStrInfo stringify(LILVarDecl value);
        LILToStrInfo stringify(LILClassDecl value);
        LILToStrInfo stringify(LILObjectDefinition value);
        LILToStrInfo stringify(LILAssignment value);
        LILToStrInfo stringify(LILValuePath value);
        LILToStrInfo stringify(LILPropertyName value);
        LILToStrInfo stringify(LILVarName value);
        LILToStrInfo stringify(LILRule value);
        LILToStrInfo stringify(LILSimpleSelector value);
        LILToStrInfo stringify(LILSelectorChain value);
        LILToStrInfo stringify(LILSelector value);
        LILToStrInfo stringify(LILCombinator value);
        LILToStrInfo stringify(LILFilter value);
        LILToStrInfo stringify(LILFlag value);
        LILToStrInfo stringify(LILFunctionDecl value);
        LILToStrInfo stringify(LILFunctionCall value);
        LILToStrInfo stringify(LILFlowControl value);
        LILToStrInfo stringify(LILInstruction value);
        LILString stringForIndent(size_t indents, std::vector<size_t> moreItems);
        inline void stringifyChildren(const std::vector<std::shared_ptr<LILNode>> & children, LILToStrInfo & info);
    };
}

#endif
