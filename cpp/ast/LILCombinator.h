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
 *	  This file is the selector in a selector chain
 *
 ********************************************************************/

#ifndef LILCOMBINATOR_H
#define LILCOMBINATOR_H

#include "LILNode.h"

namespace LIL
{
	class LILCombinator : public LILNode
	{
	public:
		LILCombinator();
		LILCombinator(const LILCombinator &other);
		std::shared_ptr<LILCombinator> clone() const;
		virtual ~LILCombinator();
		
		virtual void receiveNodeData(const LILString & data);

		bool equalTo(std::shared_ptr<LILNode> otherNode);
		
		virtual CombinatorType getCombinatorType() const;
		virtual void setCombinatorType(CombinatorType newType);
		
	protected:
		virtual std::shared_ptr<LILClonable> cloneImpl() const;
		
	private:
		CombinatorType _combinatorType;
	};
}

#endif
