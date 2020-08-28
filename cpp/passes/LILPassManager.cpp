//
//  LILPassManager.cpp
//  LILInterpreterTest
//
//  Created by Miro Keller on 24/7/20.
//  Copyright © 2020 Miro Keller. All rights reserved.
//

#include "LILPassManager.h"
#include "LILRootNode.h"
#include "LILVisitor.h"

using namespace LIL;

LILPassManager::LILPassManager()
: _verbose(false)
{
    
}

LILPassManager::~LILPassManager()
{
    
}

void LILPassManager::addPass(std::unique_ptr<LILVisitor> visitor)
{
    visitor->setVerbose(this->_verbose);
    this->_visitors.push_back(std::move(visitor));
}

void LILPassManager::execute(std::shared_ptr<LILRootNode> rootNode, const LILString & code)
{
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    for (const auto & visitor : this->_visitors) {
        visitor->initializeVisit();
        visitor->performVisit(rootNode);
        if (visitor->hasErrors())
        {
            visitor->printErrors(code);
            break;
        }
    }
}

const std::vector<std::unique_ptr<LILVisitor>> & LILPassManager::getPasses() const
{
    return this->_visitors;
}

bool LILPassManager::getVerbose() const
{
    return this->_verbose;
}
void LILPassManager::setVerbose(bool value)
{
    this->_verbose = value;
}

bool LILPassManager::hasErrors() const
{
    for (const auto & visitor : this->_visitors) {
        if (visitor->hasErrors())
        {
            return true;
        }
    }
    return false;
}
