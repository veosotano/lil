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
 *      This file resolves type aliases and type definitions
 *
 ********************************************************************/

#include "LILTypeResolver.h"
#include "LILAliasDecl.h"
#include "LILNodeToString.h"
#include "LILObjectType.h"
#include "LILSIMDType.h"
#include "LILStaticArrayType.h"
#include "LILTypeDecl.h"
#include "LILVarNode.h"

using namespace LIL;

LILTypeResolver::LILTypeResolver()
{
}

LILTypeResolver::~LILTypeResolver()
{
}

void LILTypeResolver::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  TYPE RESOLVING  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILTypeResolver::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    auto nodes = rootNode->getNodes();
    for (const auto & node : nodes) {
        this->process(node);
    }
}

void LILTypeResolver::process(std::shared_ptr<LILNode> node)
{
    if (node->isA(NodeTypeClassDecl)) {
        this->enterClassContext(std::static_pointer_cast<LILClassDecl>(node));
    }
    for (auto childNode : node->getChildNodes()) {
        this->process(childNode);
    }
    if (node->isA(NodeTypeClassDecl)) {
        this->exitClassContext();
    }
    if (node->isA(NodeTypeFunctionDecl)) {
        auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
        auto fnTy = fd->getFnType();
        auto newTy = this->_process(fnTy);
        if (newTy) {
            fd->setType(fnTy);
        }
    }
    if (node->isA(FlowControlTypeIfCast)) {
        auto fc = std::static_pointer_cast<LILFlowControl>(node);
        auto args = fc->getArguments();
        if (args.size() != 2) {
            std::cerr << "IF CAST BLOCK DID NOT HAVE 2 ARGUMENTS FAIL!!!!!\n";
            return;
        }
        auto lastArg = args.back();
        if (!lastArg->isA(NodeTypeType)) {
            std::cerr << "IF CAST BLOCK HAD NO TYPE ARG FAIL!!!!!\n";
            return;
        }
        auto ty = std::static_pointer_cast<LILType>(lastArg);
        auto newTy = this->_process(ty);
        if (newTy) {
            fc->clearArguments();
            fc->addArgument(args.front());
            fc->addArgument(newTy);
        }
        return;
    }
    if (node->isA(ExpressionTypeCast)) {
        auto exp = std::static_pointer_cast<LILExpression>(node);
        auto right = exp->getRight();
        if (!right->isA(NodeTypeType)) {
            std::cerr << "RIGHT NODE OF CAST WAS NOT TYPE FAIL!!!!!\n";
            return;
        }
        auto ty = std::static_pointer_cast<LILType>(right);
        auto newTy = this->_process(ty);
        if (newTy) {
            exp->setRight(newTy);
        }
        return;
    }
    
    if (!node->isA(NodeTypeType)) {
        auto typedNode = std::dynamic_pointer_cast<LILTypedNode>(node);
        if (typedNode) {
            auto ty = node->getType();
            if (ty) {
                auto newTy = this->_process(ty);
                if (newTy) {
                    typedNode->setType(newTy);
                }
            }
        }
    }
}

