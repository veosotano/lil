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
 *      This file makes concrete classes when using class parameters
 *
 ********************************************************************/

#include "LILClassTemplateLowerer.h"
#include "LILAssignment.h"
#include "LILClassDecl.h"
#include "LILFlowControl.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILMultipleType.h"
#include "LILNodeToString.h"
#include "LILObjectType.h"
#include "LILPointerType.h"
#include "LILStaticArrayType.h"
#include "LILTypeDecl.h"
#include "LILValueList.h"
#include "LILVarDecl.h"

using namespace LIL;

LILClassTemplateLowerer::LILClassTemplateLowerer()
{
}

LILClassTemplateLowerer::~LILClassTemplateLowerer()
{
}

void LILClassTemplateLowerer::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "== CLASS TEMPLATE LOWERING =\n";
        std::cerr << "============================\n\n";
    }
}

void LILClassTemplateLowerer::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        if (node->isA(NodeTypeClassDecl)) {
            auto cd = std::static_pointer_cast<LILClassDecl>(node);
            if (cd->isTemplate()) {
                auto ty = std::static_pointer_cast<LILObjectType>(node->getType());

                std::vector<std::pair<std::shared_ptr<LILType>, std::shared_ptr<LILClassDecl>>> newClasses;

                std::vector<std::shared_ptr<LILNode>> specializations;
                if (ty->getName() == "array") {
                    specializations = findArraySpecializations(nodes);
                } else {
                    specializations = this->findClassSpecializations(nodes, ty);
                }
                if (specializations.size() > 0) {
                    for (auto spNode : specializations) {
                        std::shared_ptr<LILType> spTy;
                        if (spNode->isA(NodeTypeValueList)) {
                            std::shared_ptr<LILType> parentTy;
                            auto parent = spNode->getParentNode();
                            if (parent) {
                                parentTy = parent->getType();
                            }
                            if (parentTy) {
                                spTy = parentTy;
                            } else {
                                std::shared_ptr<LILType> vlTy;
                                auto vl = std::static_pointer_cast<LILValueList>(spNode);
                                vlTy = vl->getType();
                                if (!vlTy) {
                                    vlTy = this->findTypeForValueList(vl.get());
                                    if (vlTy->getIsWeakType()) {
                                        auto intType = vlTy->getDefaultType();
                                        vlTy = intType;
                                    }
                                    if (vlTy) {
                                        spTy = LILObjectType::make("array");
                                        spTy->addTmplParam(vlTy);
                                    } else {
                                        std::cerr << "COULD NOT MAKE SPECIALIZATION TYPE FROM VALUE LIST FAIL !!!!!!! \n\n";
                                    }
                                }
                            }
                        } else {
                            spTy = spNode->getType();
                        }
                        if (!spTy) {
                            std::cerr << "COULD NOT FIND SPECIALIZATION TYPE FAIL !!!!!!! \n\n";
                            continue;
                        }
                        std::shared_ptr<LILClassDecl> newClass;
                        bool found = false;
                        for (auto item : newClasses) {
                            auto itemTy = item.first;
                            if (itemTy->equalTo(spTy)) {
                                found = true;
                                newClass = item.second;
                                break;
                            }
                        }
                        if (!found) {
                            newClass = this->makeSpecializedClass(cd, spTy);
                            if (newClass) {
                                newClasses.push_back(std::make_pair(spTy, newClass));
                            }
                        }
                        if (newClass) {
                            if (spNode->isTypedNode()) {
                                auto spTyNode = std::static_pointer_cast<LILTypedNode>(spNode);
                                spTyNode->setType(newClass->getType()->clone());
                            }
                        }
                    }
                    //out with the old
                    rootNode->removeClass(std::static_pointer_cast<LILClassDecl>(node));
                    rootNode->removeNode(node);
                    //in with the new
                    for (auto newCdPair : newClasses) {
                        auto newCd = newCdPair.second;
                        resultNodes.push_back(newCd);
                        rootNode->addClass(newCd);
                    }
                } else {
                    resultNodes.push_back(node);
                }
                
            } else {
                resultNodes.push_back(node);
            }
        } else {
            resultNodes.push_back(node);
        }
    }
    rootNode->setChildNodes(std::move(resultNodes));
}

