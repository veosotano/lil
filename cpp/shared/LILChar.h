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
 *	  This file implements a character in LIL
 *
 ********************************************************************/

#ifndef LILCHAR_H
#define LILCHAR_H

namespace LIL
{
	class LILCharPrivate;

	class LILChar
	{
	public:
		LILChar();
		LILChar(const LILChar & other);
		LILChar(LILUnitI64 value);
		virtual ~LILChar();

		LILChar & operator=(const LILChar &other);
		LILChar & operator=(const char * other);

		LILUnitI64 data() const;
		bool isSpace() const;
		bool isDigit() const;

	private:
		LILCharPrivate * d;
	};

	inline bool operator<(const LILChar & c1, const LILChar & c2)
	{ return c1.data() < c2.data(); }

	inline bool operator<=(const LILChar & c1, const LILChar & c2)
	{ return c1.data() <= c2.data(); }

	inline bool operator>(const LILChar & c1, const LILChar & c2)
	{ return c1.data() > c2.data(); }

	inline bool operator>=(const LILChar & c1, const LILChar & c2)
	{ return c1.data() >= c2.data(); }

	inline bool operator==(const LILChar & c1, const LILChar & c2)
	{ return c1.data() == c2.data(); }

	inline bool operator==(const LILChar & c1, const char c2)
	{ return c1.data() == c2; }

	inline bool operator==(const char c1, const LILChar & c2)
	{ return c1 == c2.data(); }

	inline bool operator!=(const LILChar & c1, const LILChar & c2)
	{ return ! operator==(c1, c2); }

	inline bool operator!=(const char c1, const LILChar & c2)
	{ return ! operator==(c1, c2); }
}


#endif
