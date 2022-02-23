
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
 *      This file calculates the values for the enums
 *
 ********************************************************************/

#include "LILEnumLowerer.h"
#include "LILAssignment.h"
#include "LILEnum.h"
#include "LILErrorMessage.h"
#include "LILNumberLiteral.h"
#include "LILRootNode.h"

using namespace LIL;

LILEnumLowerer::LILEnumLowerer()
{
}

LILEnumLowerer::~LILEnumLowerer()
{
}

void LILEnumLowerer::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "======  ENUM LOWERING  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILEnumLowerer::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        this->process(node.get());
    }
}

void LILEnumLowerer::process(LILNode * node)
{
    if (node->getNodeType() == NodeTypeEnum) {
        this->_process(static_cast<LILEnum *>(node));
    }
}

void LILEnumLowerer::_process(LILEnum * enm)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> newNodes;
    std::map<size_t, bool> values;
    size_t autoIndex = 0;
    for (auto node : enm->getValues()) {
        if (node->getNodeType() == NodeTypeAssignment) {
            auto asgmt = std::static_pointer_cast<LILAssignment>(node);
            auto value = asgmt->getValue();
            
            if (value->getNodeType() == NodeTypeExpression) {
                std::cout << "UNIMPLEMENTED FAIL !!!!!! \n\n";
                continue;
            } else if (value->getNodeType() == NodeTypeNumberLiteral) {
                auto numLit = std::static_pointer_cast<LILNumberLiteral>(value);
                auto numLitValue = numLit->getValue().toLong();
                if (values.count(numLitValue) == 0) {
                    values[numLitValue] = true;
                } else {
                    LILErrorMessage ei;
                    ei.message =  "The value "+numLit->getValue()+" was already used";
                    LILNode::SourceLocation sl = value->getSourceLocation();
                    ei.file = sl.file;
                    ei.line = sl.line;
                    ei.column = sl.column;
                    this->errors.push_back(ei);
                }
            }
        } else if (node->getNodeType() == NodeTypePropertyName) {
            auto asgmt = std::make_shared<LILAssignment>();
            asgmt->setSubject(node);
            auto numLit = std::make_shared<LILNumberLiteral>();
            numLit->setValue(LILString::number((LILUnitI64)autoIndex));
            numLit->setType(enm->getType()->clone());
            asgmt->setValue(numLit);
            newNodes.push_back(asgmt);
            autoIndex += 1;
            values[autoIndex] = true;
            hasChanges = true;
        }
    }
    if (hasChanges) {
        enm->setValues(std::move(newNodes));
    }
}
