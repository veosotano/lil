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
 *	  This file implements the #if instructions
 *
 ********************************************************************/

#include "LILIfInstruction.h"

using namespace LIL;

LILIfInstruction::LILIfInstruction()
: LILInstruction(NodeTypeIfInstruction)
, _receivesThen(false)
, _receivesElse(false)
{
	this->setInstructionType(InstructionTypeIf);
}

LILIfInstruction::LILIfInstruction(const LILIfInstruction & other)
: LILInstruction(other)
, _then(other._then)
, _else(other._else)
, _receivesThen(other._receivesThen)
, _receivesElse(other._receivesElse)
{
}

std::shared_ptr<LILIfInstruction> LILIfInstruction::clone() const
{
	return std::static_pointer_cast<LILIfInstruction> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILIfInstruction::cloneImpl() const
{
	std::shared_ptr<LILIfInstruction> clone(new LILIfInstruction(*this));

	clone->_then.clear();
	for (auto it = this->_then.begin(); it != this->_then.end(); ++it)
	{
		clone->addThen((*it)->clone());
	}
	clone->_else.clear();
	for (auto it = this->_else.begin(); it != this->_else.end(); ++it)
	{
		clone->addElse((*it)->clone());
	}
	return clone;
}

LILIfInstruction::~LILIfInstruction()
{
}

void LILIfInstruction::receiveNodeData(const LILString &data)
{
	
}

void LILIfInstruction::addThen(std::shared_ptr<LILNode> node)
{
	this->addNode(node);
	this->_then.push_back(node);
}

const std::vector<std::shared_ptr<LILNode>> & LILIfInstruction::getThen() const
{
	return this->_then;
}

void LILIfInstruction::setThen(std::vector<std::shared_ptr<LILNode>> newThen)
{
	this->_then = newThen;
	for (auto node : this->_then) {
		this->addNode(node);
	}
}

void LILIfInstruction::addElse(std::shared_ptr<LILNode> node)
{
	this->addNode(node);
	this->_else.push_back(node);
}

void LILIfInstruction::setElse(std::vector<std::shared_ptr<LILNode>> newElse)
{
	this->_else = newElse;
	for (auto node : this->_else) {
		this->addNode(node);
	}
}

const std::vector<std::shared_ptr<LILNode>> & LILIfInstruction::getElse() const
{
	return this->_else;
}

void LILIfInstruction::setReceivesThen(bool newValue)
{
	this->_receivesThen = newValue;
}

bool LILIfInstruction::getReceivesThen() const
{
	return this->_receivesThen;
}

void LILIfInstruction::setReceivesElse(bool newValue)
{
	this->_receivesElse = newValue;
}

bool LILIfInstruction::getReceivesElse() const
{
	return this->_receivesElse;
}
