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
 *      This file handles the preprocessing of the AST
 *
 ********************************************************************/

#ifndef LILCONFIGGETTER_H
#define LILCONFIGGETTER_H

#include "LILVisitor.h"
#include "LILAssignment.h"
#include "LILClassDecl.h"
#include "LILConversionDecl.h"
#include "LILExpression.h"
#include "LILDocumentation.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILIfInstruction.h"
#include "LILIndexAccessor.h"
#include "LILInstruction.h"
#include "LILMultipleType.h"
#include "LILObjectDefinition.h"
#include "LILObjectType.h"
#include "LILPropertyName.h"
#include "LILPointerType.h"
#include "LILValuePath.h"
#include "LILRule.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILSnippetInstruction.h"
#include "LILStaticArrayType.h"
#include "LILStringFunction.h"
#include "LILType.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"

namespace LIL
{
    class LILRootNode;
    class LILConfiguration;
    class LILConfigGetter : public LILVisitor
    {
    public:
        LILConfigGetter();
        virtual ~LILConfigGetter();
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        bool processGetConfigInstr(std::shared_ptr<LILNode> node);

        void setConfiguration(LILConfiguration * value);
        
    private:
        std::vector<std::vector<std::shared_ptr<LILNode>>> _nodeBuffer;
        LILConfiguration * _config;

        bool _processGetConfigInstr(std::shared_ptr<LILExpression> value);
        bool _processGetConfigInstr(std::shared_ptr<LILUnaryExpression> value);
        bool _processGetConfigInstr(std::shared_ptr<LILStringFunction> value);
        bool _processGetConfigInstr(std::shared_ptr<LILVarDecl> value);
        bool _processGetConfigInstr(std::shared_ptr<LILClassDecl> value);
        bool _processGetConfigInstr(std::shared_ptr<LILObjectDefinition> value);
        bool _processGetConfigInstr(std::shared_ptr<LILAssignment> value);
        bool _processGetConfigInstr(std::shared_ptr<LILValuePath> value);
        bool _processGetConfigInstr(std::shared_ptr<LILPropertyName> value);
        bool _processGetConfigInstr(std::shared_ptr<LILVarName> value);
        bool _processGetConfigInstr(std::shared_ptr<LILRule> value);
        bool _processGetConfigInstr(std::shared_ptr<LILSelectorChain> value);
        bool _processGetConfigInstr(std::shared_ptr<LILSimpleSelector> value);
        bool _processGetConfigInstr(std::shared_ptr<LILFunctionDecl> value);
        bool _processGetConfigInstr(std::shared_ptr<LILFunctionCall> value);
        bool _processGetConfigInstr(std::shared_ptr<LILFlowControl> value);
        bool _processGetConfigInstr(std::shared_ptr<LILFlowControlCall> value);
        bool _processGetConfigInstr(std::shared_ptr<LILInstruction> value);
        bool _processGetConfigInstrIfInstr(std::shared_ptr<LILIfInstruction> value);
        bool _processGetConfigInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value);
        bool _processGetConfigInstr(std::shared_ptr<LILValueList> value);
        bool _processGetConfigInstr(std::shared_ptr<LILIndexAccessor> value);
        bool _processGetConfigInstr(std::shared_ptr<LILConversionDecl> value);
        bool _processGetConfigInstr(std::shared_ptr<LILType> value);
        bool _processGetConfigInstr(std::shared_ptr<LILStaticArrayType> value);
        bool _processGetConfigInstr(std::shared_ptr<LILMultipleType> value);
        bool _processGetConfigInstr(std::shared_ptr<LILFunctionType> value);
        bool _processGetConfigInstr(std::shared_ptr<LILPointerType> value);
    };
}


#endif /* LILCONFIGGETTER_H */
