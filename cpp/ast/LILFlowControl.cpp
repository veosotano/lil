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
 *      This file represents if, else, for, swith and loop statements
 *
 ********************************************************************/

#include "LILFlowControl.h"
#include "LILVarDecl.h"

using namespace LIL;

LILFlowControl::LILFlowControl()
: LILVarNode(NodeTypeFlowControl)
{
    this->_receivesFunctionBody = false;
    this->_receivesElse = false;
    this->_flowControlType = FlowControlTypeNone;
}

LILFlowControl::LILFlowControl(const LILFlowControl &other)
: LILVarNode(other)
{
    this->_arguments = other._arguments;
    this->_then = other._then;
    this->_else = other._else;
    this->_receivesFunctionBody = other._receivesFunctionBody;
    this->_receivesElse = other._receivesElse;
    this->_flowControlType = other._flowControlType;
    this->_subjectNode = other._subjectNode;
}

std::shared_ptr<LILFlowControl> LILFlowControl::clone() const
{
    return std::static_pointer_cast<LILFlowControl> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILFlowControl::cloneImpl() const
{
    std::shared_ptr<LILFlowControl> clone(new LILFlowControl(*this));
    clone->clearChildNodes();

    clone->clearLocalVars();

    clone->_arguments.clear();
    for (auto it = this->_arguments.begin(); it != this->_arguments.end(); ++it)
    {
        clone->addArgument((*it)->clone());
    }
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
    if (this->_subjectNode) {
        clone->setSubject(this->_subjectNode->clone());
    }

    return clone;
}

LILFlowControl::~LILFlowControl()
{
    
}

void LILFlowControl::receiveNodeData(const LILString &data)
{
    if (data == "if")
    {
        this->setFlowControlType(FlowControlTypeIf);
    }
    else if (data == "else")
    {
        this->setFlowControlType(FlowControlTypeElse);
    }
    else if (data == "switch")
    {
        this->setFlowControlType(FlowControlTypeSwitch);
    }
    else if (data == "case")
    {
        this->setFlowControlType(FlowControlTypeCase);
    }
    else if (data == "default")
    {
        this->setFlowControlType(FlowControlTypeDefault);
    }
    else if (data == "loop")
    {
        this->setFlowControlType(FlowControlTypeLoop);
    }
    else if (data == "for")
    {
        this->setFlowControlType(FlowControlTypeFor);
    }
    else if (data == "finally")
    {
        this->setFlowControlType(FlowControlTypeFinally);
    }
}

void LILFlowControl::addArgument(std::shared_ptr<LILNode> arg)
{
    this->addNode(arg);
    this->_arguments.push_back(arg);
    
    if (!arg->isA(NodeTypeVarDecl))
        return;
    std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(arg);
    this->setLocalVariable(vd->getName(), vd);
}

const std::vector<std::shared_ptr<LILNode>> & LILFlowControl::getArguments() const
{
    return this->_arguments;
}

void LILFlowControl::clearArguments()
{
    this->_arguments.clear();
}

void LILFlowControl::setArguments(const std::vector<std::shared_ptr<LILNode>> && newArgs)
{
    for (auto node : this->_arguments) {
        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            this->unsetLocalVariable(vd->getName());
        }

        this->removeNode(node);
    }
    this->_arguments = std::move(newArgs);
    for (auto arg : this->_arguments) {
        arg->setParentNode(this->shared_from_this());
        if (arg->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(arg);
            this->setLocalVariable(vd->getName(), arg);
        }
    }
}

void LILFlowControl::addThen(std::shared_ptr<LILNode> node)
{
    this->addNode(node);
    this->_then.push_back(node);

    if (node->isA(NodeTypeVarDecl)) {
        auto vd = std::static_pointer_cast<LILVarDecl>(node);
        this->setLocalVariable(vd->getName(), node);
    }
}

const std::vector<std::shared_ptr<LILNode>> & LILFlowControl::getThen() const
{
    return this->_then;
}

void LILFlowControl::setThen(const std::vector<std::shared_ptr<LILNode>> && newThen)
{
    for (auto node : this->_then) {
        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            this->unsetLocalVariable(vd->getName());
        }

        this->removeNode(node);
    }
    this->_then = std::move(newThen);
    for (auto node : this->_then) {
        this->addNode(node);

        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            this->setLocalVariable(vd->getName(), node);
        }
    }
}

void LILFlowControl::addElse(std::shared_ptr<LILNode> node)
{
    this->addNode(node);
    this->_else.push_back(node);

    if (node->isA(NodeTypeVarDecl)) {
        auto vd = std::static_pointer_cast<LILVarDecl>(node);
        this->setLocalVariable(vd->getName(), vd);
    }
}

void LILFlowControl::setElse(const std::vector<std::shared_ptr<LILNode>> && newElse)
{
    for (auto node : this->_else) {
        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            this->unsetLocalVariable(vd->getName());
        }

        this->removeNode(node);
    }
    this->_else = std::move(newElse);
    for (auto node : this->_else) {
        this->addNode(node);

        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            this->setLocalVariable(vd->getName(), node);
        }
    }
}

const std::vector<std::shared_ptr<LILNode>> & LILFlowControl::getElse() const
{
    return this->_else;
}

void LILFlowControl::setReceivesFunctionBody(bool newValue)
{
    this->_receivesFunctionBody = newValue;
}

bool LILFlowControl::getReceivesFunctionBody() const
{
    return this->_receivesFunctionBody;
}

void LILFlowControl::setReceivesElse(bool newValue)
{
    this->_receivesElse = newValue;
}

bool LILFlowControl::getReceivesElse() const
{
    return this->_receivesElse;
}

bool LILFlowControl::isA(LIL::FlowControlType type) const
{
    return this->_flowControlType == type;
}

FlowControlType LILFlowControl::getFlowControlType() const
{
    return this->_flowControlType;
}

void LILFlowControl::setFlowControlType(FlowControlType newType)
{
    this->_flowControlType = newType;
}

void LILFlowControl::setSubject(std::shared_ptr<LILNode> node)
{
    if (this->_subjectNode) {
        this->removeNode(this->_subjectNode);
    }
    this->_subjectNode = node;
    this->addNode(node);
}

const std::shared_ptr<LILNode> & LILFlowControl::getSubject() const
{
    return this->_subjectNode;
}
