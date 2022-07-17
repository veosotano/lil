
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
 *      This file adds intermediate parts of paths that come from
 *      expanded fields of classes
 *
 ********************************************************************/

#include "LILPathExpander.h"
#include "LILAssignment.h"
#include "LILClassDecl.h"
#include "LILFlowControl.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILObjectDefinition.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILRootNode.h"
#include "LILRule.h"
#include "LILStaticArrayType.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

using namespace LIL;

LILPathExpander::LILPathExpander()
{
}

LILPathExpander::~LILPathExpander()
{
}

void LILPathExpander::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  PATH EXPANDING  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILPathExpander::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);

    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        this->process(node.get());
    }
}

void LILPathExpander::process(LILNode * node)
{
    for (auto childNode : node->getChildNodes()) {
        this->process(childNode.get());
    }
    switch (node->getNodeType()) {
        case NodeTypeValuePath:
        {
            this->_process(static_cast<LILValuePath *>(node));
            break;
        }
        case NodeTypeObjectDefinition:
        {
            this->_process(static_cast<LILObjectDefinition *>(node));
            break;
        }
        case NodeTypeFlowControl:
        {
            auto flc = static_cast<LILFlowControl *>(node);
            if (flc->getFlowControlType() == FlowControlTypeIfCast) {
                this->inhibitSearchingForIfCastType = true;
                for (auto arg : flc->getArguments()) {
                    this->process(arg.get());
                }
                this->inhibitSearchingForIfCastType = false;
            }
            break;
        }
        case NodeTypeRule:
        {
            auto rule = static_cast<LILRule *>(node);
            this->_process(rule);
            break;
        }
        default:
            break;
    }
}

