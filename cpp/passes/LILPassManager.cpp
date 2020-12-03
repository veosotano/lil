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
, _hasErrors(false)
{
    
}

LILPassManager::~LILPassManager()
{
    
}

void LILPassManager::execute(const std::vector<LILVisitor *> & visitors, std::shared_ptr<LILRootNode> rootNode, const LILString & code)
{
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    for (const auto & visitor : visitors) {
        visitor->setVerbose(this->getVerbose());
        visitor->initializeVisit();
        visitor->performVisit(rootNode);
        if (visitor->hasErrors())
        {
            visitor->printErrors(code);
            this->_hasErrors = true;
            break;
        }
    }
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
    return this->_hasErrors;
}