std::vector<std::shared_ptr<LILNode>> LILClassTemplateLowerer::findClassSpecializations(const std::vector<std::shared_ptr<LILNode>> & nodes, const std::shared_ptr<LILType> & ty) const
{
    std::vector<std::shared_ptr<LILNode>> ret;
    for (auto node : nodes) {
        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            auto vdTy = vd->getType();
            
            if (
                vdTy
                && vdTy->getName() == ty->getName()
            ) {
                ret.push_back(node);
            }
        }

        const auto & childNodes = node->getChildNodes();
        if (childNodes.size() > 0) {
            auto childRet = this->findClassSpecializations(childNodes, ty);
            if (childRet.size() > 0) {
                ret.insert(ret.end(), childRet.begin(), childRet.end());
            }
        }
        if (node->isA(NodeTypeObjectDefinition)) {
            auto objDefTy = node->getType();
            if (
                objDefTy
                && objDefTy->getName() == ty->getName()
            ) {
                ret.push_back(node);
            }
        }
    }
    return ret;
}

std::vector<std::shared_ptr<LILNode>> LILClassTemplateLowerer::findArraySpecializations(const std::vector<std::shared_ptr<LILNode>> & nodes) const
{
    std::vector<std::shared_ptr<LILNode>> ret;
    for (auto node : nodes) {
        const auto & childNodes = node->getChildNodes();
        if (childNodes.size() > 0) {
            auto childRet = this->findArraySpecializations(childNodes);
            if (childRet.size() > 0) {
                ret.insert(ret.end(), childRet.begin(), childRet.end());
            }
        }
        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            auto vdTy = vd->getType();
            
            if (vdTy && vdTy->getName() == "array") {
                ret.push_back(node);
            }
            if (!vdTy) {
                auto initVal = vd->getInitVal();
                if (initVal) {
                    vdTy = initVal->getType();
                    if ( vdTy && vdTy->getName() == "array") {
                        ret.push_back(initVal);
                    }
                }
            }
        } else if (node->isA(NodeTypeObjectDefinition)) {
            auto objDefParent = node->getParentNode();
            if (objDefParent && objDefParent->isA(NodeTypeVarDecl)) {
                //it will get added when reaching the var decl as the init val
                continue;
            }
            auto objDefTy = node->getType();
            if ( objDefTy && objDefTy->getName() == "array" ) {
                ret.push_back(node);
            }
        } else if (node->isA(NodeTypeValueList)) {
            auto vl = std::static_pointer_cast<LILValueList>(node);
            if (vl->getValues().size() == 0) {
                break;
            }
            auto vlParent = vl->getParentNode();
            switch (vlParent->getNodeType()) {
                case NodeTypeVarDecl:
                {
                    auto vlTy = vlParent->getType();
                    if (vlTy) {
                        if (vlTy->getName() == "array") {
                            ret.push_back(node);
                        }
                    } else {
                        ret.push_back(node);
                    }
                    break;
                }
                    
                case NodeTypeAssignment:
                {
                    break;
                }

                default:
                    std::cerr << "UNKNOWN NODE TYPE WHILE FINDING ARRAY SPECIALIZATION FAIL !!!!!\n\n";
                    return ret;
            }
        }
    }
    return ret;
}

std::shared_ptr<LILClassDecl> LILClassTemplateLowerer::makeSpecializedClass(std::shared_ptr<LILClassDecl> cd, std::shared_ptr<LILType> specializedType) const
{
    const auto & paramTys = cd->getTmplParams();
    const auto & specializedParamTys = specializedType->getTmplParams();
    auto ret = cd->clone();
    auto newSpTy = specializedType->clone();
    ret->clearTmplParams();
    for (size_t i=0, j=paramTys.size(); i<j; i+=1) {
        auto cdParamNode = paramTys.at(i);
        if (cdParamNode->isA(NodeTypeTypeDecl)) {
            auto cdParamTyDecl = std::static_pointer_cast<LILTypeDecl>(cdParamNode);
            auto cdParamTy = cdParamTyDecl->getSrcType();
            
            std::shared_ptr<LILType> spParamTy;
            if (specializedParamTys.size() > i) {
                spParamTy = std::static_pointer_cast<LILType>(specializedParamTys.at(i));
            } else {
                spParamTy = cdParamTyDecl->getDstType();
                newSpTy->addTmplParam(spParamTy);
            }
            this->replaceTypeWithSpecializedType(ret->getChildNodes(), cdParamTy, spParamTy);

        } else {
            std::cerr << "UNIMPLEMENTED FAIL !!!! !! ! \n\n";
            return ret;
        }
    }
    LILString newName = "lil_"+this->typeToString(newSpTy);
    auto newObjType = LILObjectType::make(newName);
    ret->setType(newObjType);
    return ret;
}

