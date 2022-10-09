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
 *	  This file encapsulates the name of a property
 *
 ********************************************************************/

#ifndef LILVARNAME_H
#define LILVARNAME_H

#include "LILTypedNode.h"

namespace LIL
{
	class LILVarName : public LILTypedNode
	{
	public:
		LILVarName();
		LILVarName(const LILVarName &other);
		std::shared_ptr<LILVarName> clone() const;
		virtual ~LILVarName();

		void receiveNodeData(const LILString & data) override;

		bool equalTo(std::shared_ptr<LILNode> otherNode) override;

		void setName(LILString newName);
		const LILString getName() const;

	protected:
		std::shared_ptr<LILClonable> cloneImpl() const override;
		
	private:
		LILString _name;
	};
}

#endif
