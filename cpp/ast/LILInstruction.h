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
 *	  This file is a instruction node
 *
 ********************************************************************/


#ifndef LILINSTRUCTION_H
#define LILINSTRUCTION_H


#include "LILTypedNode.h"

namespace LIL
{
	class LILInstruction : public LILTypedNode
	{
	public:
		static LILString instructionTypeToString(InstructionType instrType);
		LILInstruction(NodeType nodeTy = NodeTypeInstruction);
		LILInstruction(const LILInstruction &other);
		std::shared_ptr<LILInstruction> clone() const;
		virtual ~LILInstruction();
		void receiveNodeData(const LILString &data) override;

		void setInstructionType(InstructionType value);
		InstructionType getInstructionType() const override;
		bool isA(InstructionType otherType) const override;
		void setName(LILString name);
		LILString getName() const;
		void setIsColorInstruction(bool value);
		bool getIsColorInstruction() const;
		
		void setArgument(std::shared_ptr<LILNode> value);
		std::shared_ptr<LILNode> getArgument() const;
		
		void setVerbose(bool value);
		bool getVerbose() const;

		bool getReceivesBody() const;
		void setReceivesBody(bool value);
		
	protected:
		std::shared_ptr<LILClonable> cloneImpl() const  override;
		
	private:
		LILString _name;
		std::shared_ptr<LILNode> _argument;
		InstructionType _instructionType;
		bool _isColorInstruction;
		bool _verbose;
		bool _receivesBody;
	};
}

#endif
