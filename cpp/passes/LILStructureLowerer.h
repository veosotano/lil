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
 *      This file disambiguates nodes in the AST with multiple types
 *
 ********************************************************************/

#ifndef LILSTRUCTURELOWERER_H
#define LILSTRUCTURELOWERER_H

#include "LILVisitor.h"
#include "LILRootNode.h"

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
#include "LILFunctionType.h"
#include "LILInstruction.h"
#include "LILNullLiteral.h"
#include "LILNumberLiteral.h"
#include "LILMultipleType.h"
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
    class LILStructureLowerer : public LILVisitor
    {
    public:
        LILStructureLowerer();
        virtual ~LILStructureLowerer();
        void initializeVisit() override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void visit(LILNode * node) override;

        void process(std::shared_ptr<LILNode> node);
        void _process(std::shared_ptr<LILBoolLiteral> value);
        void _process(std::shared_ptr<LILNumberLiteral> value);
        void _process(std::shared_ptr<LILPercentageLiteral> value);
        void _process(std::shared_ptr<LILExpression> value);
        void _process(std::shared_ptr<LILStringLiteral> value);
        void _process(std::shared_ptr<LILStringFunction> value);
        void _process(std::shared_ptr<LILNullLiteral> value);
        void _process(std::shared_ptr<LILType> value);
        void _process(std::shared_ptr<LILVarDecl> value);
        void _process(std::shared_ptr<LILClassDecl> value);
        void _process(std::shared_ptr<LILObjectDefinition> value);
        void _process(std::shared_ptr<LILAssignment> value);
        void _process(std::shared_ptr<LILValuePath> value);
        void _process(std::shared_ptr<LILPropertyName> value);
        void _process(std::shared_ptr<LILVarName> value);
        void _process(std::shared_ptr<LILRule> value);
        void _process(std::shared_ptr<LILSimpleSelector> value);
        void _process(std::shared_ptr<LILSelectorChain> value);
        void _process(std::shared_ptr<LILSelector> value);
        void _process(std::shared_ptr<LILCombinator> value);
        void _process(std::shared_ptr<LILFilter> value);
        void _process(std::shared_ptr<LILFlag> value);
        void _process(std::shared_ptr<LILFunctionDecl> value);
        void _process(std::shared_ptr<LILFunctionCall> value);
        void _process(std::shared_ptr<LILFlowControl> value);
        void _process(std::shared_ptr<LILFlowControlCall> value);
        void _process(std::shared_ptr<LILInstruction> value);

        void processChildren(const std::vector<std::shared_ptr<LILNode> > &children);

    private:
        std::vector<std::vector<std::shared_ptr<LILNode>>> _nodeBuffer;

        std::vector<std::shared_ptr<LILNode>> reduceIfIsBlocks(std::shared_ptr<LILNode> node, LILString argName, LILString tyName);
        void replaceNodeWithContents(std::shared_ptr<LILNode> node, std::vector<std::shared_ptr<LILNode>> contents);
    };
}

#endif
