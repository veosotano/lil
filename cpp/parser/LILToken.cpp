;/********************************************************************
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


#include "../shared/LILShared.h"
#include "LILToken.h"

using namespace LIL;

LILToken::LILToken(TokenType type, size_t line, size_t column, size_t index)
{
    this->type = type;
    this->line = line;
    this->column = column;
    this->index = index;
}

LILToken::LILToken(TokenType type, LILString value, size_t line, size_t column, size_t index)
{
    this->type = type;
    this->line = line;
    this->column = column;
    this->index = index;
    this->_value = value;
    if (this->isNumeric())
    {
        this->_number = value.toDouble();
    }
}
LILToken::LILToken(TokenType type, LILUnitF64 value, size_t line, size_t column, size_t index)
{
    this->type = type;
    this->line = line;
    this->column = column;
    this->index = index;
    this->_number = value;
}

LILToken::~LILToken()
{
}

bool LILToken::isA(TokenType otherType) const
{
    return otherType == this->type;
}

TokenType LILToken::getType() const
{
    return this->type;
}

LILString LILToken::tokenStringRepresentation(TokenType type)
{
    static std::map<TokenType, LILString> types;
    if (types.empty())
    {
        types[TokenTypeNone] = "TokenTypeNone";
        types[TokenTypeIdentifier] = "TokenTypeIdentifier";
        types[TokenTypeNumberInt] = "TokenTypeNumberInt";
        types[TokenTypeNumberFP] = "TokenTypeNumberFP";
        types[TokenTypePercentageNumberInt] = "TokenTypePercentageNumberInt";
        types[TokenTypePercentageNumberFP] = "TokenTypePercentageNumberFP";
        types[TokenTypeHexNumber] = "TokenTypeHexNumber";
        types[TokenTypeDoubleQuoteString] = "TokenTypeDoubleQuoteString";
        types[TokenTypeSingleQuoteString] = "TokenTypeSingleQuoteString";
        types[TokenTypeWhitespace] = "TokenTypeWhitespace";
        types[TokenTypeInstructionSign] = "TokenTypeInstructionSign";
        types[TokenTypeObjectSign] = "TokenTypeObjectSign";
        types[TokenTypeBlockOpen] = "TokenTypeBlockOpen";
        types[TokenTypeBlockClose] = "TokenTypeBlockClose";
        types[TokenTypeComma] = "TokenTypeComma";
        types[TokenTypeColon] = "TokenTypeColon";
        types[TokenTypeSemicolon] = "TokenTypeSemicolon";
        types[TokenTypeParenthesisOpen] = "TokenTypeParenthesisOpen";
        types[TokenTypeParenthesisClose] = "TokenTypeParenthesisClose";
        types[TokenTypeLineComment] = "TokenTypeLineComment";
        types[TokenTypeBlockComment] = "TokenTypeBlockComment";
        types[TokenTypeNegator] = "TokenTypeNegator";
        types[TokenTypeAmpersand] = "TokenTypeAmpersand";
        types[TokenTypeDot] = "TokenTypeDot";
        types[TokenTypeDoubleDot] = "TokenTypeDoubleDot";
        types[TokenTypeEqualSign] = "TokenTypeEqualSign";
        types[TokenTypeBiggerComparator] = "TokenTypeBiggerComparator";
        types[TokenTypeBiggerOrEqualComparator] = "TokenTypeBiggerOrEqualComparator";
        types[TokenTypeSmallerComparator] = "TokenTypeSmallerComparator";
        types[TokenTypeSmallerOrEqualComparator] = "TokenTypeSmallerOrEqualComparator";
        types[TokenTypeSquareBracketOpen] = "TokenTypeSquareBracketOpen";
        types[TokenTypeSquareBracketClose] = "TokenTypeSquareBracketClose";
        types[TokenTypeDocumentation] = "TokenTypeDocumentation";
    }
    
    if (types.count(type)){
        return types[type];
    }
    return "";
}

LILString LILToken::getString()
{
    return this->_value;
}

void LILToken::setString(LILString newValue)
{
    this->_value = newValue;
}

LILUnitF64 LILToken::getNumber()
{
    return this->_number;
}

bool LILToken::equals(TokenType otherType, LILString otherValue)
{
    return otherType == this->type && otherValue == this->_value;
}

bool LILToken::equals(TokenType otherType, LILUnitF64 otherValue)
{
    return otherType == this->type && otherValue == this->_number;
}

LILString LILToken::toString()
{
    LILString tokenstr = this->tokenStringRepresentation(this->type);
    if (this->isNumeric())
    {
        return LILString::format("LILToken of type: %s and value: %f", tokenstr.chardata(), this->_number);
    }
    else
    {
        return "LILToken of type: " + tokenstr + " and value: " + this->_value;
    }
}

bool LILToken::isNumeric()
{
    return this->type == TokenTypeNumberInt || this->type == TokenTypeNumberFP || this->type == TokenTypePercentageNumberInt || this->type == TokenTypePercentageNumberFP;
}
