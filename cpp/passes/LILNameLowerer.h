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
 *	  This file changes the name of variables and functions to a
 *	  lower level
 *
 ********************************************************************/

#ifndef LILNAMELOWERER_H
#define LILNAMELOWERER_H

#include "LILVisitor.h"
#include "LILNode.h"

#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCombinator.h"
#include "LILDocumentation.h"
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFlowControl.h"
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
#include "LILPropertyName.h"
#include "LILValuePath.h"
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
	class LILNameLowerer : public LILVisitor
	{
	public:
		LILNameLowerer();
		virtual ~LILNameLowerer();
		void initializeVisit();
		virtual void visit(LILNode * node);

		void process(LILNode * node);
		void _process(LILBoolLiteral * value);
		void _process(LILNumberLiteral * value);
		void _process(LILPercentageLiteral * value);
		void _process(LILExpression * value);
		void _process(LILUnaryExpression * value);
		void _process(LILStringLiteral * value);
		void _process(LILStringFunction * value);
		void _process(LILNullLiteral * value);
		void _process(LILType * value);
		void _process(LILVarDecl * value);
		void _process(LILClassDecl * value);
		void _process(LILObjectDefinition * value);
		void _process(LILAssignment * value);
		void _process(LILValuePath * value);
		void _process(LILPropertyName * value);
		void _process(LILVarName * value);
		void _process(LILRule * value);
		void _process(LILSimpleSelector * value);
		void _process(LILSelectorChain * value);
		void _process(LILSelector * value);
		void _process(LILCombinator * value);
		void _process(LILFilter * value);
		void _process(LILFlag * value);
		void _process(LILFunctionDecl * value);
		void _process(LILFunctionCall * value);
		void _process(LILFlowControl * value);
		void _process(LILInstruction * value);
		void _process(LILValueList * value);
		void _process(LILDocumentation * value);
		void _process(LILIndexAccessor * value);
		inline void processChildren(const std::vector<std::shared_ptr<LILNode>> & children);
	};
}

#endif
