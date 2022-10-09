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
 *	  This file represents a selector chain
 *
 ********************************************************************/


#ifndef LILSELECTORCHAIN_H
#define LILSELECTORCHAIN_H


#include "LILNode.h"

namespace LIL
{
	class LILSelectorChain : public LILNode
	{
	public:
		LILSelectorChain();
		LILSelectorChain(const LILSelectorChain &other);
		std::shared_ptr<LILSelectorChain> clone() const;
		virtual ~LILSelectorChain();
		
		const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
		void setNodes(std::vector<std::shared_ptr<LILNode>> && nodes);
		const std::shared_ptr<LILNode> & getFirstNode() const;
		const std::shared_ptr<LILNode> & getLastNode() const;

	protected:
		virtual std::shared_ptr<LILClonable> cloneImpl() const;
		
	private:
		
	};
}

#endif
