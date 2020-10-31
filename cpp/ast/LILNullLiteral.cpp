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
 *      This file represents a null value in the source code
 *
 ********************************************************************/

#include "LILNullLiteral.h"
#include "LILType.h"

using namespace LIL;

LILNullLiteral::LILNullLiteral()
: LILNode(NodeTypeNull)
{
    
}

LILNullLiteral::LILNullLiteral(const LILNullLiteral & other)
: LILNode(other)
{

}

std::shared_ptr<LILNullLiteral> LILNullLiteral::clone() const
{
    return std::static_pointer_cast<LILNullLiteral> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILNullLiteral::cloneImpl() const
{
    return std::shared_ptr<LILNullLiteral>(new LILNullLiteral(*this));
}

LILNullLiteral::~LILNullLiteral()
{
    
}

void LILNullLiteral::receiveNodeData(const LIL::LILString &data)
{
    
}

LILString LILNullLiteral::stringRep()
{
    return "null";
}

bool LILNullLiteral::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    std::shared_ptr<LILNullLiteral> castedNode = std::static_pointer_cast<LILNullLiteral>(otherNode);
    return true;
}

std::shared_ptr<LILType> LILNullLiteral::getType() const
{
    static std::shared_ptr<LILType> nullTy;
    if (!nullTy) {
        nullTy = std::make_shared<LILType>();
        nullTy->setName("null");
    }
    return nullTy;
}
