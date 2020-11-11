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
 *      This file encapsulates an expression
 *
 ********************************************************************/

#include "LILExpression.h"

using namespace LIL;

LILString LILExpression::expressionTypeToString(ExpressionType type)
{
    switch (type) {
        case ExpressionTypeSum:
            return "sum";
        case ExpressionTypeSubtraction:
            return "subtraction";
        case ExpressionTypeMultiplication:
            return "multiplication";
        case ExpressionTypeDivision:
            return "division";
        case ExpressionTypeEqualComparison:
            return "equal comparison";
        case ExpressionTypeNotEqualComparison:
            return "not equal comparison";
        case ExpressionTypeBiggerComparison:
            return "bigger comparison";
        case ExpressionTypeBiggerOrEqualComparison:
            return "bigger or equal comparison";
        case ExpressionTypeSmallerComparison:
            return "smaller comparison";
        case ExpressionTypeSmallerOrEqualComparison:
            return "smaller or equal comparison";
        case ExpressionTypeLogicalAnd:
            return "logical and";
        case ExpressionTypeLogicalOr:
            return "logical or";
        case ExpressionTypeBitwiseAnd:
            return "bitwise and";
        case ExpressionTypeBitwiseOr:
            return "bitwise or";
        case ExpressionTypeCast:
            return "cast";

        default:
            break;
    }
    return "";
}

LILExpression::LILExpression()
: LILTypedNode(NodeTypeExpression)
{

}

LILExpression::LILExpression(const LILExpression &orig)
: LILTypedNode(orig)
{
    this->expressionType = orig.expressionType;
    this->_leftNode = orig._leftNode;
    this->_rightNode = orig._rightNode;
}

std::shared_ptr<LILExpression> LILExpression::clone() const
{
    return std::static_pointer_cast<LILExpression> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILExpression::cloneImpl() const
{
    std::shared_ptr<LILExpression> clone(new LILExpression(*this));
    clone->clearChildNodes();

    if (this->_leftNode)
    {
        clone->setLeft(this->_leftNode->clone());
    }
    if (this->_rightNode)
    {
        clone->setRight(this->_rightNode->clone());
    }
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILExpression::~LILExpression()
{

}

LILString LILExpression::stringRep()
{
    LILString tempstr("");
    if (this->getLeft())
    {
        tempstr += this->getLeft()->stringRep();
    }
    switch (this->expressionType)
    {
        case ExpressionTypeSum:
        {
            tempstr += " + ";
            break;
        }
        case ExpressionTypeSubtraction:
        {
            tempstr += " - ";
            break;
        }
        case ExpressionTypeMultiplication:
        {
            tempstr += " * ";
            break;
        }
        case ExpressionTypeDivision:
        {
            tempstr += " / ";
            break;
        }
        case ExpressionTypeEqualComparison:
        {
            tempstr += " = ";
            break;
        }
        case ExpressionTypeNotEqualComparison:
        {
            tempstr += " != ";
            break;
        }
        case ExpressionTypeBiggerComparison:
        {
            tempstr += " > ";
            break;
        }
        case ExpressionTypeBiggerOrEqualComparison:
        {
            tempstr += " >= ";
            break;
        }
        case ExpressionTypeSmallerComparison:
        {
            tempstr += " < ";
            break;
        }
        case ExpressionTypeSmallerOrEqualComparison:
        {
            tempstr += " <= ";
            break;
        }
        case ExpressionTypeLogicalAnd:
        {
            tempstr += " && ";
            break;
        }
        case ExpressionTypeLogicalOr:
        {
            tempstr += " || ";
            break;
        }
        case ExpressionTypeBitwiseAnd:
        {
            tempstr += " & ";
            break;
        }
        case ExpressionTypeBitwiseOr:
        {
            tempstr += " | ";
            break;
        }
        default:
            break;
    }
    if (this->getRight())
    {
        tempstr += this->getRight()->stringRep();
    }
    return tempstr;
}

bool LILExpression::isA(ExpressionType otherType) const
{
    return otherType == this->expressionType;
}

ExpressionType LILExpression::getExpressionType() const
{
    return this->expressionType;
}

void LILExpression::setExpressionType(ExpressionType newType)
{
    this->expressionType = newType;
}

void LILExpression::setLeft(std::shared_ptr<LILNode> newLeft)
{
    this->_leftNode = newLeft;
    if (this->_childNodes.size() == 0) {
        this->addNode(newLeft);
    } else {
        newLeft->setParentNode(this->shared_from_this());
        this->_childNodes[0].swap(newLeft);
    }
}

std::shared_ptr<LILNode> LILExpression::getLeft()
{
    return this->_leftNode;
}

void LILExpression::setRight(std::shared_ptr<LILNode> newRight)
{
    this->_rightNode = newRight;
    if (this->_childNodes.size() > 1) {
        this->_childNodes.pop_back();
    }
    this->addNode(newRight);
}

std::shared_ptr<LILNode> LILExpression::getRight()
{
    return this->_rightNode;
}

const std::vector<std::shared_ptr<LILNode>> & LILExpression::getNodes() const
{
    return this->getChildNodes();
}
