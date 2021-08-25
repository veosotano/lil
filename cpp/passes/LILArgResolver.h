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

#ifndef LILARGRESOLVER_H
#define LILARGRESOLVER_H

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
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILRule.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILSnippetInstruction.h"
#include "LILStringFunction.h"
#include "LILType.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"

namespace LIL
{
    class LILRootNode;
    class LILArgResolver : public LILVisitor
    {
    public:
        LILArgResolver();
        virtual ~LILArgResolver();
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        bool processArgInstr(std::shared_ptr<LILNode> node);

        bool hasCustomArg(const LILString & name) const;
        std::shared_ptr<LILNode> getCustomArg(const LILString & name);
        void setCustomArgs(std::vector<LILString> & args);

    private:
        std::map<LILString, std::shared_ptr<LILNode>> _customArgs;
        std::vector<std::vector<std::shared_ptr<LILNode>>> _nodeBuffer;

        bool _processArgInstr(std::shared_ptr<LILExpression> value);
        bool _processArgInstr(std::shared_ptr<LILUnaryExpression> value);
        bool _processArgInstr(std::shared_ptr<LILStringFunction> value);
        bool _processArgInstr(std::shared_ptr<LILType> value);
        bool _processArgInstr(std::shared_ptr<LILVarDecl> value);
        bool _processArgInstr(std::shared_ptr<LILClassDecl> value);
        bool _processArgInstr(std::shared_ptr<LILObjectDefinition> value);
        bool _processArgInstr(std::shared_ptr<LILAssignment> value);
        bool _processArgInstr(std::shared_ptr<LILValuePath> value);
        bool _processArgInstr(std::shared_ptr<LILPropertyName> value);
        bool _processArgInstr(std::shared_ptr<LILVarName> value);
        bool _processArgInstr(std::shared_ptr<LILRule> value);
        bool _processArgInstr(std::shared_ptr<LILSelectorChain> value);
        bool _processArgInstr(std::shared_ptr<LILSimpleSelector> value);
        bool _processArgInstr(std::shared_ptr<LILFunctionDecl> value);
        bool _processArgInstr(std::shared_ptr<LILFunctionCall> value);
        bool _processArgInstr(std::shared_ptr<LILFlowControl> value);
        bool _processArgInstr(std::shared_ptr<LILFlowControlCall> value);
        bool _processArgInstr(std::shared_ptr<LILInstruction> value);
        bool _processArgInstrIfInstr(std::shared_ptr<LILIfInstruction> value);
        bool _processArgInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value);
        bool _processArgInstr(std::shared_ptr<LILValueList> value);
        bool _processArgInstr(std::shared_ptr<LILIndexAccessor> value);
        bool _processArgInstr(std::shared_ptr<LILConversionDecl> value);
    };
}


#endif /* LILARGRESOLVER_H */
