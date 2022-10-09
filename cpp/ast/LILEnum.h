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
 *	  This file represents an enum
 *
 ********************************************************************/

#ifndef LILENUM_H
#define LILENUM_H

#include "LILTypedNode.h"

namespace LIL
{
	class LILEnum : public LILTypedNode
	{
	public:
		LILEnum();
		LILEnum(const LILEnum &other);
		std::shared_ptr<LILEnum> clone() const;
		virtual ~LILEnum();
		
		const std::vector<std::shared_ptr<LILNode>> & getValues() const;
		void setValues(const std::vector<std::shared_ptr<LILNode>> & newValues);
		void addValue(std::shared_ptr<LILNode> value);
		void setName(LILString newName);
		std::shared_ptr<LILNode> getValueNamed(const LILString & name) const;
		const LILString getName() const;
		void receiveNodeData(const LILString & data) override;
		
	protected:
		std::shared_ptr<LILClonable> cloneImpl() const override;
		
	private:
		LILString _name;
		bool _preventEmitCallToIVar;
	};
}

#endif
