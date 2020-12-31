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
#include "LILNodeToString.h"
#include "LILObjectType.h"
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
            if (ty && ty->getParamTypes().size() > 0) {
                std::vector<std::shared_ptr<LILClassDecl>> newClasses;
                
                auto specializations = this->findClassSpecializations(nodes);
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
    }
    rootNode->setChildNodes(std::move(resultNodes));
}

std::vector<std::shared_ptr<LILNode>> LILClassTemplateLowerer::findClassSpecializations(const std::vector<std::shared_ptr<LILNode>> & nodes) const
{
    std::vector<std::shared_ptr<LILNode>> ret;
    for (auto node : nodes) {
        const auto & childNodes = node->getChildNodes();
        if (childNodes.size() > 0) {
            auto childRet = this->findClassSpecializations(childNodes);
            if (childRet.size() > 0) {
                ret.insert(ret.end(), childRet.begin(), childRet.end());
            }
        }
        
        if (node->isA(NodeTypeObjectDefinition)) {
            auto ty = node->getType();
            if (ty && ty->getParamTypes().size() > 0) {
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
            if (nodeTy && nodeTy->equalTo(templateType)) {
                tyNode->setType(specializedType);
            }
        }
    }
}
