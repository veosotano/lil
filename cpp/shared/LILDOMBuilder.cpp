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
 *	  This file analyzes the code to determine types automatically
 *
 ********************************************************************/

#include "LILDOMBuilder.h"
#include "../ast/LILInstruction.h"
#include "../ast/LILNumberLiteral.h"
#include "../ast/LILObjectType.h"
#include "../ast/LILRootNode.h"
#include "../ast/LILRule.h"
#include "../ast/LILSelector.h"

using namespace LIL;

const std::shared_ptr<LILElement> & LILElement::add(LILString name, std::shared_ptr<LILType> ty, long int id)
{
	auto newItem = std::make_shared<LILElement>();
	newItem->name = name;
	newItem->ty = ty;
	newItem->id = id;
	this->children.push_back(newItem);
	return this->children.back();
}

void LILElement::remove(std::shared_ptr<LILElement> elem)
{
	this->children.erase(std::remove(this->children.begin(), this->children.end(), elem), this->children.end());
}

const std::shared_ptr<LILElement> & LILElement::at(size_t index) const
{
	return this->children.at(index);
}

const std::vector<std::shared_ptr<LILElement>> & LILElement::getChildren() const
{
	return this->children;
}

LILDOMBuilder::LILDOMBuilder()
: elementCount(0)
, insertionPoint(nullptr)
{
}

LILDOMBuilder::~LILDOMBuilder()
{
}

void LILDOMBuilder::initializeVisit()
{
	if (this->getVerbose()) {
		std::cerr << "\n\n";
		std::cerr << "============================\n";
		std::cerr << "=====  DOM BUILDING   ======\n";
		std::cerr << "============================\n\n";
	}
}

void LILDOMBuilder::visit(LILNode *node)
{
	
}

void LILDOMBuilder::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
	this->setRootNode(rootNode);
	this->createDOM();
}

void LILDOMBuilder::createDOM()
{
	auto root = std::make_shared<LILElement>();
	root->name = "@root";
	root->ty = LILObjectType::make("container");
	root->id = 0;
	this->dom = root;
	this->elementCount = 1;
	this->insertionPoint = this->dom.get();
	for (const auto & rule : this->getRootNode()->getRules()) {
		for (const auto & innerRule : rule->getChildRules()) {
			this->recursiveAddElement(innerRule.get());
		}
	}
}

void LILDOMBuilder::recursiveAddElement(LILRule * rule)
{
	auto insertionPointBackup = this->insertionPoint;
	
	const auto & instrNode = rule->getInstruction();
	if (instrNode && instrNode->getInstructionType() == InstructionTypeNew) {
		auto instr = std::static_pointer_cast<LILInstruction>(instrNode);
		long int iterations = 1;
		const auto & arg = instr->getArgument();
		if (arg && arg->getNodeType() == NodeTypeNumberLiteral) {
			auto numLit = std::static_pointer_cast<LILNumberLiteral>(arg);
			iterations = stol(numLit->getValue().data());
		}
		for (long int i = 0; i < iterations; i += 1) {
			auto firstSel = rule->getFirstSelector();
			auto nameSel = std::static_pointer_cast<LILSelector>(firstSel);
			auto ruleTy = rule->getType();
			if (!ruleTy) {
				ruleTy = instrNode->getType();
			}
			auto newElem = this->insertionPoint->add(nameSel->getName(), ruleTy, this->elementCount);
			this->elementCount += 1;
			this->insertionPoint = newElem.get();
			
			for (const auto & innerRule : rule->getChildRules()) {
				this->recursiveAddElement(innerRule.get());
			}
		}
	}
	this->insertionPoint = insertionPointBackup;
}

const std::shared_ptr<LILElement> & LILDOMBuilder::getDOM() const
{
	return this->dom;
}
