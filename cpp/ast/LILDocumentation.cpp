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
 *	  This file implements object definitions
 *
 ********************************************************************/

#include "LILDocumentation.h"
#include "LILObjectType.h"

using namespace LIL;

LILDocumentation::LILDocumentation()
: LILNode(NodeTypeDocumentation)
{
}

LILDocumentation::LILDocumentation(const LILDocumentation &other)
: LILNode(other)
, content(other.content)
{
}

std::shared_ptr<LILDocumentation> LILDocumentation::clone() const
{
	return std::static_pointer_cast<LILDocumentation> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILDocumentation::cloneImpl() const
{
	std::shared_ptr<LILDocumentation> clone(new LILDocumentation(*this));
	clone->clearChildNodes();
	
	for (auto it = this->getChildNodes().begin(); it!=this->getChildNodes().end(); ++it)
	{
		clone->addNode((*it)->clone());
	}
	return clone;
}

LILDocumentation::~LILDocumentation()
{
}

void LILDocumentation::receiveNodeData(const LIL::LILString &data)
{
	//the first two letters are always #= or #-
	auto newContent = data.substr(2);
	bool done = false;
	while (!done) {
		done = true;
		auto tempStr = newContent.strip('=').strip('-').strip(' ');
		if (tempStr.length() != newContent.length()) {
			newContent = tempStr;
			done = false;
		}
	}
	this->content = newContent;
}

const std::vector<std::shared_ptr<LILNode>> & LILDocumentation::getNodes() const
{
	return this->getChildNodes();
}

void LILDocumentation::add(const std::shared_ptr<LILNode> & node)
{
	this->addNode(node);
}

void LILDocumentation::setNodes(const std::vector<std::shared_ptr<LILNode>> && nodes)
{
	this->setChildNodes(std::move(nodes));
}

const LILString & LILDocumentation::getContent() const
{
	return this->content;
}

void LILDocumentation::setContent(const LILString & value)
{
	this->content = value;
}
