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
 *	  This file encapsulates an expression
 *
 ********************************************************************/

#include "LILUnaryExpression.h"

using namespace LIL;

LILString LILUnaryExpression::expressionTypeToString(UnaryExpressionType type)
{
	switch (type) {
		case UnaryExpressionTypeSum:
			return "sum";
		case UnaryExpressionTypeSubtraction:
			return "subtraction";
		case UnaryExpressionTypeMultiplication:
			return "multiplication";
		case UnaryExpressionTypeDivision:
			return "division";

		default:
			break;
	}
	return "";
}

ExpressionType LILUnaryExpression::uexpToExpType(UnaryExpressionType type)
{
	switch (type) {
		case UnaryExpressionTypeSum:
			return ExpressionTypeSum;
		case UnaryExpressionTypeSubtraction:
			return ExpressionTypeSubtraction;
		case UnaryExpressionTypeMultiplication:
			return ExpressionTypeMultiplication;
		case UnaryExpressionTypeDivision:
			return ExpressionTypeDivision;

		default:
			break;
	}
	return ExpressionTypeNone;
}

LILUnaryExpression::LILUnaryExpression()
: LILTypedNode(NodeTypeUnaryExpression)
{
	
}

LILUnaryExpression::LILUnaryExpression(const LILUnaryExpression &orig)
: LILTypedNode(orig)
{
	this->_uexpType = orig._uexpType;
	this->_value = orig._value;
	this->_subject = orig._subject;
}

std::shared_ptr<LILUnaryExpression> LILUnaryExpression::clone() const
{
	return std::static_pointer_cast<LILUnaryExpression> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILUnaryExpression::cloneImpl() const
{
	std::shared_ptr<LILUnaryExpression> clone(new LILUnaryExpression(*this));
	clone->clearChildNodes();
	
	if (this->_value)
	{
		clone->setValue(this->_value->clone());
	}
	if (this->_subject)
	{
		clone->setSubject(this->_subject->clone());
	}
	//clone LILTypedNode
	if (this->_type) {
		clone->setType(this->_type->clone());
	}
	return clone;
}

LILUnaryExpression::~LILUnaryExpression()
{
	
}

bool LILUnaryExpression::isA(UnaryExpressionType otherType) const
{
	return otherType == this->_uexpType;
}

UnaryExpressionType LILUnaryExpression::getUnaryExpressionType() const
{
	return this->_uexpType;
}

void LILUnaryExpression::setUnaryExpressionType(UnaryExpressionType newType)
{
	this->_uexpType = newType;
}

void LILUnaryExpression::setValue(std::shared_ptr<LILNode> newValue)
{
	this->_value = newValue;
	this->addNode(newValue);
}

std::shared_ptr<LILNode> LILUnaryExpression::getValue()
{
	return this->_value;
}

void LILUnaryExpression::setSubject(std::shared_ptr<LILNode> newSubject)
{
	this->_subject = newSubject;
	this->addNode(newSubject);
}

std::shared_ptr<LILNode> LILUnaryExpression::getSubject()
{
	return this->_subject;
}

void LILUnaryExpression::receiveNodeData(const LILString &data)
{
	if (data == "+") {
		this->setUnaryExpressionType(UnaryExpressionTypeSum);
	}
	else if (data == "-") {
		this->setUnaryExpressionType(UnaryExpressionTypeSubtraction);
	}
	else if (data == "*") {
		this->setUnaryExpressionType(UnaryExpressionTypeMultiplication);
	}
	else if (data == "/") {
		this->setUnaryExpressionType(UnaryExpressionTypeDivision);
	}
	else if (data == "!") {
		this->setUnaryExpressionType(UnaryExpressionTypeNot);
	}
}
