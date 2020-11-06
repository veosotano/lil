/********************************************************************
 *
 *      LIL Is a Language
 *
 *      AUTHORS: Miro Keller
 *
 *      COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *      LICENSE: see LICENSE file
 *
 *      This file implements a wrapper for basic strings
 *
 ********************************************************************/

#ifndef LILSTRING_H
#define LILSTRING_H

#include "LILTypeEnums.h"
#include "LILBasicValues.h"
#include "LILChar.h"
#include <string>
#include <vector>

namespace LIL
{
    class LILStringPrivate;

    class LILString
    {
    public:
        static LILString number(LILUnitI8 num);
        static LILString number(LILUnitI16 num);
        static LILString number(LILUnitI32 num);
        static LILString number(LILUnitI64 num);
        static LILString number(LILUnitF32 num);
        static LILString number(LILUnitF64 num);
        static LILString format(const LILString format, ...);
        static LILString join(std::vector<LILString> strings, const char * glue);

        LILString();
        LILString(const LILString & other);
        virtual ~LILString();
        LILString(const char * cStr);
        LILString(std::string str);
        LILString(LILChar c);
        const std::string & data() const;
        const char * chardata() const;

        std::string::iterator begin() const;
        std::string::iterator end() const;

        LILString & operator=(const LILString &other);
        inline LILString &operator+=(const LILString &s) { return append(s); }
        inline LILString &operator+=(const LILChar &c) { return append(c.data()); }
        void clear();
        LILString & append(const LILString &other);
        LILString & append(const LILChar &c);
        void truncate(size_t pos);

        const bool operator==(const LILString &other) const;
        const bool operator==(const char * other) const;
        friend inline bool operator!=(const LILString &s1, const LILString &s2) { return !(s1 == s2); }
        float toFloat() const;
        double toDouble() const;
        short int toShortInt() const;
        char toChar() const;
        int toInt() const;
        long toLong() const;
        long long toLongLong() const;
        unsigned int toHex() const;
        LILString trimmed() const;
        LILString stripQuotes() const;
        LILString replaceEscapes() const;
        LILString toUpperFirstCase() const;
        bool isEmpty() const;
        size_t length() const;
        LILString substr(size_t start, size_t length) const;
        LILString substr(size_t start) const;
        LILString at(size_t pos) const;
        bool contains(const char * needle) const;

    private:
        LILStringPrivate * d;
        static unsigned int _count_decimals(LILUnitF64 num);
    };

    inline const LILString operator+(const LILString & s1, const LILString & s2)
    { LILString t(s1); t += s2; return t; }

    inline const LILString operator+(const LILString & s1, const char * s2)
    { LILString t(s1); t += s2; return t; }

    inline const bool operator<(const LILString & s1, const LILString & s2)
    { return s1.data() < s2.data(); }
}


#endif
