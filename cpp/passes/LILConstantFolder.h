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
 *	  This file tries to pre-bake strings from string functions
 *
 ********************************************************************/

#ifndef LILCONSTANTFOLDER_H
#define LILCONSTANTFOLDER_H

#include "LILVisitor.h"

namespace LIL
{
	class LILConstantFolder : public LILVisitor
	{
	public:
		
		LILConstantFolder();
		virtual ~LILConstantFolder();
		
		void initializeVisit() override;
		void visit(LILNode * node) override;
		void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
		
		inline void processChildren(std::shared_ptr<LILNode> parent, const std::vector<std::shared_ptr<LILNode> > &nodes);
		void process(std::shared_ptr<LILNode> node);
		void addReplacementNode(std::shared_ptr<LILNode> node);

	private:
		std::vector<std::vector<std::shared_ptr<LILNode>>> _nodeBuffer;
	};
}

#endif
