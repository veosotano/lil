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
 *      This file is a combinator in a selector chain
 *
 ********************************************************************/

#include "LILCombinator.h"

using namespace LIL;

LILCombinator::LILCombinator()
: LIL::LILNode(NodeTypeCombinator)
{
    this->_combinatorType = CombinatorTypeNone;
}

LILCombinator::LILCombinator(const LILCombinator &other)
: LILNode(other)
{
    this->_combinatorType = other._combinatorType;
}

std::shared_ptr<LILCombinator> LILCombinator::clone() const
{
    return std::static_pointer_cast<LILCombinator> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILCombinator::cloneImpl() const
{
    std::shared_ptr<LILCombinator> clone(new LILCombinator(*this));
    return clone;
}

LILCombinator::~LILCombinator()
{
    
}

void LILCombinator::receiveNodeData(const LIL::LILString &data)
{
    if (data == "..")
    {
        this->setCombinatorType(CombinatorTypeDescendants);
    }
    else if (data == "=")
    {
        this->setCombinatorType(CombinatorTypeSiblings);
    }
    else if (data == "+")
    {
        this->setCombinatorType(CombinatorTypeNextSiblings);
    }
    else if (data == "-")
    {
        this->setCombinatorType(CombinatorTypePreviousSiblings);
    }
    else
    {
        this->setCombinatorType(CombinatorTypeChildren);
    }
}

LILString LILCombinator::stringRep()
{
    switch (this->getCombinatorType())
    {
        case CombinatorTypeDescendants:
        {
            return "descendants (..)";
            break;
        }
        case CombinatorTypeSiblings:
        {
            return "siblings (=)";
            break;
        }
        case CombinatorTypeNextSiblings:
        {
            return "next siblings (+)";
            break;
        }
        case CombinatorTypePreviousSiblings:
        {
            return "previous siblings (-)";
            break;
        }
        case CombinatorTypeChildren:
        {
            return "children";
            break;
        }

        default:
            break;
    }
    return "unkown combinator";
}

bool LILCombinator::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    //other checks
    if ( this->_combinatorType != otherNode->getCombinatorType()) return false;
    return true;
}

CombinatorType LILCombinator::getCombinatorType() const
{
    return this->_combinatorType;
}

void LILCombinator::setCombinatorType(CombinatorType newType)
{
    this->_combinatorType = newType;
}