std::shared_ptr<LILType> LILTypeResolver::_process(std::shared_ptr<LILType> value)
{
    std::shared_ptr<LILType> ret = nullptr;
    switch (value->getTypeType()) {
        case TypeTypePointer:
        {
            auto ptrTy = std::static_pointer_cast<LILPointerType>(value);
            auto arg = ptrTy->getArgument();
            if (arg) {
                auto newTy = this->_process(arg);
                if (newTy) {
                    ptrTy->setArgument(newTy);
                    ret = ptrTy;
                }
            }
            break;
        }
        case TypeTypeMultiple:
        {
            auto multiTy = std::static_pointer_cast<LILMultipleType>(value);
            std::vector<std::shared_ptr<LILType>> tys;
            bool changed = false;
            for (auto childTy : multiTy->getTypes()) {
                auto newTy = this->_process(childTy);
                if (newTy) {
                    changed = true;
                    tys.push_back(newTy);
                } else {
                    tys.push_back(childTy);
                }
            }
            if (changed) {
                multiTy->setTypes(std::move(tys));
                ret = multiTy;
            }
            multiTy->sortTypes();
            break;
        }
        case TypeTypeFunction:
        {
            auto fnTy = std::static_pointer_cast<LILFunctionType>(value);
            std::vector<std::shared_ptr<LILNode>> newArgs;
            bool changed = false;
            for (auto childNode : fnTy->getArguments()) {
                std::shared_ptr<LILType> ty;
                if (childNode->isA(NodeTypeVarDecl)) {
                    auto vd = std::static_pointer_cast<LILVarDecl>(childNode);
                    ty = vd->getType();
                    auto newTy = this->_process(ty);
                    if (newTy) {
                        vd->setType(newTy);
                        changed = true;
                    }
                    newArgs.push_back(vd);
                } else if (childNode->isA(NodeTypeType)) {
                    ty = std::static_pointer_cast<LILType>(childNode);
                    auto newTy = this->_process(ty);
                    if (newTy) {
                        newArgs.push_back(newTy);
                        changed = true;
                    } else {
                        newArgs.push_back(ty);
                    }
                }
            }
            if (changed) {
                fnTy->setArguments(newArgs);
            }
            auto retTy = fnTy->getReturnType();
            if (retTy) {
                auto newRetTy = this->_process(retTy);
                if (newRetTy) {
                    fnTy->setReturnType(newRetTy);
                }
            }
            if (changed) {
                ret = fnTy;
            }
            break;
        }
        case TypeTypeSingle:
        {
            if (!LILType::isBuiltInType(value.get())) {
                auto cd = this->getClassContext();
                if (cd) {
                    for (auto alias : cd->getAliases()) {
                        auto aTy = alias->getSrcType();
                        if (aTy->equalTo(value)) {
                            auto dstTy = alias->getDstType();
                            auto newTy = this->_process(dstTy);
                            if (newTy) {
                                ret = newTy;
                            } else {
                                ret = dstTy->clone();
                            }
                            break;
                        }
                    }
                }
                if (!ret) {
                    const auto & rootNode = this->getRootNode();
                    auto aliases = rootNode->getAliases();
                    for (auto alias : aliases) {
                        auto aTy = alias->getSrcType();
                        if (aTy->equalTo(value)) {
                            auto dstTy = alias->getDstType();
                            auto newTy = this->_process(dstTy);
                            if (newTy) {
                                ret = newTy;
                            } else {
                                ret = dstTy->clone();
                            }
                            break;
                        }
                    }
                }
                if (!ret) {
                    const auto & rootNode = this->getRootNode();
                    auto typeDecls = rootNode->getTypes();
                    for (auto typeDecl : typeDecls) {
                        auto tTy = typeDecl->getSrcType();
                        if (tTy->equalTo(value)) {
                            auto dstTy = typeDecl->getDstType();
                            auto newTy = this->_process(dstTy);
                            if (newTy) {
                                ret = newTy;
                            } else {
                                ret = dstTy->clone();
                            }
                            ret->setStrongTypeName(value->getName());
                            break;
                        }
                    }
                }
            }
            break;
        }
            
        case TypeTypeObject:
        {
            auto cd = this->getClassContext();
            if (cd) {
                for (auto alias : cd->getAliases()) {
                    auto aTy = alias->getSrcType();
                    if (aTy->equalTo(value)) {
                        auto dstTy = alias->getDstType();
                        auto newTy = this->_process(dstTy);
                        if (newTy) {
                            ret = newTy;
                        } else {
                            ret = dstTy->clone();
                        }
                        break;
                    }
                }
            }
            if (!ret) {
                const auto & rootNode = this->getRootNode();
                auto aliases = rootNode->getAliases();
                for (auto alias : aliases) {
                    auto aTy = alias->getSrcType();
                    if (aTy->equalTo(value)) {
                        auto dstTy = alias->getDstType();
                        auto newTy = this->_process(dstTy);
                        if (newTy) {
                            ret = newTy;
                        } else {
                            ret = dstTy->clone();
                        }
                        break;
                    }
                }
            }
            if (!ret) {
                const auto & rootNode = this->getRootNode();
                auto typeDecls = rootNode->getTypes();
                for (auto typeDecl : typeDecls) {
                    auto tTy = typeDecl->getSrcType();
                    if (tTy->equalTo(value)) {
                        auto dstTy = typeDecl->getDstType();
                        auto newTy = this->_process(dstTy);
                        if (newTy) {
                            ret = newTy;
                        } else {
                            ret = dstTy->clone();
                        }
                        ret->setStrongTypeName(value->getName());
                        break;
                    }
                }
            }
            if (ret) {
                auto objTy = std::static_pointer_cast<LILObjectType>(value);
                if (objTy->getTmplParams().size() > 0) {
                    auto retObjTy = std::static_pointer_cast<LILObjectType>(ret);
                    for (auto pTy : objTy->getTmplParams()) {
                        retObjTy->addTmplParam(pTy->clone());
                    }
                }
            }
            break;
        }
        case TypeTypeStaticArray:
        {
            auto saTy = std::static_pointer_cast<LILStaticArrayType>(value);
            auto saChildTy = saTy->getType();
            if (saChildTy) {
                auto processedSaChildTy = this->_process(saChildTy);
                if (processedSaChildTy) {
                    saTy->setType(processedSaChildTy);
                }
            }
            break;
        }
        case TypeTypeSIMD:
        {
            auto simdTy = std::static_pointer_cast<LILSIMDType>(value);
            auto innerTy = simdTy->getType();
            if (this->_process(innerTy)) {
                simdTy->setType(innerTy);
            }
            break;
        }
        case TypeTypeNone:
            //do nothing
            break;
    }
    return ret;
}

std::shared_ptr<LILClassDecl> LILTypeResolver::getClassContext() const
{
    if (this->_classContext.size() > 0) {
        return this->_classContext.back();
    }
    return nullptr;
}

void LILTypeResolver::enterClassContext(std::shared_ptr<LILClassDecl> value)
{
    this->_classContext.push_back(value);
}

void LILTypeResolver::exitClassContext()
{
    this->_classContext.pop_back();
}
