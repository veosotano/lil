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
 *	  This file adds intermediate parts of paths that come from
 *	  expanded fields of classes
 *
 ********************************************************************/

#ifndef LILPATHEXPANDER_H
#define LILPATHEXPANDER_H

#include "LILVisitor.h"

namespace LIL
{
	class LILValuePath;
	class LILObjectDefinition;

	class LILPathExpander : public LILVisitor
	{
	public:
		LILPathExpander();
		virtual ~LILPathExpander();
		void initializeVisit() override;
		void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
		void process(LILNode * node);
		
	private:
		void _process(LILValuePath * vp);
		std::shared_ptr<LILNode> _addExpandedFields(std::deque<std::shared_ptr<LILNode>> &newNodes, std::shared_ptr<LILClassDecl> classDecl, const LILString & pnName, bool & hasChanges, bool isMethod = false);
		void _process(LILObjectDefinition * objdef);
		void _process(LILRule * rule);
		void _processRuleInner(LILRule * rule);
	};
}

#endif /* LILPATHEXPANDER_H */
