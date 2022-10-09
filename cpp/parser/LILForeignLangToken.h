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
 *	  This kind of token holds code written in another language
 *
 ********************************************************************/

#ifndef LILFOREIGNLANGTOKEN_H
#define LILFOREIGNLANGTOKEN_H

#include "LILToken.h"

namespace LIL
{
	class LILNode;
	
	class LILForeignLangToken : public LILToken
	{
	public:
		LILForeignLangToken(TokenType type, size_t line, size_t column, size_t index);
		virtual ~LILForeignLangToken();
		void setValue(LILString newValue);
		
		bool equals(TokenType otherType, LILString otherValue);
		LILString toString();

		void setLanguage(LILString value);
		const LILString & getLanguage() const;

		void setContent(LILString value);
		const LILString & getContent() const;
		
		
	private:
		LILString _language;
		LILString _content;
	};
}

#endif
