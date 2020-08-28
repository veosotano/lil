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
 *      This file represents a string using parameters
 *
 ********************************************************************/

#include "LILStringFunction.h"

using namespace LIL;

LILStringFunction::LILStringFunction()
: LILNode(NodeTypeStringFunction)
{
    
}

LILStringFunction::LILStringFunction(const LILStringFunction & other)
: LILNode(other)
{
    this->_startChunk = other._startChunk;
    this->_midChunks = other._midChunks;
    this->_endChunk = other._endChunk;
}

std::shared_ptr<LILStringFunction> LILStringFunction::clone() const
{
    return std::static_pointer_cast<LILStringFunction> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILStringFunction::cloneImpl() const
{
    return std::shared_ptr<LILStringFunction>(new LILStringFunction(*this));
}

LILStringFunction::~LILStringFunction()
{
    
}

void LILStringFunction::receiveNodeData(const LIL::LILString &data)
{
    
}

void LILStringFunction::setStartChunk(LILString newValue)
{
    this->_startChunk = newValue;
}
LILString LILStringFunction::getStartChunk() const
{
    return this->_startChunk;
}
void LILStringFunction::addMidChunk(LILString newValue)
{
    this->_midChunks.push_back(newValue);
}
std::vector<LILString> LILStringFunction::getMidChunks() const
{
    return this->_midChunks;
}
void LILStringFunction::setEndChunk(LILString newValue)
{
    this->_endChunk = newValue;
}
LILString LILStringFunction::getEndChunk() const
{
    return this->_endChunk;
}

LILString LILStringFunction::stringRep()
{
    return this->_startChunk + "...";
}

bool LILStringFunction::equalTo(std::shared_ptr<LILNode> otherNode)
{
    if ( ! LILNode::equalTo(otherNode)) return false;
    std::shared_ptr<LILStringFunction> castedNode = std::static_pointer_cast<LILStringFunction>(otherNode);
    if ( this->_startChunk != castedNode->_startChunk ) return false;
    if ( this->_endChunk != castedNode->_endChunk ) return false;
    if ( this->_midChunks.size() != castedNode->_midChunks.size() ) return false;
    for (size_t i = 0, j = this->_midChunks.size(); i<j; ++i) {
        if ( this->_midChunks[i] != castedNode->_midChunks[i] ) return false;
    }
    return true;
}

const std::vector<std::shared_ptr<LILNode>> & LILStringFunction::getNodes() const
{
    return this->getChildNodes();
}
