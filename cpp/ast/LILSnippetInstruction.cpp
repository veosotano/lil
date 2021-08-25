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
 *      This file implements the #snippet instructions
 *
 ********************************************************************/

#include "LILSnippetInstruction.h"

using namespace LIL;

LILSnippetInstruction::LILSnippetInstruction()
: LILInstruction(NodeTypeSnippetInstruction)
{
    this->setInstructionType(InstructionTypeSnippet);
}

LILSnippetInstruction::LILSnippetInstruction(const LILSnippetInstruction & other)
: LILInstruction(other)
, _body(other._body)
{
}

std::shared_ptr<LILSnippetInstruction> LILSnippetInstruction::clone() const
{
    return std::static_pointer_cast<LILSnippetInstruction> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILSnippetInstruction::cloneImpl() const
{
    std::shared_ptr<LILSnippetInstruction> clone = std::static_pointer_cast<LILSnippetInstruction>(LILInstruction::cloneImpl());
    
    clone->_body.clear();
    for (auto it = this->_body.begin(); it != this->_body.end(); ++it)
    {
        clone->add((*it)->clone());
    }
    return clone;
}

LILSnippetInstruction::~LILSnippetInstruction()
{
}

void LILSnippetInstruction::receiveNodeData(const LILString &data)
{
    
}

void LILSnippetInstruction::add(std::shared_ptr<LILNode> node)
{
    this->addNode(node);
    this->_body.push_back(node);
}

const std::vector<std::shared_ptr<LILNode>> & LILSnippetInstruction::getBody() const
{
    return this->_body;
}

void LILSnippetInstruction::setBody(const std::vector<std::shared_ptr<LILNode>> && newBody)
{
    this->_body = newBody;
    for (auto node : this->_body) {
        this->addNode(node);
    }
}