void LILClassTemplateLowerer::replaceTypeWithSpecializedType(const std::vector<std::shared_ptr<LILNode>> & nodes, std::shared_ptr<LILType> templateType, std::shared_ptr<LILType> specializedType) const
{
    for (auto node : nodes) {
        const auto & childNodes = node->getChildNodes();
        if (childNodes.size() > 0) {
            this->replaceTypeWithSpecializedType(childNodes, templateType, specializedType);
        }
        if (node->isTypedNode()) {
            auto tyNode = std::static_pointer_cast<LILTypedNode>(node);
            auto nodeTy = tyNode->getType();
            if (nodeTy) {
                auto replacementTy = this->replaceType(nodeTy, templateType, specializedType);
                if (replacementTy) {
                    tyNode->setType(replacementTy);
                }
            }
        } else if (node->isA(NodeTypeFunctionDecl)) {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
            auto fnTy = fd->getFnType();
            std::vector<std::shared_ptr<LILNode>> newArgs;
            bool argsChanges = false;
            for (auto arg : fnTy->getArguments()) {
                if (arg->isA(NodeTypeType)) {
                    auto replacementTy = this->replaceType(std::static_pointer_cast<LILType>(arg), templateType, specializedType);
                    if (replacementTy) {
                        newArgs.push_back(replacementTy);
                        argsChanges = true;
                    } else {
                        newArgs.push_back(arg);
                    }
                } else if (arg->isTypedNode()) {
                    auto typedArgNode = std::static_pointer_cast<LILTypedNode>(arg);
                    auto argTy = arg->getType();
                    auto replacementTy = this->replaceType(argTy, templateType, specializedType);
                    if (replacementTy) {
                        argsChanges = true;
                        typedArgNode->setType(replacementTy);
                    }
                    newArgs.push_back(arg);
                }
            }
            if (argsChanges) {
                fnTy->setArguments(newArgs);
            }
            auto returnTy = fnTy->getReturnType();
            if (returnTy) {
                auto replacementReturnTy = this->replaceType(returnTy, templateType, specializedType);
                if (replacementReturnTy) {
                    fnTy->setReturnType(replacementReturnTy);
                }
            }

        } else if (node->isA(NodeTypeTypeDecl)){
            auto td = std::static_pointer_cast<LILTypeDecl>(node);
            auto srcTy = td->getSrcType();
            if (srcTy) {
                auto replacementSrcTy = this->replaceType(srcTy, templateType, specializedType);
                if (replacementSrcTy) {
                    td->setSrcType(replacementSrcTy);
                }
            }
            auto dstTy = td->getDstType();
            if (dstTy) {
                auto replacementDstTy = this->replaceType(dstTy, templateType, specializedType);
                if (replacementDstTy) {
                    td->setDstType(replacementDstTy);
                }
            }
        } else if (node->isA(NodeTypeFlowControl)) {
            auto fc = std::static_pointer_cast<LILFlowControl>(node);
            std::vector<std::shared_ptr<LILNode>> newArgs;
            bool argsChanges = false;
            for (auto arg : fc->getArguments()) {
                if (arg->isA(NodeTypeType)) {
                    auto replacementTy = this->replaceType(std::static_pointer_cast<LILType>(arg), templateType, specializedType);
                    if (replacementTy) {
                        newArgs.push_back(replacementTy);
                        argsChanges = true;
                    } else {
                        newArgs.push_back(arg);
                    }
                } else if (arg->isTypedNode()) {
                    auto typedArgNode = std::static_pointer_cast<LILTypedNode>(arg);
                    auto argTy = arg->getType();
                    auto replacementTy = this->replaceType(std::static_pointer_cast<LILType>(arg), templateType, specializedType);
                    if (replacementTy) {
                        argsChanges = true;
                        typedArgNode->setType(replacementTy);
                    }
                    newArgs.push_back(arg);
                }
            }
            if (argsChanges) {
                fc->setArguments(std::move(newArgs));
            }
        }
    }
}

