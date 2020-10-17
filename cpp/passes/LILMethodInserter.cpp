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
 *      This file inserts default methods into classes
 *
 ********************************************************************/

#include "LILMethodInserter.h"
#include "LILAssignment.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILPropertyName.h"
#include "LILSelector.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

using namespace LIL;

LILMethodInserter::LILMethodInserter()
{
}

LILMethodInserter::~LILMethodInserter()
{
}

void LILMethodInserter::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "====  METHOD INSERTING   ===\n";
        std::cerr << "============================\n\n";
    }
}

void LILMethodInserter::visit(LILNode *node)
{
    this->process(node);
}

void LILMethodInserter::process(LILNode * node)
{
    if (!node->isA(NodeTypeClassDecl)) {
        return;
    }
    
    if (this->getDebug()) {
        std::cerr << "## inserting methods " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    auto value = static_cast<LILClassDecl *>(node);
    
    if (value->getIsExtern()) {
        return;
    }
    
    std::vector<std::shared_ptr<LILNode>> nodes = value->getMethods();
    bool hasCtor = false;
    for (auto method : nodes) {
        auto vd = std::static_pointer_cast<LILVarDecl>(method);
        if (vd->getName() == "construct") {
            hasCtor = true;
            break;
        }
    }
    if (!hasCtor) {
        auto varWrapper = std::make_shared<LILVarDecl>();
        varWrapper->setName("construct");
        auto ctor = std::make_shared<LILFunctionDecl>();
        ctor->setFunctionDeclType(FunctionDeclTypeFn);
        ctor->setName("construct");
        ctor->setIsConstructor(true);
        auto fnTy = std::make_shared<LILFunctionType>();
        fnTy->setName("fn");
        varWrapper->setType(fnTy);
        varWrapper->setInitVal(ctor);
        value->addMethod(varWrapper);
    }
    
    bool hasDtor = false;
    for (auto method : nodes) {
        auto vd = std::static_pointer_cast<LILVarDecl>(method);
        if (vd->getName() == "destruct") {
            hasDtor = true;
            break;
        }
    }
    if (!hasDtor) {
        auto varWrapper = std::make_shared<LILVarDecl>();
        varWrapper->setName("destruct");
        auto dtor = std::make_shared<LILFunctionDecl>();
        dtor->setFunctionDeclType(FunctionDeclTypeFn);
        dtor->setName("destruct");
        auto fnTy = std::make_shared<LILFunctionType>();
        fnTy->setName("fn");
        varWrapper->setType(fnTy);
        varWrapper->setInitVal(dtor);
        value->addMethod(varWrapper);
    }
    
    for (auto field : value->getFields()) {
        if (field->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(field);
            if (vd->getIsIVar()) {
                //getter
                bool needsGetter = true;
                auto name = vd->getName();
                auto getter = this->_findMethod(true, value, name);
                std::shared_ptr<LILFunctionDecl> fd;
                if (getter) {
                    if (getter->isA(NodeTypeVarDecl)) {
                        auto initVal = std::static_pointer_cast<LILVarDecl>(getter)->getInitVal();
                        if ( initVal && initVal->isA(NodeTypeFunctionDecl)) {
                            fd = std::static_pointer_cast<LILFunctionDecl>(initVal);
                            auto retStatements = this->_findReturnStatements(fd->getBody());
                            if (retStatements.size() > 0) {
                                needsGetter = false;
                                for (auto retStatmt : retStatements) {
                                    auto retVal = std::static_pointer_cast<LILFlowControlCall>(retStatmt)->getArgument();
                                    if (retVal->isA(NodeTypeValuePath)) {
                                        auto vp = std::static_pointer_cast<LILValuePath>(retVal);
                                        vp->setPreventEmitCallToIVar(true);
                                    }
                                }
                            }
                        }
                    }
                    if (fd && needsGetter) {
                        auto returnStmt = std::make_shared<LILFlowControlCall>();
                        returnStmt->setFlowControlCallType(FlowControlCallTypeReturn);
                        auto newVp = std::make_shared<LILValuePath>();
                        auto selfSel = std::make_shared<LILSelector>();
                        selfSel->setSelectorType(SelectorTypeSelfSelector);
                        selfSel->setName("@self");
                        newVp->addChild(selfSel);
                        auto pn = std::make_shared<LILPropertyName>();
                        pn->setName(name);
                        newVp->addChild(pn);
                        newVp->setPreventEmitCallToIVar(true);
                        returnStmt->setArgument(newVp);
                        fd->addEvaluable(returnStmt);
                    }
                }
                //setter
                bool needsSetter = true;
                auto setter = this->_findMethod(false, value, name);
                if (setter) {
                    if (setter->isA(NodeTypeVarDecl)) {
                        auto initVal = std::static_pointer_cast<LILVarDecl>(setter)->getInitVal();
                        if ( initVal && initVal->isA(NodeTypeFunctionDecl)) {
                            fd = std::static_pointer_cast<LILFunctionDecl>(initVal);
                            auto setStatements = this->_findSetterStatements(name, fd->getBody());
                            if (setStatements.size() > 0) {
                                needsSetter = false;
                                for (auto setStatmt : setStatements) {
                                    auto asgmt = std::static_pointer_cast<LILAssignment>(setStatmt);
                                    auto asgmtSubj = asgmt->getSubject();
                                    auto subjVp = std::static_pointer_cast<LILValuePath>(asgmtSubj);
                                    subjVp->setPreventEmitCallToIVar(true);
                                }
                            }
                        }
                    }
                    if (fd && needsSetter) {
                        auto assignment = std::make_shared<LILAssignment>();
                        auto vp = std::make_shared<LILValuePath>();
                        auto selfSel = std::make_shared<LILSelector>();
                        selfSel->setSelectorType(SelectorTypeSelfSelector);
                        selfSel->setName("@self");
                        vp->addChild(selfSel);
                        auto pn = std::make_shared<LILPropertyName>();
                        pn->setName(name);
                        vp->addChild(pn);
                        vp->setPreventEmitCallToIVar(true);
                        
                        assignment->setSubject(vp);
                        
                        auto vp2 = std::make_shared<LILValuePath>();
                        auto vn = std::make_shared<LILVarName>();
                        auto ty = fd->getType();
                        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                        auto firstArg = fnTy->getArguments().front();
                        if (firstArg->isA(NodeTypeVarDecl)) {
                            auto vd = std::static_pointer_cast<LILVarDecl>(firstArg);
                            vn->setName(vd->getName());
                        }
                        vp2->addChild(vn);

                        assignment->setValue(vp2);

                        fd->addEvaluable(assignment);
                    }
                }
            }
        }
    }
    
}

std::shared_ptr<LILNode> LILMethodInserter::_findMethod(bool getter, LILClassDecl * value, LILString name)
{
    LILString compareName = (getter ? "get" : "set") + name.toUpperFirstCase();
    for (auto method : value->getMethods()) {
        if (method->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(method);
            if (vd->getName() == compareName) {
                return method;
            }
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<LILNode>> LILMethodInserter::_findReturnStatements(const std::vector<std::shared_ptr<LILNode>> & body)
{
    std::vector<std::shared_ptr<LILNode>> ret;
    for (auto node : body) {
        switch (node->getNodeType()) {
            case NodeTypeFlowControlCall:
            {
                if(node->isA(FlowControlCallTypeReturn)){
                    ret.push_back(node);
                    return ret;
                }
                break;
            }
                
            case NodeTypeFlowControl:
            {
                auto fc = std::static_pointer_cast<LILFlowControl>(node);
                if (fc->isA(FlowControlTypeIf)) {
                    auto retThen = this->_findReturnStatements(fc->getThen());
                    auto it = retThen.begin();
                    auto retElse = this->_findReturnStatements(fc->getElse());
                    retThen.insert(it+retThen.size(), retElse.begin(), retElse.end());
                    return retThen;
                    
                } else {
                    return this->_findReturnStatements(fc->getThen());
                }
            }
            default:
                break;
        }
    }
    return ret;
}

std::vector<std::shared_ptr<LILNode>> LILMethodInserter::_findSetterStatements(LILString name, const std::vector<std::shared_ptr<LILNode>> & body)
{
    std::vector<std::shared_ptr<LILNode>> ret;
    for (auto node : body) {
        switch (node->getNodeType()) {
            case NodeTypeAssignment:
            {
                auto asgmt = std::static_pointer_cast<LILAssignment>(node);
                auto subject = asgmt->getSubject();
                if (subject->isA(NodeTypeValuePath)) {
                    auto vp = std::static_pointer_cast<LILValuePath>(subject);
                    const auto & nodes = vp->getNodes();
                    auto firstNode = nodes.front();
                    if (firstNode && firstNode->isA(SelectorTypeSelfSelector)) {
                        if (nodes.size() == 2) {
                            auto secondNode = nodes[1];
                            if (secondNode && secondNode->isA(NodeTypePropertyName)) {
                                auto pn = std::static_pointer_cast<LILPropertyName>(secondNode);
                                if (pn->getName() == name) {
                                    ret.push_back(asgmt);
                                }
                            }
                        }
                    }
                }
                break;
            }
            case NodeTypeFlowControl:
            {
                auto fc = std::static_pointer_cast<LILFlowControl>(node);
                if (fc->isA(FlowControlTypeIf)) {
                    auto retThen = this->_findSetterStatements(name, fc->getThen());
                    for (const auto & rt : retThen) {
                        ret.push_back(rt);
                    }
                    auto retElse = this->_findSetterStatements(name, fc->getElse());
                    for (const auto & re : retElse) {
                        ret.push_back(re);
                    }

                } else {
                    const auto & retThen = this->_findSetterStatements(name, fc->getThen());
                    for (const auto & rt : retThen) {
                        ret.push_back(rt);
                    }
                }
            }

            default:
                break;
        }
    }
    return ret;
}
