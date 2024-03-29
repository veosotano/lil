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
 *	  This file represents a string written in the source code
 *
 ********************************************************************/

#ifndef LILSTRINGLITERAL_H
#define LILSTRINGLITERAL_H

#include "LILNode.h"

namespace LIL
{
	class LILStringLiteral : public LILNode
	{
	public:
		LILStringLiteral();
		LILStringLiteral(const LILStringLiteral & other);
		std::shared_ptr<LILStringLiteral> clone() const;
		virtual ~LILStringLiteral();
		void receiveNodeData(const LIL::LILString &data) override;
		void setValue(LILString newValue);
		LILString getValue();
		bool equalTo(std::shared_ptr<LILNode> otherNode) override;
		void setIsCString(bool value);
		bool getIsCString() const;
		std::shared_ptr<LILType> getType() const override;

	private:
		LILString _value;
		bool _isCStr;
		
	private:
		virtual std::shared_ptr<LILClonable> cloneImpl() const override;
	};
}

#endif
