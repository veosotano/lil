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
 *	  This file prints nodes as text in a hierarchical tree
 *
 ********************************************************************/

#ifndef LILDOCUMENTATIONWRITER_H
#define LILDOCUMENTATIONWRITER_H

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
	class LILDocumentationTmplManager;

	class LILDocumentationWriter : public LILVisitor
	{
	public:
		LILDocumentationWriter();
		virtual ~LILDocumentationWriter();
		void initializeVisit() override;
		void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
		void visit(LILNode * node) override;
		void setValue(std::shared_ptr<LILDocumentation> doc);
		const std::shared_ptr<LILDocumentation> & getValue() const;
		const std::string & getResult() const;
		void setTemplateManager(LILDocumentationTmplManager * mgr);
		std::string writeAliasesTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl);
		std::string writeMemberVarsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl);
		std::string writeVvarsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl);
		std::string writeMemberFnsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl);
		std::string writeFnArgsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILFunctionDecl * fnDecl);
		std::string createBoilerplateClass(LILClassDecl * classDecl, LILRootNode * rootNode);
		std::string createBoilerplateFn(LILFunctionDecl * classDecl, LILRootNode * rootNode);
		std::string writeAliasBoilerplate(LILAliasDecl * alias) const;
		std::string writeVvarBoilerplate(LILVarDecl * vd) const;
		std::string writeMemberVarBoilerplate(LILVarDecl * vd) const;
		std::string writeMemberFnBoilerplate(LILFunctionDecl * fnDecl) const;
		std::string writeFnArgBoilerplate(LILVarDecl * vd) const;

	private:
		std::shared_ptr<LILDocumentation> _value;
		std::string _result;
		LILDocumentationTmplManager * _tmplManager;

		std::string _htmlEncode(const std::string& data) const;
		std::string _typeStringRep(LILType * type) const;
	};
}

#endif
