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
 *	  This file defines the base class visitors
 *
 ********************************************************************/

#ifndef LILVISITOR_H
#define LILVISITOR_H

#include "LILNode.h"

namespace LIL
{
	class LILClassDecl;
	class LILEnum;
	class LILErrorMessage;
	class LILFlowControl;
	class LILValuePath;
	class LILVarName;
	class LILPropertyName;
	class LILRootNode;
	class LILRule;
	class LILValueList;

	class LILVisitor
	{
	public:
		bool inhibitSearchingForIfCastType;

		LILVisitor();
		virtual ~LILVisitor();
		virtual void initializeVisit() = 0;
		virtual void performVisit(std::shared_ptr<LILRootNode> rootNode);
		virtual void visit(LILNode * node);
		bool hasErrors() const;
		
		std::vector<LILErrorMessage> errors;

		void setPrintHeadline(bool value);
		bool getPrintHeadline() const;
		void setVerbose(bool value);
		bool getVerbose() const;
		void setDebug(bool value);
		bool getDebug() const;
		std::shared_ptr<LILNode> findNodeForVarName(LILVarName * name) const;
		std::shared_ptr<LILNode> findNodeForName(LILString name, LILNode * parent) const;
		std::shared_ptr<LILNode> findNodeForValuePath(LILValuePath * vp) const;
		std::shared_ptr<LILNode> findNodeForPropertyName(LILPropertyName * name) const;
		std::shared_ptr<LILNode> recursiveFindNode(std::shared_ptr<LILNode> node) const;

		LILString decorate(LILString ns, LILString className, LILString name, std::shared_ptr<LILType> type) const;
		LILString typeToString(std::shared_ptr<LILType> type) const;
		
		void setRootNode(std::shared_ptr<LILRootNode> value);
		std::shared_ptr<LILRootNode> getRootNode() const;

		std::shared_ptr<LILClassDecl> findClassWithName(const LILString & name) const;
		std::shared_ptr<LILEnum> findEnumWithName(const LILString & name) const;
		std::shared_ptr<LILClassDecl> findAncestorClass(std::shared_ptr<LILNode> node) const;
		std::shared_ptr<LILRule> findAncestorRule(std::shared_ptr<LILNode> node) const;
		std::shared_ptr<LILFlowControl> findAncestorFor(std::shared_ptr<LILNode> node) const;
		std::shared_ptr<LILType> findIfCastType(LILValuePath * vp, size_t & outStartIndex) const;
		std::shared_ptr<LILType> findIfCastTypeVN(LILVarName * vn) const;
		std::shared_ptr<LILNode> findExpandedField(std::shared_ptr<LILClassDecl> classDecl, const LILString & pnName) const;
		std::shared_ptr<LILType> findTypeForValueList(LILValueList * value) const;

	private:
		bool _printHeadline;
		bool _verbose;
		bool _debug;
		std::shared_ptr<LILRootNode> _rootNode;
	};
}

#endif