void LILPathExpander::_process(LILValuePath * vp)
{
    bool hasChanges = false;
    std::deque<std::shared_ptr<LILNode>> tempNodes;
    std::vector<std::shared_ptr<LILNode>> newNodes;
    auto nodes = vp->getNodes();
    std::shared_ptr<LILNode> firstNode;
    std::shared_ptr<LILType> currentTy;
    
    size_t startIndex = 1;
    std::shared_ptr<LILType> ifCastType;
    if (!this->inhibitSearchingForIfCastType) {
        ifCastType = this->findIfCastType(vp, startIndex);
    }
    if (ifCastType) {
        currentTy = ifCastType;
        if (startIndex == nodes.size()) {
            return;
        }
    } else {
        firstNode = nodes.front();
        if (firstNode->isA(NodeTypeVarName)) {
            auto remoteNode = this->recursiveFindNode(firstNode);
            std::shared_ptr<LILType> subjTy;
            if (remoteNode) {
                if (remoteNode->getNodeType() == NodeTypeEnum) {
                    return;
                }
                subjTy = remoteNode->getType();
                if (!subjTy && remoteNode->isA(NodeTypeVarDecl)) {
                    auto vd = std::static_pointer_cast<LILVarDecl>(remoteNode);
                    auto initVal = vd->getInitVal();
                    subjTy = initVal->getType();
                }
            }
            if (subjTy) {
                currentTy = subjTy;
            } else {
                std::cerr << "SUBJ TY WAS NULL FAIL!!!!\n";
                return;
            }
        } else if (firstNode->isA(NodeTypePropertyName)) {
            auto parentNode = vp->getParentNode();
            if (parentNode->isA(NodeTypeAssignment)) {
                auto grandpaNode = parentNode->getParentNode();
                if (grandpaNode->isA(NodeTypeObjectDefinition)) {
                    auto objDef = std::static_pointer_cast<LILObjectDefinition>(grandpaNode);
                    currentTy = objDef->getType();
                    startIndex = 0;
                }
            }
            if (!currentTy) {
                std::cerr << "COULD NOT FIND TYPE FOR PROPERTY NAME FAIL!!!!\n";
                return;
            }
        }
        else if (firstNode->isA(SelectorTypeSelfSelector)) {
            auto classDecl = this->findAncestorClass(firstNode);
            currentTy = classDecl->getType();
        }
        else if (firstNode->isA(SelectorTypeThisSelector)) {
            auto rule = this->findAncestorRule(firstNode);
            if (rule) {
                currentTy = rule->getType();
            }
            if (!currentTy) {
                std::cerr << "RULE HAD NO TYPE FAIL!!!!\n";
                return;
            }
        }
    }
    for (size_t i = 0; i<startIndex; ++i) {
        newNodes.push_back(nodes[i]->clone());
    }
    bool isLast = false;
    for (size_t i=startIndex, j=nodes.size(); i<j; ++i) {
        isLast = i==j-1;
        auto node = nodes[i];
        switch (node->getNodeType()) {
            case NodeTypePropertyName:
            {
                auto pn = std::static_pointer_cast<LILPropertyName>(node);
                auto pnName = pn->getName();
                if (currentTy->isA(TypeTypePointer)) {
                    auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
                    currentTy = ptrTy->getArgument();
                }
                if (!currentTy->isA(TypeTypeObject)) {
                    std::cerr << "CURRENT TY WAS NOT OBJECT TY FAIL!!!!\n";
                    return;
                }
                auto className = currentTy->getName().data();
                auto classDecl = this->findClassWithName(className);
                if (!classDecl) {
                    std::cerr << "CLASS "+className+" NOT FOUND FAIL!!!!\n";
                    return;
                }
                auto field = classDecl->getFieldNamed(pnName);
                if (!field) {
                    field = this->_addExpandedFields(tempNodes, classDecl, pnName, hasChanges, false);
                    if (field) {
                        classDecl = this->findAncestorClass(field);
                        for (auto tempNode : tempNodes) {
                            newNodes.push_back(tempNode->clone());
                        }
                        hasChanges = true;
                    }
                    tempNodes.clear();
                }
                if (!field) {
                    std::cerr << "FIELD WAS NULL FAIL!!!!\n";
                    return;
                }
                if (field->getNodeType() == NodeTypeVarDecl) {
                    auto vd = std::static_pointer_cast<LILVarDecl>(field);
                    if (vd->getIsVVar()) {
                        auto retTy = vd->getReturnType();
                        if (retTy) {
                            currentTy = retTy;
                            break;
                        }
                    }
                }
                currentTy = field->getType();
                newNodes.push_back(node->clone());
                break;
            }
            case NodeTypeFunctionCall:
            {
                auto fc = std::static_pointer_cast<LILFunctionCall>(node);
                if (currentTy->isA(TypeTypePointer)) {
                    auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
                    currentTy = ptrTy->getArgument();
                }
                auto className = currentTy->getName().data();
                if (!currentTy->isA(TypeTypeObject)) {
                    std::cerr << "TYPE " << className << " IS NOT OBJECT TYPE FAIL!!!!\n";
                    return;
                }
                auto classDecl = this->findClassWithName(className);
                if (!classDecl) {
                    std::cerr << "CLASS " << className << " NOT FOUND FAIL!!!!\n";
                    return;
                }
                auto methodName = fc->getName();
                auto methodNode = classDecl->getMethodNamed(methodName);
                if (!methodNode) {
                    methodNode = this->_addExpandedFields(tempNodes, classDecl, methodName, hasChanges, true);
                    if (methodNode) {
                        classDecl = this->findAncestorClass(methodNode);
                        for (auto tempNode : tempNodes) {
                            newNodes.push_back(tempNode->clone());
                        }
                        hasChanges = true;
                    }
                }
                if (!methodNode) {
                    std::cerr << "METHOD " << methodName.data() << " NOT FOUND FAIL!!!!\n";
                    return;
                }
                if (!methodNode->isA(NodeTypeFunctionDecl)) {
                    std::cerr << "METHOD NODE IS NOT FUNCTION DECL FAIL!!!!\n";
                    return;
                }
                auto method = std::static_pointer_cast<LILFunctionDecl>(methodNode);
                auto fnTy = method->getFnType();
                auto retTy = fnTy->getReturnType();
                if (!isLast) {
                    if (retTy) {
                        currentTy = retTy;
                    } else {
                        std::cerr << "FN HAD NO RETURN TYPE FAIL!!!!!!\n";
                        return;
                    }
                }
                newNodes.push_back(node->clone());
                break;
            }
            case NodeTypeIndexAccessor:
            {
                if (currentTy->isA(TypeTypeObject)) {
                    const auto & className = currentTy->getName();
                    auto cd = this->findClassWithName(className);
                    if (!cd) {
                        std::cerr << "CLASS " + className.data() + " NOT FOUND FAIL!!!!\n";
                        return;
                    }
                    auto method = cd->getMethodNamed("at");
                    if (!method) {
                        std::cerr << "CLASS " + className.data() + " HAD NOT at METHOD FAIL!!!!\n";
                        return;
                    }
                    auto methodTy = method->getType();
                    if (!methodTy || !methodTy->isA(TypeTypeFunction)) {
                        std::cerr << "BAD AT METHOD FAIL!!!!\n";
                        return;
                    }
                    auto mFnTy = std::static_pointer_cast<LILFunctionType>(methodTy);
                    auto retTy = mFnTy->getReturnType();
                    if (!retTy) {
                        std::cerr << "FN TYPE HAD NO RETURN TY FAIL!!!!\n";
                        return;
                    }
                    currentTy = retTy;
                    newNodes.push_back(node->clone());
                }
                else if (currentTy->isA(TypeTypeStaticArray))
                {
                    auto saTy = std::static_pointer_cast<LILStaticArrayType>(currentTy);
                    currentTy = saTy->getType();
                    newNodes.push_back(node->clone());
                }
                else
                {
                    std::cerr << "FIELD TYPE IS NOT ARRAY TYPE FAIL!!!!\n";
                    return;
                }
                break;
            }
                
            default:
                std::cerr << "!!!!!!!!!!VALUE PATH NODE TYPE FAIL!!!!!!!!!!!!!!!!\n";
                break;
        }
    }
    
    if (hasChanges) {
        vp->setNodes(std::move(newNodes));
    }
}

