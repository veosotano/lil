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
 *      This file changes the name of variables and functions to a
 *      lower level
 *
 ********************************************************************/

#include "LILNameLowerer.h"
#include "LILNodeToString.h"
#include "LILObjectType.h"
#include "LILPointerType.h"
#include "LILVarNode.h"

using namespace LIL;

LILNameLowerer::LILNameLowerer()
{
}

LILNameLowerer::~LILNameLowerer()
{
}

void LILNameLowerer::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  NAME  LOWERING  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILNameLowerer::visit(LILNode *node)
{
    this->process(node);
}

void LILNameLowerer::process(LILNode * node)
{
    if (this->getDebug()) {
        std::cerr << "## lowering name " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node).data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBoolLiteral:
        {
            LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeNumberLiteral:
        {
            LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypePercentage:
        {
            LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeExpression:
        {
            LILExpression * value = static_cast<LILExpression *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeUnaryExpression:
        {
            LILUnaryExpression * value = static_cast<LILUnaryExpression *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeStringLiteral:
        {
            LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeStringFunction:
        {
            LILStringFunction * value = static_cast<LILStringFunction *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeNull:
        {
            LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeVarDecl:
        {
            LILVarDecl * value = static_cast<LILVarDecl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeAliasDecl:
        case NodeTypeTypeDecl:
        case NodeTypeConversionDecl:
        {
            //do nothing
            break;
        }
        case NodeTypeClassDecl:
        {
            LILClassDecl * value = static_cast<LILClassDecl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeObjectDefinition:
        {
            LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeAssignment:
        {
            LILAssignment * value = static_cast<LILAssignment *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeValuePath:
        {
            LILValuePath * value = static_cast<LILValuePath *>(node);
            this->_process(value);
            break;
        }
        case NodeTypePropertyName:
        {
            LILPropertyName * value = static_cast<LILPropertyName *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeVarName:
        {
            LILVarName * value = static_cast<LILVarName *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeRule:
        {
            LILRule * value = static_cast<LILRule *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeSimpleSelector:
        {
            LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeSelectorChain:
        {
            LILSelectorChain * value = static_cast<LILSelectorChain *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeSelector:
        {
            LILSelector * value = static_cast<LILSelector *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeCombinator:
        {
            LILCombinator * value = static_cast<LILCombinator *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFilter:
        {
            LILFilter * value = static_cast<LILFilter *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFlag:
        {
            LILFlag * value = static_cast<LILFlag *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFunctionCall:
        {
            LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeFlowControl:
        {
            LILFlowControl * value = static_cast<LILFlowControl *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeType:
        {
            break;
        }
        case NodeTypeValueList:
        {
            LILValueList * value = static_cast<LILValueList *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeDocumentation:
        {
            LILDocumentation * value = static_cast<LILDocumentation *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeIndexAccessor:
        {
            LILIndexAccessor * value = static_cast<LILIndexAccessor *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeSnippetInstruction:
        {
            //ignore
            break;
        }

        default:
            std::cerr << "Error: unkonwn node type to process\n";
            break;
    }
}

void LILNameLowerer::_process(LILBoolLiteral * value)
{

}

void LILNameLowerer::_process(LILNumberLiteral * value)
{

}

void LILNameLowerer::_process(LILPercentageLiteral * value)
{

}

void LILNameLowerer::_process(LILExpression * value)
{

}

void LILNameLowerer::_process(LILUnaryExpression * value)
{

}

void LILNameLowerer::_process(LILStringLiteral * value)
{

}

void LILNameLowerer::_process(LILStringFunction * value)
{
    this->processChildren(value->getNodes());
}

void LILNameLowerer::_process(LILNullLiteral * value)
{

}

void LILNameLowerer::_process(LILVarDecl * value)
{

}

void LILNameLowerer::_process(LILClassDecl * value)
{
    if (!value->getIsExtern()) {
        auto ty = value->getType();
        if (!ty || !value->isTemplate()) {
            this->processChildren(value->getFields());
            for (const auto & methodPair : value->getMethods())
            {
                auto methodNode = methodPair.second;
                if (methodNode->isA(NodeTypeFunctionDecl)) {
                    auto fd = std::static_pointer_cast<LILFunctionDecl>(methodNode);
                    if (fd->getHasMultipleImpls()) {
                        auto ptrTy = std::make_shared<LILPointerType>();
                        ptrTy->setName("ptr");
                        ptrTy->setArgument(value->getType()->clone());
                        auto argVd = std::make_shared<LILVarDecl>();
                        argVd->setName("@self");
                        argVd->setType(ptrTy);
                        
                        for (const auto & innerFd : fd->getImpls()) {
                            auto ty = innerFd->getType();
                            if (ty->getTypeType() != TypeTypeFunction) {
                                std::cerr << "INNER FD HAD NO FN TYPE FAIL!!!!!!!!!\n\n";
                                return;
                            }
                            auto newName = this->decorate("", value->getName().data(), innerFd->getName(), ty);
                            innerFd->setName(newName);
                            auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                            fnTy->prependArgument(argVd);
                        }
                        //add the @self argument to the parent fd too
                        auto fnTy = fd->getFnType();
                        if (!fnTy) {
                            std::cerr << "FD HAD NO FN TYPE FAIL!!!!!!!!!\n\n";
                            return;
                        }
                        fnTy->prependArgument(argVd);
                    } else {
                        auto ty = fd->getType();
                        if (ty->getTypeType() != TypeTypeFunction) {
                            return;
                        }
                        auto newName = this->decorate("", value->getName().data(), fd->getName(), ty);
                        fd->setName(newName);
                        
                        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                        auto ptrTy = std::make_shared<LILPointerType>();
                        ptrTy->setName("ptr");
                        ptrTy->setArgument(value->getType()->clone());
                        auto argVd = std::make_shared<LILVarDecl>();
                        argVd->setName("@self");
                        argVd->setType(ptrTy);
                        fnTy->prependArgument(argVd);
                    }
                }
            }
        }
    }
}

void LILNameLowerer::_process(LILObjectDefinition * value)
{
    this->processChildren(value->getNodes());
}

void LILNameLowerer::_process(LILAssignment * value)
{
    this->processChildren(value->getNodes());
}

void LILNameLowerer::_process(LILValuePath * value)
{
    this->processChildren(value->getNodes());
}

void LILNameLowerer::_process(LILPropertyName * value)
{
}

void LILNameLowerer::_process(LILRule * value)
{
    this->processChildren(value->getChildNodes());
}

void LILNameLowerer::_process(LILSimpleSelector * value)
{

}

void LILNameLowerer::_process(LILSelectorChain * value)
{

}

void LILNameLowerer::_process(LILSelector * value)
{
}

void LILNameLowerer::_process(LILCombinator * value)
{
}

void LILNameLowerer::_process(LILFilter * value)
{
}

void LILNameLowerer::_process(LILFlag * value)
{
}

void LILNameLowerer::_process(LILVarName * value)
{
}

void LILNameLowerer::_process(LILFunctionDecl * value)
{
    auto parent = value->getParentNode();
    if (!parent) {
        return;
    }
    if (!parent->isA(NodeTypeClassDecl)) {
        if (value->getHasMultipleImpls()) {
            for (const auto & impl : value->getImpls()) {
                auto newImplName = this->decorate("", "", impl->getName(), impl->getFnType());
                impl->setName(newImplName);
            }
        }
        return;
    }
    auto ty = value->getType();
    if (ty->getTypeType() != TypeTypeFunction) {
        return;
    }
    auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
    auto name = value->getName();

    if (parent->isA(NodeTypeClassDecl)) {
        auto cd = std::static_pointer_cast<LILClassDecl>(parent);
        auto classTy = cd->getType();
        if (value->getHasMultipleImpls()) {
            for (const auto & impl : value->getImpls()) {
                auto newImplName = this->decorate("", classTy->getName(), impl->getName(), impl->getFnType());
                impl->setName(newImplName);
            }
        } else {
            auto newName = this->decorate("", classTy->getName(), name, ty);
            value->setName(newName);
        }

    } else {
        bool needsDecor = false;
        for (auto arg : fnTy->getArguments()) {
            if (arg->getTypeType() == TypeTypeMultiple) {
                needsDecor = true;
                break;
            }
        }
        if (needsDecor) {
            auto newName = this->decorate("", "", name, fnTy);
            value->setName(newName);
        }
    }
}

void LILNameLowerer::_process(LILFunctionCall * value)
{
    if (value->getFunctionCallType() == FunctionCallTypeValuePath) {
        auto vp = std::static_pointer_cast<LILValuePath>(value->getParentNode());
        auto firstNode = vp->getNodes().front();
        if (firstNode && firstNode->isA(NodeTypeVarName)) {
            auto vn = std::static_pointer_cast<LILVarName>(firstNode);
            auto name = vn->getName();
            auto newName = this->decorate("", "", name, nullptr);
        }
    }
    this->processChildren(value->getArguments());
}

void LILNameLowerer::_process(LILFlowControl * value)
{
    this->processChildren(value->getThen());
    this->processChildren(value->getElse());
}

void LILNameLowerer::_process(LILInstruction * value)
{
}

void LILNameLowerer::_process(LILValueList * value)
{
    this->processChildren(value->getValues());
}

void LILNameLowerer::_process(LILDocumentation * value)
{
}

void LILNameLowerer::_process(LILIndexAccessor * value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->process(arg.get());
    }
}

void LILNameLowerer::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process((*it).get());
    };
}
