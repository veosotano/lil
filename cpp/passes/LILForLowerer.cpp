
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

#include "LILForLowerer.h"
#include "../shared/LILErrorMessage.h"
#include "../ast/LILAssignment.h"
#include "../ast/LILExpression.h"
#include "../ast/LILFlowControl.h"
#include "../ast/LILNumberLiteral.h"
#include "../ast/LILPropertyName.h"
#include "../ast/LILRootNode.h"
#include "../ast/LILUnaryExpression.h"
#include "../ast/LILValuePath.h"
#include "../ast/LILVarDecl.h"
#include "../ast/LILVarName.h"

using namespace LIL;

LILForLowerer::LILForLowerer()
{
}

LILForLowerer::~LILForLowerer()
{
}

void LILForLowerer::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "======  FOR LOWERING  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILForLowerer::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();

    for (auto node : nodes) {
        this->processForBlocks(node.get());
    }
}

void LILForLowerer::processForBlocks(LILNode * node)
{
    for (const auto & childNode : node->getChildNodes()) {
        this->processForBlocks(childNode.get());
    }
    if (node->getFlowControlType() == FlowControlTypeFor) {
        auto fc = static_cast<LILFlowControl *>(node);
        const auto & args = fc->getArguments();
        if (args.size() == 1) {
            auto arg = args.at(0);
            fc->setSubject(arg);
            switch (arg->getNodeType()) {
                case NodeTypeNumberLiteral:
                {
                    this->_createForArgsNumber(fc, arg.get());
                    break;
                }
                case NodeTypeVarName:
                case NodeTypeValuePath:
                {
                    auto remoteNode = this->recursiveFindNode(arg);
                    if (!remoteNode) {
                        std::cerr << "REMOTE NODE NOT FOUND FAIL !!!!!!!!\n";
                        return;
                    }
                    auto ty = remoteNode->getType();
                    if (!ty) {
                        std::cerr << "REMOTE NODE HAD NO TYPE FAIL !!!!!!!!\n";
                        return;
                    }
                    if (LILType::isNumberType(ty.get())) {
                        this->_createForArgsNumber(fc, arg.get());
                    } else if (ty->getTypeType() == TypeTypeObject) {
                        this->_createForArgsObject(fc, arg.get(), ty.get());
                    } else {
                        std::cerr << "UNEXPECTED TYPE FAIL !!!!!!!!\n";
                        return;
                    }
                    break;
                }
                    
                default:
                    std::cerr << "UNEXPECTED NOTE TYPE FAIL !!!!!!!!\n";
                    return;
            }
        }
    }
}

void LILForLowerer::_createForArgsNumber(LILFlowControl * fc, LILNode * arg) const
{
    std::vector<std::shared_ptr<LILNode>> newArgs;
    auto vd = std::make_shared<LILVarDecl>();
    auto numTy = LILType::make("i64");
    vd->setName("@value");
    vd->setType(numTy);
    auto vdInitVal = std::make_shared<LILNumberLiteral>();
    vdInitVal->setValue("0");
    vdInitVal->setType(numTy);
    vd->setInitVal(vdInitVal);
    newArgs.push_back(vd);
    auto comparison = std::make_shared<LILExpression>();
    comparison->setExpressionType(ExpressionTypeSmallerComparison);
    comparison->setType(numTy);
    auto vn = std::make_shared<LILVarName>();
    vn->setName("@value");
    vn->setType(numTy);
    comparison->setLeft(vn);
    comparison->setRight(arg->clone());
    newArgs.push_back(comparison);
    auto plusOne = std::make_shared<LILUnaryExpression>();
    plusOne->setUnaryExpressionType(UnaryExpressionTypeSum);
    plusOne->setType(numTy);
    plusOne->setSubject(vn->clone());
    auto oneLit = std::make_shared<LILNumberLiteral>();
    oneLit->setValue("1");
    oneLit->setType(numTy);
    plusOne->setValue(oneLit);
    newArgs.push_back(plusOne);
    fc->setArguments(std::move(newArgs));
}

void LILForLowerer::_createForArgsObject(LILFlowControl * fc, LILNode * arg, LILType * ty) const
{
    auto cd = this->findClassWithName(ty->getName());
    if (!cd) {
        std::cerr << "CLASS " + ty->getName().data() + " NOT FOUND FAIL !!!!\n";
        return;
    }
    std::vector<std::shared_ptr<LILNode>> newArgs;
    auto vd = std::make_shared<LILVarDecl>();
    auto numTy = LILType::make("i64");
    vd->setName("@key");
    vd->setType(numTy);
    auto vdInitVal = std::make_shared<LILNumberLiteral>();
    vdInitVal->setValue("0");
    vdInitVal->setType(numTy);
    vd->setInitVal(vdInitVal);
    newArgs.push_back(vd);
    auto comparison = std::make_shared<LILExpression>();
    comparison->setExpressionType(ExpressionTypeSmallerComparison);
    comparison->setType(numTy);
    auto vn = std::make_shared<LILVarName>();
    vn->setName("@key");
    vn->setType(numTy);
    comparison->setLeft(vn);
    auto vp = std::make_shared<LILValuePath>();
    vp->addNode(arg->shared_from_this());
    auto sizePn = std::make_shared<LILPropertyName>();
    sizePn->setName("size");
    vp->setType(numTy);
    vp->addNode(sizePn);
    comparison->setRight(vp);
    newArgs.push_back(comparison);
    auto plusOne = std::make_shared<LILUnaryExpression>();
    plusOne->setUnaryExpressionType(UnaryExpressionTypeSum);
    plusOne->setType(numTy);
    plusOne->setSubject(vn->clone());
    auto oneLit = std::make_shared<LILNumberLiteral>();
    oneLit->setValue("1");
    oneLit->setType(numTy);
    plusOne->setValue(oneLit);
    newArgs.push_back(plusOne);
    fc->setArguments(std::move(newArgs));
}
