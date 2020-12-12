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
 *      This file inserts calls to type conversion functions
 *
 ********************************************************************/

#include "LILConversionInserter.h"
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

LILConversionInserter::LILConversionInserter()
{
}

LILConversionInserter::~LILConversionInserter()
{
}

void LILConversionInserter::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=== CONVERSION INSERTING ===\n";
        std::cerr << "============================\n\n";
    }
}

void LILConversionInserter::visit(LILNode *node)
{
    this->process(node->shared_from_this());
}

void LILConversionInserter::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    auto nodes = rootNode->getNodes();
    for (const auto & node : nodes) {
        this->process(node);
    }
}

void LILConversionInserter::process(std::shared_ptr<LILNode> node)
{
    this->processChildren(node->getChildNodes());
    switch (node->getNodeType()) {
        case NodeTypeFunctionCall:
        {
            auto value = std::static_pointer_cast<LILFunctionCall>(node);
            this->process(value);
            break;
        }
            
        default:
            break;
    }
}

void LILConversionInserter::process(std::shared_ptr<LILFunctionCall> fc)
{
    if (fc->isA(FunctionCallTypeNone)) {
        auto localNode = this->findNodeForName(fc->getName(), fc->getParentNode().get());
        if (!localNode) {
            std::cerr << "!!!!!!! TARGET NODE NOT FOUND FAIL !!!!!!!\n";
            return;
        }

        auto ty = localNode->getType();
        if (
            !(localNode->isA(NodeTypeVarDecl) || localNode->isA(NodeTypeFunctionDecl))
            || !ty || !ty->isA(TypeTypeFunction)
        ) {
            std::cerr << "!!!!!!! UNKNOWN TARGET NODE FAIL !!!!!!!\n";
            return;
        }
        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
        auto fnTyArgs = fnTy->getArguments();
        auto fcArgs = fc->getArguments();
        if (!fnTy->getIsVariadic() && fcArgs.size() != fnTyArgs.size()) {
            std::cerr << "!!!!!!! SIZE OF FUNCTION CALL ARGS AND ARGUMENT TYPES WAS NOT THE SAME FAIL !!!!!!!\n";
            return;
        }
        
        auto conversions = this->getRootNode()->getConversions();
        bool changed = false;

        std::vector<std::shared_ptr<LILNode>> newArguments;
        std::vector<std::shared_ptr<LILType>> newArgumentTypes;

        size_t i = 0;
        for (auto fcArg : fcArgs)
        {
            if (i > fnTyArgs.size()-1) {
                break;
            }
            auto fnTyArg = fnTyArgs[i];
            auto fcArgTy = fcArg->getType();
            std::shared_ptr<LILType> argTy;
            LILString argName;
            if (fnTyArg->isA(NodeTypeType))
            {
                argTy = std::static_pointer_cast<LILType>(fnTyArg);
                argName = LILString::number((LILUnitI64)i+1);
            }
            else if (fnTyArg->isA(NodeTypeVarDecl))
            {
                argTy = fnTyArg->getType();
                auto vd = std::static_pointer_cast<LILVarDecl>(fnTyArg);
                argName = vd->getName();
            }
            if (argTy->equalTo(fcArgTy)) {
                newArguments.push_back(fcArg);
                newArgumentTypes.push_back(argTy);
                continue;
            } else {
                LILString conversionName = fcArgTy->stringRep();
                conversionName += "_to_";
                conversionName += argTy->stringRep();
                if (conversions.count(conversionName)) {
                    auto conv = conversions[conversionName];
                    changed = true;
                    auto newCall = std::make_shared<LILFunctionCall>();
                    newCall->setFunctionCallType(FunctionCallTypeConversion);
                    newCall->setName(conv->encodedName());
                    newCall->addArgument(fcArg);
                    std::vector<std::shared_ptr<LILType>> fcTypes;
                    fcTypes.push_back(fcArgTy);
                    newCall->setArgumentTypes(fcTypes);
                    newCall->setReturnType(argTy);

                    newArguments.push_back(newCall);
                    newArgumentTypes.push_back(argTy);
                }
            }
            
            i += 1;
        }
        
        if (changed) {
            fc->setArguments(newArguments);
            fc->setArgumentTypes(newArgumentTypes);
        }
    }
}

void LILConversionInserter::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process((*it));
    };
}
