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
 *
 ********************************************************************/

#include "LILParameterSorter.h"
#include "LILVarNode.h"

using namespace LIL;

LILParameterSorter::LILParameterSorter()
: _debug(false)
{
}

LILParameterSorter::~LILParameterSorter()
{
}

void LILParameterSorter::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  PARAMETER SORTING   =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILParameterSorter::visit(LILNode *node)
{
    this->process(node);
}

void LILParameterSorter::process(LILNode * node)
{
    if (this->_debug) {
        std::cerr << "## sorting parameters " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBool:
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

void LILParameterSorter::_process(LILBoolLiteral * value)
{
    
}

void LILParameterSorter::_process(LILNumberLiteral * value)
{
    
}

void LILParameterSorter::_process(LILPercentageLiteral * value)
{
    
}

void LILParameterSorter::_process(LILExpression * value)
{
    this->process(value->getLeft().get());
    this->process(value->getRight().get());
}

void LILParameterSorter::_process(LILStringLiteral * value)
{
    
}

void LILParameterSorter::_process(LILStringFunction * value)
{
    this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILNullLiteral * value)
{
    
}

void LILParameterSorter::_process(LILVarDecl * value)
{
    this->processChildren(value->getInitVals());
}

void LILParameterSorter::_process(LILClassDecl * value)
{
    this->processChildren(value->getFields());
    this->processChildren(value->getMethods());
}

void LILParameterSorter::_process(LILObjectDefinition * value)
{
    this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILAssignment * value)
{
    this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILValuePath * value)
{
    this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILPropertyName * value)
{
}

void LILParameterSorter::_process(LILRule * value)
{
    this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILSimpleSelector * value)
{
    
}

void LILParameterSorter::_process(LILSelectorChain * value)
{
    this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILSelector * value)
{
    
}

void LILParameterSorter::_process(LILCombinator * value)
{
}

void LILParameterSorter::_process(LILFilter * value)
{
}

void LILParameterSorter::_process(LILFlag * value)
{
}

void LILParameterSorter::_process(LILVarName * value)
{
}

void LILParameterSorter::_process(LILFunctionDecl * value)
{
    this->processChildren(value->getBody());
}

void LILParameterSorter::_process(LILFunctionCall * value)
{
    if (value->getFunctionCallType() != FunctionCallTypeNone) {
        return;
    }
    
    auto vp = std::static_pointer_cast<LILValuePath>(value->getParentNode());
    
    auto childNodes = vp->getNodes();
    if (childNodes.size() > 1)
    {
        auto it = childNodes.begin();
        auto firstNode = *it;
        bool isExtern = false;
        std::shared_ptr<LILVarDecl> vd;
        LILNode * subject;
        LILString varName;
        
        if (firstNode->isA(NodeTypeVarName)) {
            auto vn = std::static_pointer_cast<LILVarName>(firstNode);
            std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
            subject = subjectNode.get();
            if (subjectNode && subjectNode->isA(NodeTypeVarDecl)) {
                vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
                varName = vd->getName();
                if (vd->getIsExtern()) {
                    isExtern = true;
                }
            }
        } else {
            //selector
            auto sel = std::static_pointer_cast<LILSelector>(firstNode);
            switch (sel->getSelectorType()) {
                default:
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
                    return;
            }
        }
        
        if (isExtern) {
            return;
        }
        
        ++it;
        
        while (it != childNodes.end()) {
            auto currentNode = *it;
            switch (currentNode->getNodeType()) {
                case NodeTypeFunctionCall:
                {
                    auto fc = std::static_pointer_cast<LILFunctionCall>(currentNode);
                    auto initVal = vd->getInitVal();
                    if (initVal->isA(NodeTypeFunctionDecl)) {
                        auto declArgs = std::static_pointer_cast<LILFunctionDecl>(initVal)->getArguments();
                        
                        std::vector<std::shared_ptr<LILNode>> newArgs;
                        
                        //in the order of the arguments in the declaration
                        for (auto declArg : declArgs) {
                            if (!declArg->isA(NodeTypeVarDecl)) {
                                std::cerr << "!!!!!!!!!!DECL ARG WAS NOT VAR DECL FAIL!!!!!!!!!!!!!!!!\n";
                                return;
                            }
                            auto declVd = std::static_pointer_cast<LILVarDecl>(declArg);
                            
                            //find the argument in the call
                            auto callArgs = fc->getArguments();
                            bool found = false;
                            for (auto callArg : callArgs) {
                                if (!callArg->isA(NodeTypeAssignment)) {
                                    std::cerr << "!!!!!!!!!!CALL ARG WAS NOT ASSIGNMENT FAIL!!!!!!!!!!!!!!!!\n";
                                    return;
                                }
                                auto callAsgmt = std::static_pointer_cast<LILAssignment>(callArg);
                                auto callArgFirstNode = callAsgmt->getNodes().front();
                                if (!callArgFirstNode->isA(NodeTypeVarName)) {
                                    std::cerr << "!!!!!!!!!!FIRST NODE WAS NOT VAR NAME FAIL!!!!!!!!!!!!!!!!\n";
                                    return;
                                }
                                auto cavn = std::static_pointer_cast<LILVarName>(callArgFirstNode);
                                if (declVd->getName() == cavn->getName()) {
                                    found = true;
                                    newArgs.push_back(callAsgmt);
                                    break;
                                }
                            }
                            //if we need the default value
                            if (!found && declVd->getInitVal()) {
                                newArgs.push_back(this->_varDeclToAssignment(declVd));
                            }
                        }
                        
                        fc->setArguments(std::move(newArgs));
                        
                    } else {
                        std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
                    }
                    
                    break;
                }
                    
                case NodeTypePropertyName:
                {
                    auto pn = std::static_pointer_cast<LILPropertyName>(currentNode);
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
                    break;
                }

                default:
                    break;
            }
            
            ++it;
        }
    }
}

void LILParameterSorter::_process(LILFlowControl * value)
{
    this->processChildren(value->getThen());
    this->processChildren(value->getElse());
}

void LILParameterSorter::_process(LILInstruction * value)
{
}

void LILParameterSorter::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process((*it).get());
    };
}

void LILParameterSorter::setDebug(bool value)
{
    this->_debug = value;
}

std::shared_ptr<LILAssignment> LILParameterSorter::_varDeclToAssignment(std::shared_ptr<LILVarDecl> vd)
{
    std::shared_ptr<LILAssignment> ret = std::make_shared<LILAssignment>();
    std::shared_ptr<LILValuePath> vp = std::make_shared<LILValuePath>();
    std::shared_ptr<LILVarName> vn = std::make_shared<LILVarName>();
    vn->setName(vd->getName());
    vp->addChild(vn);
    ret->setSubject(vp);
    ret->setValue(vd->getInitVal());
    return ret;
    
}
