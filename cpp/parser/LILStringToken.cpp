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
 *	  This file encapsulates a token that contains a string
 *
 ********************************************************************/


#include "../shared/LILShared.h"
#include "LILStringToken.h"

using namespace LIL;

LILStringToken::LILStringToken(TokenType type, size_t line, size_t column, size_t index)
: LILToken(type, line, column, index)
, _hasArguments(false)
{
	this->type = type;
}

LILStringToken::~LILStringToken()
{
}

bool LILStringToken::equals(TokenType otherType, LILString otherValue)
{
	return otherType == this->type && otherValue == this->_value;
}

LILString LILStringToken::toString()
{
	LILString tokenstr = this->tokenStringRepresentation(this->type);
	return "LILStringToken of type: " + tokenstr + " and value: " + this->_value;
}

void LILStringToken::setHasArguments(bool newValue)
{
	this->_hasArguments = newValue;
}

bool LILStringToken::hasArguments() const
{
	return this->_hasArguments;
}

void LILStringToken::addArgument(std::shared_ptr<LIL::LILNode> parserNode)
{
	this->_arguments.push_back(parserNode);
}

const std::vector<std::shared_ptr<LILNode> > & LILStringToken::getArguments() const
{
	return this->_arguments;
}

void LILStringToken::addIndex(size_t index)
{
	this->_indexes.push_back(index);
}

const std::vector<size_t> & LILStringToken::getIndexes() const
{
	return this->_indexes;
}
