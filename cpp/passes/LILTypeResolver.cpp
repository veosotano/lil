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
 *      This file replaces custom types with the types they resolve to
 *
 ********************************************************************/

#include "LILTypeResolver.h"
#include "LILConversionDecl.h"
#include "LILExpression.h"
#include "LILFlowControl.h"
#include "LILFunctionCall.h"
#include "LILFunctionType.h"
#include "LILMultipleType.h"
#include "LILPointerType.h"
#include "LILRootNode.h"
#include "LILTypeDecl.h"
#include "LILVarDecl.h"

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
        std::cerr << "===== TYPE RESOLVING  ======\n";
        std::cerr << "============================\n\n";
    }
}

void LILTypeResolver::visit(LILNode *node)
{
    this->process(node->shared_from_this());
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
    this->processChildren(node->getChildNodes());

    if (node->isA(FlowControlTypeIfIs)) {
        auto fc = std::static_pointer_cast<LILFlowControl>(node);
        auto args = fc->getArguments();
        if (args.size() != 2) {
            std::cerr << "IF IS BLOCK DID NOT HAVE 2 ARGUMENTS FAIL!!!!!\n";
            return;
        }
        auto lastArg = args.back();
        if (!lastArg->isA(NodeTypeType)) {
            std::cerr << "IF IS BLOCK HAD NO TYPE ARG FAIL!!!!!\n";
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
    if (node->isA(NodeTypeFunctionCall)) {
        auto fc = std::static_pointer_cast<LILFunctionCall>(node);
        auto argTypes = fc->getArgumentTypes();
        std::vector<std::shared_ptr<LILType>> newArgTypes;
        bool changed = false;
        for (auto argTy : argTypes) {
            auto newTy = this->_process(argTy);
            if (newTy) {
                newArgTypes.push_back(newTy);
                changed = true;
            } else {
                newArgTypes.push_back(argTy);
            }
        }
        if (changed) {
            fc->setArgumentTypes(newArgTypes);
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
                this->_process(arg);
            }
            break;
        }
        case TypeTypeMultiple:
        {
            auto multiTy = std::static_pointer_cast<LILMultipleType>(value);
            multiTy->sortTypes();
            
            for (auto childTy : multiTy->getTypes()) {
                this->_process(childTy);
            }
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

            break;
        }
        case TypeTypeSingle:
        {
            auto name = value->getName();
            if (!LILType::isBuiltInType(name)) {
                auto rootNode = this->getRootNode();
                auto types = rootNode->getTypes();
                for (auto type : types) {
                    if (type->getName() == name) {
                        auto typeTy = type->getType();
                        auto newTy = this->_process(typeTy);
                        if (newTy) {
                            ret = newTy;
                        } else {
                            ret = typeTy;
                        }
                        break;
                    }
                }
            }
            break;
        }
            
        default:
            break;
    }
    return ret;
}

void LILTypeResolver::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process((*it));
    };
}
