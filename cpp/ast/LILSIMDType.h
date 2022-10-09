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
 *	  This file is used to represent "vector types"
 *
 ********************************************************************/

#ifndef LILSIMDTYPE_H
#define LILSIMDTYPE_H

#include "LILType.h"

namespace LIL
{
	class LILSIMDType : public LILType
	{
	public:
		LILSIMDType();
		LILSIMDType(const LILSIMDType &other);
		std::shared_ptr<LILSIMDType> clone() const;
		virtual ~LILSIMDType();
		bool equalTo(std::shared_ptr<LILNode> otherNode) override;
		virtual void receiveNodeData(const LILString & data) override;

		void setWidth(unsigned int value);
		unsigned int getWidth() const;

		void setType(std::shared_ptr<LILType> type);
		std::shared_ptr<LILType> getType() const override;

	protected:
		virtual std::shared_ptr<LILClonable> cloneImpl() const override;
		
	private:
		std::shared_ptr<LILType> _type;
		unsigned int _width;
	};
}

#endif
