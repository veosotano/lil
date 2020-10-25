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
 *      This file sorts the fields of classes for optimal memory usage
 *      WARNING: this is just a stub. this hasn't been implemented yet
 *
 ********************************************************************/

#include "LILFieldSorter.h"
#include "LILVarNode.h"

using namespace LIL;

LILFieldSorter::LILFieldSorter()
{
}

LILFieldSorter::~LILFieldSorter()
{
}

void LILFieldSorter::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  FIELD SORTING   =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILFieldSorter::visit(LILNode *node)
{
    this->process(node);
}

void LILFieldSorter::process(LILNode * node)
{
    if (this->getDebug()) {
        std::cerr << "## sorting fields " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBoolLiteral:
        {
            LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeNumberLiteral:
        {
            LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypePercentage:
        {
            LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeExpression:
        {
            LILExpression * value = static_cast<LILExpression *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeStringLiteral:
        {
            LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeStringFunction:
        {
            LILStringFunction * value = static_cast<LILStringFunction *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeNull:
        {
            LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeVarDecl:
        {
            LILVarDecl * value = static_cast<LILVarDecl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeClassDecl:
        {
            LILClassDecl * value = static_cast<LILClassDecl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeObjectDefinition:
        {
            LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeAssignment:
        {
            LILAssignment * value = static_cast<LILAssignment *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeValuePath:
        {
            LILValuePath * value = static_cast<LILValuePath *>(node);
            this->_process(value);
            break;
        }
        case NodeTypePropertyName:
        {
            LILPropertyName * value = static_cast<LILPropertyName *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeVarName:
        {
            LILVarName * value = static_cast<LILVarName *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeRule:
        {
            LILRule * value = static_cast<LILRule *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeSimpleSelector:
        {
            LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeSelectorChain:
        {
            LILSelectorChain * value = static_cast<LILSelectorChain *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeSelector:
        {
            LILSelector * value = static_cast<LILSelector *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeCombinator:
        {
            LILCombinator * value = static_cast<LILCombinator *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFilter:
        {
            LILFilter * value = static_cast<LILFilter *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFlag:
        {
            LILFlag * value = static_cast<LILFlag *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFunctionCall:
        {
            LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFlowControl:
        {
            LILFlowControl * value = static_cast<LILFlowControl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeType:
        {
            break;
        }
            
        default:
            std::cerr << "Error: unkonwn node type to process\n";
            break;
    }
}

void LILFieldSorter::_process(LILBoolLiteral * value)
{
    
}

void LILFieldSorter::_process(LILNumberLiteral * value)
{
    
}

void LILFieldSorter::_process(LILPercentageLiteral * value)
{
    
}

void LILFieldSorter::_process(LILExpression * value)
{
    
}

void LILFieldSorter::_process(LILStringLiteral * value)
{
    
}

void LILFieldSorter::_process(LILStringFunction * value)
{
    this->processChildren(value->getNodes());
}

void LILFieldSorter::_process(LILNullLiteral * value)
{
    
}

void LILFieldSorter::_process(LILVarDecl * value)
{
    
}

void LILFieldSorter::_process(LILClassDecl * value)
{
    if (!value->getIsExtern()) {
        this->processChildren(value->getFields());
        this->processChildren(value->getMethods());
    }
}

void LILFieldSorter::_process(LILObjectDefinition * value)
{
    this->processChildren(value->getNodes());
}

void LILFieldSorter::_process(LILAssignment * value)
{
    this->processChildren(value->getNodes());
}

void LILFieldSorter::_process(LILValuePath * value)
{
    this->processChildren(value->getNodes());
}

void LILFieldSorter::_process(LILPropertyName * value)
{
}

void LILFieldSorter::_process(LILRule * value)
{
    this->processChildren(value->getNodes());
}

void LILFieldSorter::_process(LILSimpleSelector * value)
{
    
}

void LILFieldSorter::_process(LILSelectorChain * value)
{
    
}

void LILFieldSorter::_process(LILSelector * value)
{
}

void LILFieldSorter::_process(LILCombinator * value)
{
}

void LILFieldSorter::_process(LILFilter * value)
{
}

void LILFieldSorter::_process(LILFlag * value)
{
}

void LILFieldSorter::_process(LILVarName * value)
{
}

void LILFieldSorter::_process(LILFunctionDecl * value)
{
    
}

void LILFieldSorter::_process(LILFunctionCall * value)
{
    
}

void LILFieldSorter::_process(LILFlowControl * value)
{
    this->processChildren(value->getThen());
    this->processChildren(value->getElse());
}

void LILFieldSorter::_process(LILFlowControlCall * value)
{
    this->process(value->getArgument().get());
}

void LILFieldSorter::_process(LILInstruction * value)
{
}

void LILFieldSorter::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process((*it).get());
    };
}
