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
    if (this->getDebug()) {
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
        case NodeTypeFlowControlCall:
        {
            LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
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
    if (!value->getIsExtern()) {
        this->processChildren(value->getFields());
        this->processChildren(value->getMethods());
    }
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
    if (value->getFunctionCallType() != FunctionCallTypeValuePath) {
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
        LILString varName;
        
        if (firstNode->isA(NodeTypeVarName)) {
            auto vn = std::static_pointer_cast<LILVarName>(firstNode);
            std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
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
                    auto ty = vd->getType();
                    if (!ty->isA(TypeTypeObject)) {
                        std::cerr << "VALUE PATH NODE DOES NOT POINT TO OBJECT FAIL!!!!!!!!\n";
                        return;
                    }
                    auto classDecl = this->findClassWithName(ty->getName());
                    if (!classDecl) {
                        std::cerr << "CLASS NOT FOUND FAIL!!!!!!!!\n";
                        return;
                    }
                    auto method = classDecl->getMethodNamed(fc->getName());
                    if (!method) {
                        std::cerr << "METHOD NOT FOUND FAIL!!!!!!!!\n";
                        return;
                    }
                    if (!method->isA(NodeTypeVarDecl)) {
                        std::cerr << "METHOD WAS NOT VAR DECL FAIL!!!!!!!!\n";
                        return;
                    }
                    vd = std::static_pointer_cast<LILVarDecl>(method);
                    
                    ty = vd->getType();
                    if (ty->isA(TypeTypeFunction)) {
                        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                        auto declArgs = fnTy->getArguments();
                        auto callArgs = fc->getArguments();
                        std::vector<std::shared_ptr<LILAssignment>> asgmtArgs;
                        std::vector<std::shared_ptr<LILNode>> plainArgs;
                        
                        for (auto callArg : callArgs) {
                            if (callArg->isA(NodeTypeAssignment)) {
                                asgmtArgs.push_back(std::static_pointer_cast<LILAssignment>(callArg));
                            } else {
                                plainArgs.push_back(callArg);
                            }
                        }
                        
                        std::vector<std::shared_ptr<LILNode>> newArgs;
                        
                        //in the order of the arguments in the declaration
                        size_t plainArgCount = 0;
                        for (auto declArg : declArgs) {
                            if (!declArg->isA(NodeTypeVarDecl)) {
                                std::cerr << "!!!!!!!!!!DECL ARG WAS NOT VAR DECL FAIL!!!!!!!!!!!!!!!!\n";
                                return;
                            }
                            auto declVd = std::static_pointer_cast<LILVarDecl>(declArg);
                            
                            //find the argument in the call
                            bool found = false;
                            for (auto asgmtArg : asgmtArgs) {
                                auto callAsgmtSubj = asgmtArg->getSubject();
                                if (!callAsgmtSubj->isA(NodeTypeVarName)) {
                                    std::cerr << "!!!!!!!!!!SUBJECT OF ASSIGNMENT WAS NOT VAR NAME FAIL!!!!!!!!!!!!!!!!\n";
                                    return;
                                }
                                auto caVn = std::static_pointer_cast<LILVarName>(callAsgmtSubj);
                                if (declVd->getName() == caVn->getName()) {
                                    found = true;
                                    newArgs.push_back(asgmtArg);
                                    break;
                                }
                            }
                            if (!found && plainArgs.size() >= plainArgCount+1) {
                                auto newAsgmt = std::make_shared<LILAssignment>();
                                auto newVn = std::make_shared<LILVarName>();
                                newVn->setName(declVd->getName());
                                auto callArg = plainArgs[plainArgCount];
                                auto callArgTy = callArg->getType();
                                if (!callArg) {
                                    std::cerr << "!!!!!!!!!!CALL ARG HAD NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
                                    return;
                                }
                                newAsgmt->setType(callArgTy->clone());
                                
                                newAsgmt->setSubject(newVn);
                                newAsgmt->setValue(callArg);
                                newArgs.push_back(newAsgmt);
                                ++plainArgCount;
                                found = true;
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
                    auto vdTy = vd->getType();
                    if (!vdTy->isA(TypeTypeObject)) {
                        std::cerr << "VALUE PATH NODE DOES NOT POINT TO OBJECT FAIL!!!!!!!!\n";
                        return;
                    }
                    auto classDecl = this->findClassWithName(vdTy->getName());
                    if (!classDecl) {
                        std::cerr << "CLASS NOT FOUND FAIL!!!!!!!!\n";
                        return;
                    }
                    auto field = classDecl->getFieldNamed(pn->getName());
                    if (!field) {
                        std::cerr << "FIELD NOT FOUND FAIL!!!!!!!!\n";
                        return;
                    }
                    if (!field->isA(NodeTypeVarDecl)) {
                        std::cerr << "FIELD WAS NOT VAR DECL FAIL!!!!!!!!\n";
                        return;
                    }
                    vd = std::static_pointer_cast<LILVarDecl>(field);
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

void LILParameterSorter::_process(LILFlowControlCall * value)
{
    
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
