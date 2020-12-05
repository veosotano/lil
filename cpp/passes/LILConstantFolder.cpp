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
 *      This file tries to pre-bake strings from string functions
 *
 ********************************************************************/

#include "LILConstantFolder.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILRootNode.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

using namespace LIL;

LILConstantFolder::LILConstantFolder()
{
}

LILConstantFolder::~LILConstantFolder()
{
}

void LILConstantFolder::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "==== CONSTANT FOLDING  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILConstantFolder::visit(LILNode *node)
{
    this->process(node->shared_from_this());
}

void LILConstantFolder::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    this->processChildren(rootNode, nodes);
}

void LILConstantFolder::processChildren(std::shared_ptr<LILNode> parent, const std::vector<std::shared_ptr<LILNode> > &nodes)
{
    
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->process(node);
        if (this->_nodeBuffer.back().size() > 0) {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        } else {
            resultNodes.push_back(node);
        }
        
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        parent->clearChildNodes();
        parent->setChildNodes(std::move(resultNodes));
    }
}

void LILConstantFolder::process(std::shared_ptr<LILNode> node)
{
    this->processChildren(node, node->getChildNodes());
    if (node->isA(NodeTypeStringFunction)) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);

        auto strFn = std::static_pointer_cast<LILStringFunction>(node);
        const auto & childNodes = strFn->getNodes();
        auto & midChunks = strFn->_midChunks;
        if (childNodes.size() > 0 && childNodes.size() == midChunks.size()+1) {
            bool doneWithFirstChunk = false;
            size_t startIndex = 0;
            std::vector<LILString> newChunks;
            for (size_t i=0, j=childNodes.size(); i<j; i+=1) {
                auto childNode = childNodes[i];
                std::shared_ptr<LILNode> remoteNode = this->_resolveRemoteNode(childNode);
                if (!remoteNode->isA(NodeTypeVarDecl)) {
                    continue;
                }
                auto vd = std::static_pointer_cast<LILVarDecl>(remoteNode);
                auto initVal = vd->getInitVal();
                //FIXME: use static analysis to determine if the literal stays a literal
                if (
                    initVal
                    && (
                        initVal->isA(NodeTypeCStringLiteral)
                        || initVal->isA(NodeTypeStringLiteral)
                    )
                ) {
                    auto str = std::static_pointer_cast<LILStringLiteral>(initVal);
                    if (!doneWithFirstChunk) {
                        strFn->_startChunk.append(str->getValue().stripQuotes().data());
                        if (midChunks.size() > i) {
                            strFn->_startChunk.append(midChunks.at(i).data());
                        } else {
                            strFn->_startChunk.append(strFn->_endChunk.data());
                        }
                        
                    } else {
                        std::string newChunk = str->getValue().data();
                        newChunk.append(midChunks.at(i).data());
                        newChunks.push_back(newChunk);
                    }
                } else {
                    if (!doneWithFirstChunk) {
                        doneWithFirstChunk = true;
                        startIndex = i;
                    }
                }
            }
            if (!doneWithFirstChunk)
            {
                auto stringLiteral = std::make_shared<LILStringLiteral>();
                stringLiteral->setValue(strFn->_startChunk);
                this->addReplacementNode(stringLiteral);
            }
            else
            {
                std::cerr << "!!!! UNIMPLEMENTED FAIL !!!!!!!! \n";
                return;
            }
        }
    }
}

void LILConstantFolder::addReplacementNode(std::shared_ptr<LILNode> node)
{
    this->_nodeBuffer.back().push_back(node);
}

std::shared_ptr<LILNode> LILConstantFolder::_resolveRemoteNode(std::shared_ptr<LILNode> node) const
{
    switch (node->getNodeType()) {
        case NodeTypeVarName:
        {
            return this->_resolveRemoteNode(this->findNodeForVarName(static_cast<LILVarName *>(node.get())));
        }
        case NodeTypeValuePath:
        {
            return this->_resolveRemoteNode(this->findNodeForValuePath(static_cast<LILValuePath *>(node.get())));
        }
        default:
            return node;
    }
}
