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
 *	  This file traverses the ast in search for resource fields
 *
 ********************************************************************/

#ifndef LILRESOURCEGATHERER_H
#define LILRESOURCEGATHERER_H

#include "../shared/LILVisitor.h"

#endif /* LILRESOURCEGATHERER_H */

namespace LIL {
	class LILObjectDefinition;
	class LILRootNode;
	class LILVarDecl;

	class LILResourceGatherer : public LILVisitor
	{
	public:
		LILResourceGatherer();
		virtual ~LILResourceGatherer();
		void initializeVisit() override;
		void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
		const std::vector<LILString> gatherResources() const;
		std::shared_ptr<LILVarDecl> recursiveGetResourceVd(LILObjectDefinition * objDef) const;
	private:
		const std::vector<LILString> _gatherResources(LILRule * rule) const;
	};

}
