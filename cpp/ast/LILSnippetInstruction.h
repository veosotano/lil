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
 *	  This file implements the #snippet instructions
 *
 ********************************************************************/

#ifndef LILSNIPPETINSTRUCTION_H
#define LILSNIPPETINSTRUCTION_H


#include "LILInstruction.h"

namespace LIL
{
	class LILSnippetInstruction : public LILInstruction
	{
	public:
		LILSnippetInstruction();
		LILSnippetInstruction(const LILSnippetInstruction &other);
		std::shared_ptr<LILSnippetInstruction> clone() const;
		virtual ~LILSnippetInstruction();
		void receiveNodeData(const LILString &data) override;
		
		void add(std::shared_ptr<LILNode> node);
		const std::vector<std::shared_ptr<LILNode>> & getBody() const;
		void setBody(const std::vector<std::shared_ptr<LILNode>> && newBody);
		
	protected:
		std::shared_ptr<LILClonable> cloneImpl() const  override;
		
	private:
		std::vector<std::shared_ptr<LILNode>> _body;
	};
}

#endif
