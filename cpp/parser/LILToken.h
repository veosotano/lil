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
 *      This file is the smallest piece in the parsing system
 *
 ********************************************************************/

#ifndef LILTOKEN_H
#define LILTOKEN_H

#include "../shared/LILTypeEnums.h"
#include "../shared/LILBasicValues.h"

namespace LIL
{
    class LILToken
    {
    public:
        static LILString tokenStringRepresentation(TokenType type);

        LILToken(TokenType type, size_t line, size_t column, size_t index);
        LILToken(TokenType type, LILString value, size_t line, size_t column, size_t index);
        LILToken(TokenType type, LILUnitF64 value, size_t line, size_t column, size_t index);
        virtual ~LILToken();
        bool isA(TokenType otherType) const;
        TokenType getType() const;
        virtual LILString toString();

        size_t line;
        size_t column;
        size_t index;

        LILString getString();
        void setString(LILString newValue);

        LILUnitF64 getNumber();
        bool equals(TokenType otherType, LILString otherValue);
        bool equals(TokenType otherType, LILUnitF64 otherValue);
        bool isNumeric();

    protected:
        TokenType type;
        LILString _value;
        LILUnitF64 _number;
    };
}

#endif
