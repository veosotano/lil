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
#include "LILClassDecl.h"
#include "LILFunctionType.h"
#include "LILMultipleType.h"
#include "LILNodeToString.h"
#include "LILObjectType.h"
#include "LILPointerType.h"
#include "LILStaticArrayType.h"
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
            auto ty = node->getType();
            if (cd->isTemplate()) {
                std::vector<std::shared_ptr<LILClassDecl>> newClasses;
                
                auto specializations = this->findClassSpecializations(nodes, ty);
                if (specializations.size() > 0) {
                    for (auto spNode : specializations) {
                        auto spTy = spNode->getType();
                        auto newClass = this->makeSpecializedClass(cd, spTy);
                        if (newClass) {
                            newClasses.push_back(newClass);
                        }
                        if (spNode->isTypedNode()) {
                            auto spTyNode = std::static_pointer_cast<LILTypedNode>(spNode);
                            spTyNode->setType(newClass->getType()->clone());
                        }
                    }
                    //out with the old
                    rootNode->removeClass(std::static_pointer_cast<LILClassDecl>(node));
                    rootNode->removeNode(node);
                    //in with the new
                    for (auto newCd : newClasses) {
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
                && vdTy->getParamTypes().size() > 0
                && vdTy->getName() == ty->getName()
            ) {
                ret.push_back(node);
            }
            if (!vdTy) {
                auto initVal = vd->getInitVal();
                if (initVal) {
                    vdTy = initVal->getType();
                    if (
                        vdTy
                        && vdTy->getParamTypes().size() > 0
                        && vdTy->getName() == ty->getName()
                        ) {
                        ret.push_back(initVal);
                    }
                }
            }
            
            continue;
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
                && objDefTy->getParamTypes().size() > 0
                && objDefTy->getName() == ty->getName()
            ) {
                ret.push_back(node);
            }
        }
    }
    return ret;
}

std::shared_ptr<LILClassDecl> LILClassTemplateLowerer::makeSpecializedClass(std::shared_ptr<LILClassDecl> cd, std::shared_ptr<LILType> specializedType) const
{
    auto ty = cd->getType();
    const auto & paramTys = ty->getParamTypes();
    const auto & specializedParamTys = specializedType->getParamTypes();
    if (paramTys.size() != specializedParamTys.size()) {
        std::cerr << "PARAM TYS AND SPECIALIZED PARAM TYS WERE NOT OF THE SAME SIZE FAIL!!!!!!\n\n";
        return nullptr;
    }
    auto ret = cd->clone();
    for (size_t i=0, j=paramTys.size(); i<j; i+=1) {
        auto cdParamTy = std::static_pointer_cast<LILType>(paramTys.at(i));
        auto spParamTy = std::static_pointer_cast<LILType>(specializedParamTys.at(i));
        this->replaceTypeWithSpecializedType(ret->getChildNodes(), cdParamTy, spParamTy);
    }
    LILString newName = "lil_"+specializedType->getName();
    for (auto paramTyNode : specializedType->getParamTypes()) {
        if (!paramTyNode->isA(NodeTypeType)) {
            std::cerr << "PARAM TYPE WAS NOT TYPE FAIL!!!!!!\n\n";
            continue;
        }
        auto paramTy = std::static_pointer_cast<LILType>(paramTyNode);
        newName += "_"+paramTy->getName();
    }
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
            if (sourceTy && sourceTy->equalTo(templateTy)) {
                ret = specializedTy->clone();
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
            if (fnTy->getReturnType()->equalTo(templateTy)) {
                hasChanges = true;
                hasChangesRet = true;
            }
            if (hasChanges) {
                auto newFnTy = std::make_shared<LILFunctionType>();
                if (hasChangesArgs) {
                    newFnTy->setArguments(newArgs);
                } else {
                    newFnTy->setArguments(fnTy->getArguments());
                }
                if (hasChangesRet) {
                    newFnTy->setReturnType(specializedTy->clone());
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
                mTy->setTypes(newChildren);
                ret = mTy;
            }
            break;
        }
            
        case TypeTypeNone:
            //do nothing
            break;
    }
    return ret;
}
