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
 *      This kind of token holds code written in another language
 *
 ********************************************************************/


#include "../shared/LILShared.h"
#include "LILForeignLangToken.h"

using namespace LIL;

LILForeignLangToken::LILForeignLangToken(TokenType type, size_t line, size_t column, size_t index)
: LILToken(type, line, column, index)
{
    this->type = type;
}

LILForeignLangToken::~LILForeignLangToken()
{
}

bool LILForeignLangToken::equals(TokenType otherType, LILString otherValue)
{
    return otherType == this->type && otherValue == this->_value;
}

LILString LILForeignLangToken::toString()
{
    LILString tokenstr = this->tokenStringRepresentation(this->type);
    return "LILForeignLangToken of type: " + tokenstr + " and value: " + this->_value;
}

void LILForeignLangToken::setLanguage(LILString value)
{
    this->_language = value;
}

const LILString & LILForeignLangToken::getLanguage() const
{
    return this->_language;
}

void LILForeignLangToken::setContent(LILString value)
{
    this->_content = value;
}

const LILString & LILForeignLangToken::getContent() const
{
    return this->_content;
}
