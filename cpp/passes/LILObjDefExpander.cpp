
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
 *      This file converts long value paths in object definitions
 *      into nested objdefs
 *
 ********************************************************************/

#include "LILObjDefExpander.h"
#include "LILAssignment.h"
#include "LILClassDecl.h"
#include "LILObjectDefinition.h"
#include "LILPropertyName.h"
#include "LILRootNode.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"

using namespace LIL;

LILObjDefExpander::LILObjDefExpander()
{
}

LILObjDefExpander::~LILObjDefExpander()
{
}

void LILObjDefExpander::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "====  OBJ DEF EXPANDING  ===\n";
        std::cerr << "============================\n\n";
    }
}

void LILObjDefExpander::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        this->process(node.get());
    }
}

void LILObjDefExpander::process(LILNode * node) {
    switch (node->getNodeType()) {
        case NodeTypeObjectDefinition:
        {
            auto value = static_cast<LILObjectDefinition *>(node);
            LILString classNameStr = value->getType()->getName();
            std::string className = classNameStr.data();
            auto classValue = this->findClassWithName(classNameStr);
            
            for (const auto & field : classValue->getFields()) {
                if (field->getNodeType() != NodeTypeVarDecl) {
                    std::cerr << "FIELD WAS NOT VAR DECL FAIL !!!!!!!!\n\n";
                    continue;
                }
                auto vd = std::static_pointer_cast<LILVarDecl>(field);
                const auto & fieldName = vd->getName();
                auto vdTy = vd->getType();
                if (vdTy->getTypeType() == TypeTypeObject) {
                    auto fieldClass = this->findClassWithName(vdTy->getName());
                    if (!fieldClass) {
                        std::cerr << "CLASS OF FIELD NOT FOUND FAIL !!!!!!!!\n\n";
                        continue;
                    }
                    
                    std::shared_ptr<LILNode> initializer;
                    std::vector<std::shared_ptr<LILAssignment>> modifiers;
                    
                    std::vector<std::shared_ptr<LILNode>> newNodes;
                    bool hasChanges = false;
                    
                    const auto & nodes = value->getNodes();
                    for (auto node : nodes) {
                        bool wasUsed = false;
                        if (node->getNodeType() != NodeTypeAssignment) {
                            std::cerr << "NODE WAS NOT ASSIGNMENT FAIL !!!!!!!!\n\n";
                            continue;
                        }
                        auto asgmt = std::static_pointer_cast<LILAssignment>(node);
                        auto subj = asgmt->getSubject();
                        if (subj->getNodeType() == NodeTypeValuePath) {
                            if (subj) {
                                if (subj->isA(NodeTypePropertyName)) {
                                    auto pn = std::static_pointer_cast<LILPropertyName>(subj);
                                    if (pn->getName() == fieldName) {
                                        initializer = asgmt->getValue();
                                        newNodes.push_back(initializer);
                                        wasUsed = true;
                                    }
                                } else if (subj->isA(NodeTypeValuePath)){
                                    auto vp = std::static_pointer_cast<LILValuePath>(subj);
                                    auto vpNodes = vp->getNodes();
                                    std::shared_ptr<LILNode> firstNode = vpNodes.front();
                                    if (firstNode->isA(NodeTypePropertyName)) {
                                        auto pn = std::static_pointer_cast<LILPropertyName>(firstNode);
                                        if (pn->getName() == fieldName) {
                                            if (vpNodes.size() == 1) {
                                                initializer = asgmt->getValue();
                                                newNodes.push_back(initializer);
                                            } else {
                                                modifiers.push_back(asgmt);
                                            }
                                            wasUsed = true;
                                            hasChanges = true;
                                        }
                                    }
                                } else {
                                    std::cerr << "SUBJECT WAS NOT PROPERTY NAME OR VALUE PATH FAIL !!!!!!!!\n\n";
                                    continue;
                                }
                            }
                        }
                        if (!wasUsed) {
                            newNodes.push_back(node);
                        }
                    }
                    
                    if (modifiers.size() > 0 && !initializer) {
                        auto defaultValue = vd->getInitVal();
                        if (defaultValue) {
                            initializer = defaultValue->clone();
                        } else {
                            auto newObjDef = std::make_shared<LILObjectDefinition>();
                            newObjDef->setType(vdTy->clone());
                            initializer = newObjDef;
                        }
                        auto newAsgmt = std::make_shared<LILAssignment>();
                        auto newSubj = std::make_shared<LILPropertyName>();
                        newSubj->setName(fieldName);
                        newAsgmt->setSubject(newSubj);
                        newAsgmt->setValue(initializer);
                        newAsgmt->setType(vdTy->clone());
                        newNodes.push_back(newAsgmt);
                    }
                    
                    if (initializer && initializer->getNodeType() == NodeTypeObjectDefinition) {
                        auto objdef = std::static_pointer_cast<LILObjectDefinition>(initializer);
                        for (auto modifier : modifiers) {
                            auto newAsgmt = std::make_shared<LILAssignment>();
                            std::vector<std::shared_ptr<LILNode>> newSubj;
                            auto subj = std::static_pointer_cast<LILValuePath>(modifier->getSubject());
                            const auto & subjNodes = subj->getNodes();
                            for (size_t i=1; i<subjNodes.size(); ++i) {
                                newSubj.push_back(subjNodes.at(i)->clone());
                            }
                            if (newSubj.size() == 1) {
                                newAsgmt->setSubject(newSubj.front());
                            } else {
                                auto newVp = std::make_shared<LILValuePath>();
                                newVp->setNodes(newSubj);
                                newAsgmt->setSubject(newVp);
                            }
                            newAsgmt->setValue(modifier->getValue()->clone());
                            auto modTy = modifier->getType();
                            if (modTy) {
                                newAsgmt->setType(modTy->clone());
                            }
                            objdef->addChild(newAsgmt);
                        }
                    }
                    
                    if (hasChanges) {
                        value->setNodes(std::move(newNodes));
                    }
                }
            }
            break;
        }
            
        default:
            break;
    }

    for (auto child : node->getChildNodes()) {
        this->process(child.get());
    }
}

