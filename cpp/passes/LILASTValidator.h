/********************************************************************
 *
 *	  LIL Is a Language
 *
 *	  AUTHORS: Miro Keller
 *
 *	  COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *	  LICENSE: see LICENSE file
 *
 *	  This file checks the AST for logical coherence
 *
 ********************************************************************/

#ifndef LILASTVALIDATOR_H
#define LILASTVALIDATOR_H

#include "LILVisitor.h"
#include "LILErrorMessage.h"
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
#include "LILIfInstruction.h"
#include "LILIndexAccessor.h"
#include "LILInstruction.h"
#include "LILNullLiteral.h"
#include "LILNumberLiteral.h"
#include "LILObjectDefinition.h"
#include "LILObjectType.h"
#include "LILPercentageLiteral.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILRule.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILTypeDecl.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"
#include "LILVarName.h"


namespace LIL
{
	class LILASTValidator : public LILVisitor
	{
	public:
		LILASTValidator();
		virtual ~LILASTValidator();
		void initializeVisit() override;
		void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
		void illegalNodeType(LILNode* illegalNode, LILNode * container);

		void validate(const std::shared_ptr<LILNode> & node);
		void _validate(const std::shared_ptr<LILBoolLiteral> & value);
		void _validate(const std::shared_ptr<LILNumberLiteral> & value);
		void _validate(const std::shared_ptr<LILPercentageLiteral> & value);
		void _validate(const std::shared_ptr<LILExpression> & value);
		void _validate(const std::shared_ptr<LILUnaryExpression> & value);
		void _validate(const std::shared_ptr<LILStringLiteral> & value);
		void _validate(const std::shared_ptr<LILStringFunction> & value);
		void _validate(const std::shared_ptr<LILNullLiteral> & value);
		void _validate(const std::shared_ptr<LILType> & value);
		bool _isBuiltInType(const std::shared_ptr<LILType> & ty) const;
		void _validate(const std::shared_ptr<LILVarDecl> & value);
		void _validate(const std::shared_ptr<LILAliasDecl> & value);
		void _validate(const std::shared_ptr<LILTypeDecl> & value);
		void _validate(const std::shared_ptr<LILConversionDecl> & value);
		void _validate(const std::shared_ptr<LILEnum> & value);
		void _validate(const std::shared_ptr<LILClassDecl> & value);
		void _validate(const std::shared_ptr<LILObjectDefinition> & value);
		void _validate(const std::shared_ptr<LILAssignment> & value);
		void _validate(const std::shared_ptr<LILValuePath> & value);
		void _validate(const std::shared_ptr<LILPropertyName> & value);
		void _validate(const std::shared_ptr<LILVarName> & value);
		void _validate(const std::shared_ptr<LILRule> & value);
		void _validate(const std::shared_ptr<LILSimpleSelector> & value);
		void _validate(const std::shared_ptr<LILSelectorChain> & value);
		void _validate(const std::shared_ptr<LILSelector> & value);
		void _validate(const std::shared_ptr<LILCombinator> & value);
		void _validate(const std::shared_ptr<LILFilter> & value);
		void _validate(const std::shared_ptr<LILFlag> & value);
		void _validate(const std::shared_ptr<LILFunctionDecl> & value);
		void _validateFunctionDeclChild(LILFunctionDecl * value, LILNode *node);
		void _validate(const std::shared_ptr<LILFunctionCall> & value);
		void _validate(const std::shared_ptr<LILFlowControl> & value);
		void _validate(const std::shared_ptr<LILFlowControlCall> & value);
		void _validate(const std::shared_ptr<LILInstruction> & value);
		void _validate(const std::shared_ptr<LILIfInstruction> & value);
		void _validate(const std::shared_ptr<LILForeignLang> & value);
		void _validate(const std::shared_ptr<LILDocumentation> & value);
		void _validate(const std::shared_ptr<LILValueList> & value);
		void _validate(const std::shared_ptr<LILIndexAccessor> & value);
		inline void validateChildren(const std::vector<std::shared_ptr<LILNode>> & children);
	};
}

#endif