std::shared_ptr<LILNode> LILPathExpander::_addExpandedFields(std::deque<std::shared_ptr<LILNode>> &newNodes, std::shared_ptr<LILClassDecl> classDecl, const LILString & pnName, bool & hasChanges, bool isMethod)
{
    std::shared_ptr<LILNode> ret = nullptr;
    bool found = false;
    for (auto field : classDecl->getFields()) {
        if (!field->isA(NodeTypeVarDecl)) {
            continue;
        }
        auto vd = std::static_pointer_cast<LILVarDecl>(field);
        if (vd->getIsExpanded()) {
            auto vdTy = vd->getType();
            if (vdTy && vdTy->isA(TypeTypeObject)) {
                auto expClassDecl = this->findClassWithName(vdTy->getName());
                if (!found) {
                    if (isMethod) {
                        ret = expClassDecl->getMethodNamed(pnName);
                    } else {
                        ret = expClassDecl->getFieldNamed(pnName);
                    }
                    if (!ret) {
                        ret = this->_addExpandedFields(newNodes, expClassDecl, pnName, hasChanges);
                    }
                    if (ret) {
                        found = true;
                        auto newPn = std::make_shared<LILPropertyName>();
                        newPn->setName(vd->getName());
                        newNodes.push_front(newPn);
                    }
                } else {
                    std::cerr << "EXPANDED FIELD CLASH FAIL!!!!!!!!!!!!!!!!\n";
                    return nullptr;
                }
            }
        }
    }
    return ret;
}

void LILPathExpander::_process(LILObjectDefinition * objdef)
{
    LILString classNameStr = objdef->getType()->getName();
    std::string className = classNameStr.data();
    auto classDecl = this->findClassWithName(classNameStr);
    
    for (auto node : objdef->getNodes()) {
        if (node->isA(NodeTypeAssignment)) {
            auto as = std::static_pointer_cast<LILAssignment>(node);
            auto subj = as->getSubject();
            if (subj->isA(NodeTypeValuePath)) {
                std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
                return;
            } else if (subj->isA(NodeTypePropertyName)) {
                auto pnName = std::static_pointer_cast<LILPropertyName>(subj)->getName();
                auto field = classDecl->getFieldNamed(pnName);
                if (!field) {
                    auto newVp = std::make_shared<LILValuePath>();
                    std::deque<std::shared_ptr<LILNode>> tempNodes;
                    bool hasChanges = false;

                    field = this->_addExpandedFields(tempNodes, classDecl, pnName, hasChanges);
                    if (field) {
                        for (auto tempNode : tempNodes) {
                            newVp->addChild(tempNode->clone());
                        }
                        newVp->addChild(subj->clone());
                        as->setSubject(newVp);
                    }
                }
            }
            
        } else {
            std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return;
        }
    }
}

void LILPathExpander::_process(LILRule * rule)
{
    this->_processRuleInner(rule);
    for (const auto & child : rule->getChildRules()) {
        this->_process(child.get());
    }
}

void LILPathExpander::_processRuleInner(LILRule * rule)
{
    auto ty = rule->getType();
    if (!ty || (ty->getTypeType() != TypeTypeObject)) {
        return;
    }
    auto classDecl = this->findClassWithName(ty->getName());
    if (!classDecl) {
        return;
    }
    for (auto node : rule->getValues()) {
        if (node->getNodeType() != NodeTypeAssignment) {
            continue;
        }
        auto as = std::static_pointer_cast<LILAssignment>(node);
        auto subjNode = as->getSubject();
        auto valueNode = as->getValue();
        if (subjNode->getNodeType() == NodeTypePropertyName) {
            auto pn = std::static_pointer_cast<LILPropertyName>(subjNode);
            auto pnName = pn->getName();
            auto field = classDecl->getFieldNamed(pnName);
            if (!field) {
                auto newVp = std::make_shared<LILValuePath>();
                std::deque<std::shared_ptr<LILNode>> tempNodes;
                bool hasChanges = false;
                
                field = this->_addExpandedFields(tempNodes, classDecl, pnName, hasChanges);
                if (field) {
                    for (auto tempNode : tempNodes) {
                        newVp->addChild(tempNode->clone());
                    }
                    newVp->addChild(subjNode->clone());
                    as->setSubject(newVp);
                }
            }
        } else if (subjNode->getNodeType() == NodeTypeValuePath) {
            auto vp = std::static_pointer_cast<LILValuePath>(subjNode);
            this->_process(vp.get());
        }
    }
}