std::shared_ptr<LILType> LILClassTemplateLowerer::replaceType(std::shared_ptr<LILType> sourceTy, std::shared_ptr<LILType> templateTy, std::shared_ptr<LILType> specializedTy) const
{
    std::shared_ptr<LILType> ret = nullptr;
    switch (sourceTy->getTypeType()) {
        case TypeTypeSingle:
        case TypeTypeObject:
        {
            if (!sourceTy) {
                break;
            }
            if (sourceTy->equalTo(templateTy)) {
                ret = specializedTy->clone();
            } else {
                const auto & tmplParams = sourceTy->getTmplParams();
                if (tmplParams.size() > 0) {
                    std::vector<std::shared_ptr<LILNode>> newTmplParams;
                    bool hasTmplParamChanges = false;
                    for (const auto & tmplParam : tmplParams) {
                        if (tmplParam->getNodeType() == NodeTypeType) {
                            auto replacementTy = this->replaceType(std::static_pointer_cast<LILType>(tmplParam), templateTy, specializedTy);
                            if (replacementTy) {
                                newTmplParams.push_back(replacementTy);
                                hasTmplParamChanges = true;
                            } else {
                                newTmplParams.push_back(tmplParam);
                            }
                        } else {
                            newTmplParams.push_back(tmplParam);
                        }
                    }
                    if (hasTmplParamChanges) {
                        ret = sourceTy->clone();
                        ret->setTmplParams(std::move(newTmplParams));
                    }
                }
            }
            break;
        }
        case TypeTypePointer:
        {
            auto ptrTy = std::static_pointer_cast<LILPointerType>(sourceTy);
            auto arg = ptrTy->getArgument();
            auto replacementTy = this->replaceType(arg, templateTy, specializedTy);
            if (replacementTy) {
                auto newPtrTy = std::make_shared<LILPointerType>();
                newPtrTy->setName("ptr");
                newPtrTy->setArgument(replacementTy);
                ret = newPtrTy;
            }
            break;
        }
            
        case TypeTypeStaticArray:
        {
            auto saTy = std::static_pointer_cast<LILStaticArrayType>(sourceTy);
            auto childTy = saTy->getType();
            auto replacementTy = this->replaceType(childTy, templateTy, specializedTy);
            if (replacementTy) {
                auto newSATy = std::make_shared<LILStaticArrayType>();
                newSATy->setType(replacementTy);
                newSATy->setArgument(saTy->getArgument()->clone());
                ret = newSATy;
            }
            break;
        }
            
        case TypeTypeFunction:
        {
            auto fnTy = std::static_pointer_cast<LILFunctionType>(sourceTy);
            bool hasChanges = false;
            bool hasChangesArgs = false;
            bool hasChangesRet = false;
            std::vector<std::shared_ptr<LILNode>> newArgs;
            for (auto arg : fnTy->getArguments()) {
                if (arg->isA(NodeTypeType)) {
                    std::shared_ptr<LILType> argTy = std::static_pointer_cast<LILType>(arg);
                    auto replacementTy = this->replaceType(argTy, templateTy, specializedTy);
                    if (replacementTy) {
                        hasChanges = true;
                        hasChangesArgs = true;
                        newArgs.push_back(replacementTy);
                    } else {
                        newArgs.push_back(arg);
                    }
                } else if (arg->isA(NodeTypeVarDecl)){
                    auto vd = std::static_pointer_cast<LILVarDecl>(arg);
                    std::shared_ptr<LILType> vdTy = vd->getType();
                    auto replacementTy = this->replaceType(vdTy, templateTy, specializedTy);
                    if (replacementTy) {
                        vd->setType(replacementTy);
                    }
                } else {
                    std::cerr << "UNKNOWN NODE TYPE OF ARGUMENT OF FUNCTION TYPE FAIL !!!!!\n\n";
                    break;
                }
            }
            auto retTy = fnTy->getReturnType();
            std::shared_ptr<LILType> replacementRetTy;
            if (retTy) {
                replacementRetTy = this->replaceType(retTy, templateTy, specializedTy);
                if (replacementRetTy) {
                    hasChanges = true;
                    hasChangesRet = true;
                }
            }
            
            if (hasChanges) {
                auto newFnTy = std::make_shared<LILFunctionType>();
                if (hasChangesArgs) {
                    newFnTy->setArguments(newArgs);
                } else {
                    newFnTy->setArguments(fnTy->getArguments());
                }
                if (hasChangesRet) {
                    newFnTy->setReturnType(replacementRetTy);
                }
                ret = newFnTy;
            }
            break;
        }
            
        case TypeTypeMultiple:
        {
            auto mTy = std::static_pointer_cast<LILMultipleType>(sourceTy);
            bool hasChanges = false;
            std::vector<std::shared_ptr<LILType>> newChildren;
            for (auto childTy : mTy->getTypes()) {
                auto replacementTy = this->replaceType(childTy, templateTy, specializedTy);
                if (replacementTy) {
                    newChildren.push_back(replacementTy);
                    hasChanges = true;
                } else {
                    newChildren.push_back(childTy);
                }
            }
            if (hasChanges) {
                mTy->setTypes(std::move(newChildren));
                ret = mTy;
            }
            break;
        }
        case TypeTypeSIMD:
        case TypeTypeNone:
        {
            //do nothing
            break;
        }
    }
    if (ret && sourceTy->getIsNullable()) {
        ret->setIsNullable(true);
    }
    return ret;
}
