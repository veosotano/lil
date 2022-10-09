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
 *	  This file holds multiple values separated by commas
 *
 ********************************************************************/

#include "LILValueList.h"
#include "LILVarNode.h"

using namespace LIL;

LILValueList::LILValueList()
: LILTypedNode(NodeTypeValueList)
{
	
}

LILValueList::LILValueList(const LILValueList &other)
: LILTypedNode(other)
{
	
}

std::shared_ptr<LILValueList> LILValueList::clone() const
{
	return std::static_pointer_cast<LILValueList> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILValueList::cloneImpl() const
{
	std::shared_ptr<LILValueList> clone(new LILValueList(*this));
	for (auto node : this->getChildNodes()) {
		clone->addValue(node->clone());
	}
	//clone LILTypedNode
	if (this->_type) {
		clone->setType(this->_type->clone());
	}
	return clone;
}

LILValueList::~LILValueList()
{
	
}

void LILValueList::receiveNodeData(const LIL::LILString &data)
{

}

void LILValueList::addValue(std::shared_ptr<LILNode> arg)
{
	this->addNode(arg);
}

void LILValueList::setValues(std::vector<std::shared_ptr<LILNode>> vals)
{
	this->clearChildNodes();
	for (auto val : vals) {
		this->addValue(val);
	}
}

void LILValueList::clearValues()
{
	this->clearChildNodes();
}

std::vector<std::shared_ptr<LILNode>> LILValueList::getValues() const
{
	return this->getChildNodes();
}
