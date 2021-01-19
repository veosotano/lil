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

#ifndef LILTYPERESOLVER_H
#define LILTYPERESOLVER_H

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
#include "LILFunctionType.h"
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
    class LILTypeResolver : public LILVisitor
    {
    public:
        
        LILTypeResolver();
        virtual ~LILTypeResolver();
        
        void initializeVisit() override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void process(std::shared_ptr<LILNode> node);
        std::shared_ptr<LILType> _process(std::shared_ptr<LILType> value);

        std::shared_ptr<LILClassDecl> getClassContext() const;
        void enterClassContext(std::shared_ptr<LILClassDecl> value);
        void exitClassContext();

    private:
        std::vector<std::shared_ptr<LILClassDecl>> _classContext;
    };
}

#endif
