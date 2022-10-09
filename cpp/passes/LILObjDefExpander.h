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
 *	  This file converts long value paths in object definitions
 *	  into nested objdefs
 *
 ********************************************************************/

#ifndef LILOBJDEFEXPANDER_H
#define LILOBJDEFEXPANDER_H

#include "LILVisitor.h"

namespace LIL
{
	class LILValuePath;
	class LILObjectDefinition;
	
	class LILObjDefExpander : public LILVisitor
	{
	public:
		LILObjDefExpander();
		virtual ~LILObjDefExpander();
		void initializeVisit() override;
		void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
		void process(LILNode * node);
	};
}

#endif /* LILOBJDEFEXPANDER_H */
