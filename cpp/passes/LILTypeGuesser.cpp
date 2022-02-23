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
 *      This file analyzes the code to determine types automatically
 *
 ********************************************************************/

#include "LILTypeGuesser.h"
#include "LILAliasDecl.h"
#include "LILNodeToString.h"
#include "LILObjectType.h"
#include "LILStaticArrayType.h"
#include "LILTypeDecl.h"
#include "LILVarNode.h"

using namespace LIL;

LILTypeGuesser::LILTypeGuesser()
{
}

LILTypeGuesser::~LILTypeGuesser()
{
}

void LILTypeGuesser::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  TYPE GUESSING   =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILTypeGuesser::visit(LILNode *node)
{
    this->process(node);
}

void LILTypeGuesser::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    auto nodes = rootNode->getNodes();
    for (const auto & node : nodes) {
        this->connectCallsWithDecls(node);
    }
    for (const auto & node : nodes) {
        this->propagateStrongTypes(node);
    }
    for (const auto & node : nodes) {
        this->searchForTypesFromInitVal(node);
    }
    for (const auto & node : nodes) {
        this->searchForTypesFromAssignments(node);
    }
    for (const auto & node : nodes) {
        this->searchForTypesForArguments(node);
    }
    for (const auto & node : nodes) {
        this->process(node.get());
    }
}

void LILTypeGuesser::connectCallsWithDecls(std::shared_ptr<LILNode> node)
{
    for (auto childNode : node->getChildNodes()) {
        this->connectCallsWithDecls(childNode);
    }
    if (node->isA(NodeTypeFunctionCall)) {
        auto fc = std::static_pointer_cast<LILFunctionCall>(node);
        auto parent = node->getParentNode();
        if (fc->isA(FunctionCallTypeNone)) {
            auto localNode = this->findNodeForName(fc->getName(), parent.get());
            if (localNode) {
                auto ty = localNode->getType();
                if (ty && ty->isA(TypeTypeFunction)) {
                    auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                    fnTy->addCaller(node);
                }
            }
        } else if (fc->isA(FunctionCallTypeValuePath)){
            auto vp = fc->getSubject();
            auto ty = this->findTypeForValuePath(vp);
            if (ty && ty->isA(TypeTypeFunction)) {
                auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                fnTy->addCaller(node);
            }
        }
    }
}

void LILTypeGuesser::propagateStrongTypes(std::shared_ptr<LILNode> node)
{
    if (this->getDebug()) {
        std::cerr << "## propagate strong types of " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }

    if (node->isA(NodeTypeVarDecl)) {
        auto ty = node->getType();
        if (ty && !ty->isA(TypeTypeFunction)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            auto initVal = vd->getInitVal();
            if (initVal) {
                this->_propagateStrongType(initVal, ty);
            }
        }
    }

    for (auto child : node->getChildNodes()) {
        this->propagateStrongTypes(child);
    }
}

void LILTypeGuesser::_propagateStrongType(std::shared_ptr<LILNode> node, std::shared_ptr<LILType> ty)
{
    switch (node->getNodeType()) {
        case NodeTypeNumberLiteral:
        {
            auto num = std::static_pointer_cast<LILNumberLiteral>(node);
            auto numTy = num->getType();
            if (!numTy) {
                num->setType(ty);
            }
            if (numTy->getIsWeakType()) {
                auto mergedTy = LILType::merge(numTy, ty);
                if (mergedTy) {
                    num->setType(mergedTy);
                }
            }
            break;
        }
            
        case NodeTypeExpression:
        {
            auto exp = std::static_pointer_cast<LILExpression>(node);
            if (exp->isA(ExpressionTypeCast)) {
                break;
            }
            exp->setType(ty);
            this->_propagateStrongType(exp->getLeft(), ty);
            this->_propagateStrongType(exp->getRight(), ty);
            break;
        }
            
        case NodeTypeUnaryExpression:
        {
            auto uexp = std::static_pointer_cast<LILUnaryExpression>(node);
            uexp->setType(ty);
            this->_propagateStrongType(uexp->getValue(), ty);
            break;
        }
            
        case NodeTypeFunctionDecl:
        {
            for (auto bNod : std::static_pointer_cast<LILFunctionDecl>(node)->getBody()) {
                this->propagateStrongTypes(bNod);
            }
            break;
        }
            
        default:
            break;
    }
}

void LILTypeGuesser::searchForTypesFromInitVal(std::shared_ptr<LILNode> node)
{
    if (node->isA(NodeTypeVarDecl))
    {
        auto vd = std::static_pointer_cast<LILVarDecl>(node);
        auto initVal = vd->getInitVal();
        if (initVal) {
            auto ty = vd->getType();
            if (!ty) {
                if (initVal->isA(NodeTypeValueList)) {
                    auto vl = std::static_pointer_cast<LILValueList>(initVal);
                    auto vlTy = this->findTypeForValueList(vl);
                    std::shared_ptr<LILType> baseTy;
                    if (vlTy->getIsWeakType()) {
                        baseTy = vlTy->getDefaultType();
                    } else {
                        baseTy = vlTy;
                    }
                    auto arrTy = LILObjectType::make("lil_array_"+this->typeToString(baseTy));
                    vd->setType(arrTy);
                } else {
                    auto ivTy = this->getNodeType(initVal);
                    if (ivTy && !ivTy->getIsWeakType()) {
                        vd->setType(ivTy);
                    }
                }
            }
        }
    }
    for (auto childNode : node->getChildNodes()) {
        this->searchForTypesFromInitVal(childNode);
    }
}

void LILTypeGuesser::searchForTypesFromAssignments(std::shared_ptr<LILNode> node)
{
    if (node->isA(NodeTypeVarDecl))
    {
        auto vd = std::static_pointer_cast<LILVarDecl>(node);
        if (!vd->getInitVal()) {
            auto ty = vd->getType();
            if (!ty) {
                auto parent = node->getParentNode();
                if (!parent->isA(NodeTypeFunctionDecl)) {
                    std::cerr << "PARENT WAS NOT FUNCTION DECL FAIL!!!!!\n";
                    return;
                }
                auto fd = std::static_pointer_cast<LILFunctionDecl>(parent);
                auto type = this->findTypeFromAssignments(fd->getBody(), vd);
                if (type) {
                    vd->setType(type);
                }
            }
        }
    }

    for (auto childNode : node->getChildNodes()) {
        this->searchForTypesFromAssignments(childNode);
    }
}

void LILTypeGuesser::searchForTypesForArguments(std::shared_ptr<LILNode> node)
{
    if (node->isA(NodeTypeFunctionDecl))
    {
        auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
        auto ty = fd->getType();
        if (!ty->isA(TypeTypeFunction)) {
            std::cerr << "TYPE WAS NOT FUNCTION TYPE FAIL!!!!!\n";
            return;
        }
        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
        auto args = fnTy->getArguments();
        size_t argCount = 0;
        for (auto arg : args) {
            if (!arg->isA(NodeTypeVarDecl)) {
                continue;
            }
            auto vd = std::static_pointer_cast<LILVarDecl>(arg);
            if (!arg->getType()) {
                auto argType = this->findTypeForArg(vd, fd, argCount);
                if (argType) {
                    vd->setType(argType);
                }
            }
            ++argCount;
        }
    }

    for (auto childNode : node->getChildNodes()) {
        this->searchForTypesForArguments(childNode);
    }
}

void LILTypeGuesser::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process((*it).get());
    };
}

void LILTypeGuesser::process(LILNode * node)
{
    if (LILNode::isContainerNode(node->getNodeType())) {
        if (!node->isA(NodeTypeClassDecl) || !static_cast<LILClassDecl *>(node)->getIsExtern()) {
            this->processChildren(node->getChildNodes());
        }
    }
    if (this->getDebug()) {
        std::cerr << "## guessing types " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node).data() + " ##\n";
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
            if (value->isA(ExpressionTypeCast)) {
                this->_processCast(value);
            } else {
                this->_process(value);
            }
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
        case NodeTypeDocumentation:
        {
            //do nothing
            break;
        }
        case NodeTypeEnum:
        {
            LILEnum * value = static_cast<LILEnum *>(node);
            this->_process(value);
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
            for (auto child : value->getChildRules()) {
                this->process(child.get());
            }
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
        case NodeTypeFlowControlCall:
        {
            LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeIfInstruction:
        {
            LILIfInstruction * value = static_cast<LILIfInstruction *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeValueList:
        {
            LILValueList * value = static_cast<LILValueList *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeIndexAccessor:
        {
            LILIndexAccessor * value = static_cast<LILIndexAccessor *>(node);
            this->_process(value);
            break;
        }
        case NodeTypeType:
        case NodeTypeSnippetInstruction:
        {
            //do nothing
            break;
        }

        default:
            std::cerr << "Error: unkonwn node type to process\n";
            break;
    }

    if (node->isTypedNode()) {
        auto tyNode = static_cast<LILTypedNode *>(node);
        auto ty = tyNode->getType();
        if (ty) {
            auto newTy = this->nullsToNullableTypes(ty);
            if (newTy.get() != ty.get()) {
                tyNode->setType(newTy);
            }
        }
    }
}

void LILTypeGuesser::_process(LILBoolLiteral * value)
{
    
}

void LILTypeGuesser::_process(LILNumberLiteral * value)
{
    std::shared_ptr<LILType> ty1 = value->getType();
    auto sharedVal = value->shared_from_this();
    if (ty1 && ty1->getIsWeakType()) {
        std::shared_ptr<LILType>ty2 = this->recursiveFindTypeFromAncestors(sharedVal);
        if (ty2 && ty2->getIsNullable()) {
            ty2 = ty2->clone();
            ty2->setIsNullable(false);
        }
        bool isDefaultType = false;
        if (ty2 && ty2->isA(TypeTypePointer)) {
            isDefaultType = true;
        } else {
            auto ty3 = LILType::merge(ty1, ty2);
            if (ty3 && !ty3->getIsWeakType()) {
                value->setType(ty3);
                this->setTypeOnAncestorIfNeeded(sharedVal, ty3);
            }
            else if (ty1->getIsWeakType())
            {
                isDefaultType = true;
            }
        }
        if (isDefaultType) {
            //decay to default
            auto intType = ty1->getDefaultType();
            value->setType(intType);
        }
    } else if (!ty1) {
        std::shared_ptr<LILType>ty2 = this->recursiveFindTypeFromAncestors(sharedVal);
        if (ty2) {
            value->setType(ty2);
        }
    }
}

void LILTypeGuesser::_process(LILPercentageLiteral * value)
{
    std::shared_ptr<LILType> ty1 = value->getType();
    auto sharedVal = value->shared_from_this();
    if (ty1 && ty1->getIsWeakType()) {
        std::shared_ptr<LILType>ty2 = this->recursiveFindTypeFromAncestors(sharedVal);
        if (ty2 && ty2->getIsNullable()) {
            ty2 = ty2->clone();
            ty2->setIsNullable(false);
        }
        auto ty3 = LILType::merge(ty1, ty2);
        if (ty3) {
            if (ty3->getIsWeakType()) {
                auto multiTy = std::static_pointer_cast<LILMultipleType>(ty3);
                auto ret = multiTy->getTypes().front();
                value->setType(ret);
                this->setTypeOnAncestorIfNeeded(sharedVal, ret);
            } else {
                value->setType(ty3);
                this->setTypeOnAncestorIfNeeded(sharedVal, ty3);
            }
        }
        else
        {
            //decay to default
            auto intType = std::make_shared<LILType>();
            intType->setName("i64%");
            value->setType(intType);
            this->setTypeOnAncestorIfNeeded(sharedVal, intType);
        }
    }
}

void LILTypeGuesser::_processCast(LILExpression * value)
{
    auto existingTy = value->getType();
    if (existingTy) {
        return;
    }
    
    auto rightNode = value->getRight();
    if (!rightNode->isA(NodeTypeType)) {
        std::cerr << "RIGHT NODE OF CAST WAS NOT A TYPE FAIL!!!!!!!!\n";
        return;
    }
    auto ty = std::static_pointer_cast<LILType>(rightNode);
    value->setType(ty);
}

void LILTypeGuesser::_process(LILExpression * value)
{
    auto existingTy = value->getType();
    if (existingTy) {
        return;
    }
    
    switch (value->getExpressionType()) {
        case ExpressionTypeSmallerComparison:
        case ExpressionTypeSmallerOrEqualComparison:
        case ExpressionTypeBiggerComparison:
        case ExpressionTypeBiggerOrEqualComparison:
        case ExpressionTypeEqualComparison:
        case ExpressionTypeNotEqualComparison:
        case ExpressionTypeLogicalOr:
        case ExpressionTypeLogicalAnd:
        {
            auto boolTy = LILType::make("bool");
            value->setType(boolTy);
            return;
        }

        default:
            break;
    }

    std::shared_ptr<LILNode> left = value->getLeft();
    std::shared_ptr<LILNode> right = value->getRight();

    if (!left || !right) {
        return;
    }

    std::shared_ptr<LILType> leftTy;
    std::shared_ptr<LILType> rightTy;
    if (left->isTypedNode())
    {
        const auto & leftTN = std::static_pointer_cast<LILTypedNode>(left);
        leftTy = leftTN->getType();
    } else {
        leftTy = this->getNodeType(left);
    }
    if (right->isTypedNode())
    {
        const auto & rightTN = std::static_pointer_cast<LILTypedNode>(right);
        rightTy = rightTN->getType();
    } else {
        rightTy = this->getNodeType(right);
    }

    auto ty = LILType::merge(leftTy, rightTy);

    if (left->isTypedNode() && leftTy && leftTy->getIsWeakType()) {
        const auto & leftTN = std::static_pointer_cast<LILTypedNode>(left);
        leftTN->setType(ty);
    }
    if (right->isTypedNode() && rightTy && rightTy->getIsWeakType()) {
        const auto & rightTN = std::static_pointer_cast<LILTypedNode>(right);
        rightTN->setType(ty);
    }
    value->setType(ty);
}

void LILTypeGuesser::_process(LILUnaryExpression * value)
{
    auto existingTy = value->getType();
    if (existingTy) {
        return;
    }

    std::shared_ptr<LILNode> val = value->getValue();

    if (!val) {
        return;
    }

    std::shared_ptr<LILType> valTy;
    if (val->isTypedNode())
    {
        const auto & valTN = std::static_pointer_cast<LILTypedNode>(val);
        valTy = valTN->getType();
    } else {
        valTy = this->getNodeType(val);
    }
    value->setType(valTy);
}

void LILTypeGuesser::_process(LILStringLiteral * value)
{

}

void LILTypeGuesser::_process(LILStringFunction * value)
{

}

void LILTypeGuesser::_process(LILNullLiteral * value)
{

}

void LILTypeGuesser::_process(LILVarDecl * value)
{
    if (!value->getType()) {
        std::shared_ptr<LILNode> initValue = value->getInitVal();
        if (initValue) {
            auto type = initValue->getType();
            if (!type) {
                type = this->getNodeType(initValue);
            }
            if(type)
                value->setType(type);
        }
    }
}

void LILTypeGuesser::_process(LILEnum * value)
{
    auto ty = value->getType();
    if (!ty) {
        auto enumTy = this->findTypeForEnum(value);
        if (enumTy) {
            value->setType(enumTy);
        }
    }
}

void LILTypeGuesser::_process(LILClassDecl * value)
{
}

void LILTypeGuesser::_process(LILObjectDefinition * value)
{

}

void LILTypeGuesser::_process(LILAssignment * value)
{
    if (!value->getType()) {
        auto subjTy = this->getNodeType(value->getSubject());
        if (subjTy) {
            value->setType(subjTy->clone());
        } else {
            auto type = this->getNodeType(value->getValue());
            if(type)
                value->setType(type);
        }
    }
}

void LILTypeGuesser::_process(LILValuePath * value)
{
    auto ty = this->findTypeForValuePath(std::static_pointer_cast<LILValuePath>(value->shared_from_this()));
    if (ty) {
        value->setType(ty);
    }
}

void LILTypeGuesser::_process(LILPropertyName * value)
{
}

void LILTypeGuesser::_process(LILRule * value)
{
    if (!value->getType()) {
        const auto & instrNode = value->getInstruction();
        if (instrNode && instrNode->getNodeType() == NodeTypeInstruction) {
            auto instr = std::static_pointer_cast<LILInstruction>(instrNode);
            if (instr->getInstructionType() == InstructionTypeNew) {
                auto instrTy = instr->getType();
                if (instrTy) {
                    value->setType(instrTy);
                }
            }
        }
    }
    if (!value->getType()) {
        auto ty = this->findTypeForSelectorChain(std::static_pointer_cast<LILSelectorChain>(value->getSelectorChain()));
        if (ty) {
            value->setType(ty);
        }
    }
}

void LILTypeGuesser::_process(LILSimpleSelector * value)
{

}

void LILTypeGuesser::_process(LILSelectorChain * value)
{

}

void LILTypeGuesser::_process(LILSelector * value)
{
}

void LILTypeGuesser::_process(LILCombinator * value)
{
}

void LILTypeGuesser::_process(LILFilter * value)
{
}

void LILTypeGuesser::_process(LILFlag * value)
{
}

void LILTypeGuesser::_process(LILVarName * value)
{
    auto ty = this->findTypeForVarName(std::static_pointer_cast<LILVarName>(value->shared_from_this()));
    if (ty) {
        value->setType(ty);
    }
}

void LILTypeGuesser::_process(LILFunctionDecl * value)
{
    auto ty = value->getType();
    if (!ty || !ty->isA(TypeTypeFunction)) {
        std::shared_ptr<LILNode> type = this->getNodeType(value->shared_from_this());
        if(type && type->isA(NodeTypeType)){
            std::shared_ptr<LILFunctionType> ft = std::dynamic_pointer_cast<LILFunctionType>(type);
            if (ft) {
                value->setType(ft);
            }
        }
    } else {
        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
        std::vector<std::shared_ptr<LILNode>> newArgs;
        bool hasChanges = false;
        for (auto arg : fnTy->getArguments()) {
            if (arg->isA(NodeTypeType)) {
                auto newTy = this->nullsToNullableTypes(std::static_pointer_cast<LILType>(arg));
                if (newTy) {
                    newArgs.push_back(newTy);
                    hasChanges = true;
                } else {
                    newArgs.push_back(arg);
                }
            } else if (arg->isA(NodeTypeVarDecl)){
                auto vd = std::static_pointer_cast<LILVarDecl>(arg);
                auto vdTy = vd->getType();
                auto newTy = this->nullsToNullableTypes(vdTy);
                if (newTy) {
                    vd->setType(newTy);
                    hasChanges = true;
                }
                newArgs.push_back(vd);
            } else {
                std::cerr << "UNKNOWN NODE TYPE IN FUNCTION TYPE ARGS FAIL!!!!\n\n";
                return;
            }
        }
        if (hasChanges) {
            fnTy->setArguments(newArgs);
        }
        auto returnTy = fnTy->getReturnType();
        if (!returnTy) {
            auto returnTy = this->getFnReturnType(value->getBody());
            if (returnTy) {
                fnTy->setReturnType(returnTy);
            }
        } else {
            auto newReturnTy = this->nullsToNullableTypes(returnTy);
            if (newReturnTy.get() != returnTy.get()) {
                fnTy->setReturnType(newReturnTy);
            }
        }
    }
}

void LILTypeGuesser::_process(LILFunctionCall * value)
{
    std::vector<std::shared_ptr<LILType>> newTypes;
    std::shared_ptr<LILFunctionCall> sharedValue = std::static_pointer_cast<LILFunctionCall>(value->shared_from_this());
    for (auto arg : value->getArguments()) {
        auto ty = this->getNodeType(arg);
        if (ty) {
            if (ty->isA(TypeTypeFunction)) {
                auto returnTy = std::static_pointer_cast<LILFunctionType>(ty)->getReturnType();
                if (returnTy) {
                    newTypes.push_back(returnTy);
                }
            } else {
                newTypes.push_back(ty);
            }

        }
    }
    value->setArgumentTypes(newTypes);
    
    value->setReturnType(this->findReturnTypeForFunctionCall(sharedValue));
}

void LILTypeGuesser::_process(LILFlowControl * value)
{
    if (value->isA(FlowControlTypeIfCast)) {
        this->inhibitSearchingForIfCastType = true;
        for (auto arg : value->getArguments()) {
            this->process(arg.get());
        }
        this->inhibitSearchingForIfCastType = false;
    }
}

void LILTypeGuesser::_process(LILFlowControlCall * value)
{
    
}

void LILTypeGuesser::_process(LILInstruction * value)
{
}

void LILTypeGuesser::_process(LILIfInstruction * value)
{
    for (auto thenNode : value->getThen()) {
        this->process(thenNode.get());
    }
    for (auto elseNode : value->getElse()) {
        this->process(elseNode.get());
    }
}

void LILTypeGuesser::_process(LILValueList * value)
{
    auto ty = value->getType();
    if (!ty) {
        auto vlTy = this->findTypeForValueList(std::static_pointer_cast<LILValueList>(value->shared_from_this()));
        if (vlTy) {
            if (vlTy->isA(TypeTypeMultiple)){
                auto mt = std::static_pointer_cast<LILMultipleType>(vlTy);
                if (mt->getTypes().size() > 0) {
                    value->setType(vlTy);
                    return;
                } else {
                    vlTy = nullptr;
                }
            } else {
                auto vlParent = value->getParentNode();
                if (vlParent) {
                    switch (vlParent->getNodeType()) {
                        case NodeTypeVarDecl:
                        {
                            if (!vlParent->getType()) {
                                value->setType(vlTy);
                            } else {
                                auto parentTy = vlParent->getType();
                                if (parentTy->isA(TypeTypeMultiple)) {
                                    auto parentTyMtTys = std::static_pointer_cast<LILMultipleType>(parentTy)->getTypes();
                                    for (auto parentTyMtTy : parentTyMtTys) {
                                        if (parentTyMtTy->isA(TypeTypeStaticArray)) {
                                            auto staticArrayTy = std::static_pointer_cast<LILStaticArrayType>(parentTyMtTy);
                                            if (staticArrayTy->getType()->equalTo(vlTy)) {
                                                value->setType(staticArrayTy);
                                                return;
                                            }
                                        } else if (parentTyMtTy->isA(TypeTypeObject) && parentTyMtTy->getName() == "array"){
                                            auto objTy = std::static_pointer_cast<LILObjectType>(parentTyMtTy);
                                            auto paramTy = objTy->getTmplParams().front();
                                            if (paramTy && paramTy->equalTo(vlTy)) {
                                                value->setType(parentTyMtTy);
                                                return;
                                            }
                                        }
                                    }
                                } else if (parentTy->isA(TypeTypeStaticArray)) {
                                    auto staticArrayTy = std::static_pointer_cast<LILStaticArrayType>(parentTy);
                                    if (staticArrayTy->getType()->equalTo(vlTy)) {
                                        value->setType(staticArrayTy);
                                        return;
                                    }
                                } else if (parentTy->isA(TypeTypeObject) && parentTy->getName() == "array"){
                                    auto objTy = std::static_pointer_cast<LILObjectType>(parentTy);
                                    auto paramTy = objTy->getTmplParams().front();
                                    if (paramTy && paramTy->equalTo(vlTy)) {
                                        value->setType(parentTy);
                                        return;
                                    }
                                }
                            }
                            break;
                        }
                        case NodeTypeAssignment:
                        {
                            auto as = std::static_pointer_cast<LILAssignment>(vlParent);
                            const auto & subj = as->getSubject();
                            auto remoteNode = this->recursiveFindNode(subj);
                            if (remoteNode) {
                                auto asTy = remoteNode->getType();
                                if (asTy) {
                                    if (asTy->isA(TypeTypeMultiple)) {
                                        auto asTyMtTys = std::static_pointer_cast<LILMultipleType>(asTy)->getTypes();
                                        for (auto asTyMtTy : asTyMtTys) {
                                            if (asTyMtTy->isA(TypeTypeStaticArray)) {
                                                auto staticArrayTy = std::static_pointer_cast<LILStaticArrayType>(asTyMtTy);
                                                if (staticArrayTy->getType()->equalTo(vlTy)) {
                                                    value->setType(staticArrayTy);
                                                    return;
                                                }
                                            } else if (asTyMtTy->isA(TypeTypeObject) && asTyMtTy->getName() == "array"){
                                                auto objTy = std::static_pointer_cast<LILObjectType>(asTyMtTy);
                                                auto paramTy = objTy->getTmplParams().front();
                                                if (paramTy && paramTy->equalTo(vlTy)) {
                                                    value->setType(asTyMtTy);
                                                    return;
                                                }
                                            }
                                        }
                                    } else if (asTy->isA(TypeTypeStaticArray)) {
                                        auto staticArrayTy = std::static_pointer_cast<LILStaticArrayType>(asTy);
                                        if (staticArrayTy->getType()->equalTo(vlTy)) {
                                            value->setType(staticArrayTy);
                                            return;
                                        }
                                    } else if (asTy->isA(TypeTypeObject) && asTy->getName() == "array"){
                                        auto objTy = std::static_pointer_cast<LILObjectType>(asTy);
                                        auto paramTy = objTy->getTmplParams().front();
                                        if (paramTy && paramTy->equalTo(vlTy)) {
                                            value->setType(asTy);
                                            return;
                                        }
                                    }
                                }
                            } else {
                                std::cerr << "REMOTE NODE NOT FOUND FAIL!!!!!!!\n\n";
                            }
                            break;
                        }
                        default:
                            std::cerr << "UNKNOWN PARENT TYPE FAIL!!!!!!!\n\n";
                            return;
                    }
                    
                }
            }
            if (!vlTy) {
                auto ancestorTy = this->recursiveFindTypeFromAncestors(value->shared_from_this());
                if (ancestorTy) {
                    if (ancestorTy->isA(TypeTypeMultiple)) {
                        for (auto ancestorMtTy : std::static_pointer_cast<LILMultipleType>(ancestorTy)->getTypes()) {
                            if (ancestorMtTy->isA(TypeTypeStaticArray)) {
                                auto staticArrayTy = std::static_pointer_cast<LILStaticArrayType>(ancestorMtTy);
                                value->setType(staticArrayTy);
                                return;
                            } else if (ancestorMtTy->isA(TypeTypeObject) && ancestorMtTy->getName() == "array"){
                                value->setType(ancestorMtTy);
                                return;
                            }
                        }
                    } else {
                        value->setType(ancestorTy);
                    }
                    return;
                }
            }
        }
    }
}

void LILTypeGuesser::_process(LILIndexAccessor * value)
{
}

std::shared_ptr<LILType> LILTypeGuesser::recursiveFindTypeFromAncestors(std::shared_ptr<LILNode> value) const
{
    std::shared_ptr<LILNode> parent = value->getParentNode();
    if (parent) {
        switch (parent->getNodeType()) {
            case NodeTypeFunctionDecl:
            {
                std::shared_ptr<LILType> parentTy = parent->getType();
                if (!parentTy) {
                    parentTy = this->getNodeType(parent);
                }
                std::shared_ptr<LILFunctionType> fnTy = std::dynamic_pointer_cast<LILFunctionType>(parentTy);
                if (fnTy) {
                    return fnTy->getReturnType();
                }
                break;
            }
            case NodeTypeFunctionCall:
            {
                std::shared_ptr<LILFunctionCall> fc = std::static_pointer_cast<LILFunctionCall>(parent);
                auto fcType = fc->getFunctionCallType();
                switch (fcType) {
                    case FunctionCallTypeNone:
                    case FunctionCallTypeValuePath:
                    {
                        auto fnTy = this->findFnTypeForFunctionCall(fc);
                        if (!fnTy) {
                            std::cerr << "FUNCTION TYPE NOT FOUND FAIL!!!!\n\n";
                            return nullptr;
                        }
                        auto args = fnTy->getArguments();
                        if (value->isA(NodeTypeVarDecl)) {
                            auto callArgVd = std::static_pointer_cast<LILVarDecl>(value);
                            
                            for (auto arg : args) {
                                if (!arg->LILNode::isA(NodeTypeVarDecl)) {
                                    std::cerr << "ARG IN FN DEFINITION IS NOT VAR DECL FAIL!!!!\n\n";
                                    return nullptr;
                                }
                                auto argVd = std::static_pointer_cast<LILVarDecl>(arg);
                                if (argVd->getName() == callArgVd->getName()) {
                                    return argVd->getType();
                                }
                            }
                        } else {
                            //get index of element in the decl
                            auto callArgs = fc->getArguments();
                            for (size_t i=0, j=callArgs.size(); i<j; ++i) {
                                if (callArgs[i].get() == value.get()) {
                                    auto argsIndex = i;
                                    if (fcType == FunctionCallTypeValuePath) {
                                        auto firstArg = args[0];
                                        if (firstArg->isA(NodeTypeVarDecl)) {
                                            auto vd = std::static_pointer_cast<LILVarDecl>(firstArg);
                                            if (vd->getName() == "@self") {
                                                argsIndex += 1;
                                            }
                                        }
                                    }
                                    if (args.size() > argsIndex) {
                                        const auto & arg = args[argsIndex];
                                        if (arg->isA(NodeTypeType)) {
                                            return std::static_pointer_cast<LILType>(arg);
                                        } else {
                                            return arg->getType();
                                        }
                                    } else {
                                        return nullptr;
                                    }
                                }
                            }
                        }
                        break;
                    }
                        
                    case FunctionCallTypeSet:
                    {
                        auto args = fc->getArguments();
                        if (args.size() != 2) {
                            std::cerr << "SET NEEDS 2 ARGUMENTS FAIL!!!!\n\n";
                            return nullptr;
                        }
                        auto firstArg = args[0];
                        auto firstTy = this->getNodeType(firstArg);
                        if (firstTy && firstTy->isA(TypeTypePointer)) {
                            auto ptrTy = std::static_pointer_cast<LILPointerType>(firstTy);
                            return ptrTy->getArgument();
                        }
                        return nullptr;
                    }
                        
                    default:
                        break;
                }
            }
                
            case NodeTypeFlowControlCall:
            {
                std::shared_ptr<LILFlowControlCall> fc = std::static_pointer_cast<LILFlowControlCall>(parent);
                if (fc->getFlowControlCallType() == FlowControlCallTypeReturn)
                {
                    auto fun = this->recursiveFindFunctionDecl(fc);
                    if(fun){
                        std::shared_ptr<LILFunctionType> funTy = std::static_pointer_cast<LILFunctionType>(fun->getType());
                        if (funTy) {
                            return funTy->getReturnType();
                        }
                        
                    } else {
                        std::cerr << "Error: return call has no parent function declaration?\n";
                    }
                } else {
                    std::shared_ptr<LILType> parentTy = parent->getType();
                    if (!parentTy) {
                        return this->getNodeType(parent);
                    } else {
                        return parentTy;
                    }
                }
                break;
            }
                
            case NodeTypeAssignment:
            {
                auto asgmt = std::static_pointer_cast<LILAssignment>(parent);

                auto grandpa = asgmt->getParentNode();
                LILString subjectName;
                if (grandpa->isA(NodeTypeObjectDefinition)) {
                    auto subject = asgmt->getSubject();
                    if (subject->isA(NodeTypeValuePath)) {
                        auto vp = std::static_pointer_cast<LILValuePath>(subject);
                        auto firstNode = vp->getNodes().front();
                        if (firstNode->isA(NodeTypePropertyName)) {
                            subjectName = std::static_pointer_cast<LILPropertyName>(firstNode)->getName();
                        } else {
                            std::cerr << "FIRST NODE WAS NOT PROPERTY NAME FAIL!!!!\n\n";
                            return nullptr;
                        }
                    } else if (subject->isA(NodeTypeVarName)) {
                        subjectName = std::static_pointer_cast<LILVarName>(subject)->getName();

                    } else if (subject->isA(NodeTypePropertyName)){
                        subjectName = std::static_pointer_cast<LILPropertyName>(subject)->getName();

                    } else {
                        std::cerr << "UNKNOWN NODE IN SUBJECT OF ASSIGNMENT FAIL!!!!\n\n";
                        return nullptr;
                    }

                    auto objdef = std::static_pointer_cast<LILObjectDefinition>(grandpa);
                    auto objTy = objdef->getType();
                    auto className = objTy->getName();
                    auto classValue = this->findClassWithName(className);
                    if (!classValue) {
                        std::cerr << "COULD NOT FIND CLASS FAIL!!!!\n\n";
                        return nullptr;
                    }
                    auto field = classValue->getFieldNamed(subjectName);
                    if (!field) {
                        field = this->findExpandedField(classValue, subjectName);
                        if(field) {
                            classValue = this->findAncestorClass(field);
                        }
                    }
                    if (!field) {
                        std::cerr << "FIELD NOT FOUND IN CLASS FAIL!!!!!!\n\n";
                        return nullptr;
                    }
                    auto fldTy = field->getType();
                    if (fldTy) {
                        return fldTy;
                    }
                }
                else if (grandpa->isA(NodeTypeEnum))
                {
                    return grandpa->getType();
                }
                else
                {
                    auto subject = asgmt->getSubject();
                    if (subject->isA(NodeTypeVarName)) {
                        return this->findTypeForVarName(std::static_pointer_cast<LILVarName>(subject));
                    } else if (subject->isA(NodeTypeValuePath)) {
                        return this->findTypeForValuePath(std::static_pointer_cast<LILValuePath>(subject));
                    } else if (subject->isA(NodeTypePropertyName)) {
                        return this->findTypeForPropertyName(std::static_pointer_cast<LILPropertyName>(subject));
                    }
                }
                
                break;
            }

            default:
            {
                std::shared_ptr<LILType> parentTy = parent->getType();
                if (!parentTy) {
                    return this->getNodeType(parent);
                } else {
                    return parentTy;
                }
                break;
            }
        }

    } else {
        std::cerr << "Error: number has no parent\n";
    }
    return nullptr;
}

std::shared_ptr<LILFunctionType> LILTypeGuesser::findFnTypeForFunctionCall(std::shared_ptr<LILFunctionCall> fc) const
{
    switch (fc->getFunctionCallType()) {
        case FunctionCallTypeNone:
        {
            auto localNode = this->findNodeForName(fc->getName(), fc->getParentNode().get());
            if (!localNode) {
                std::cerr << "LOCAL VAR NOT FOUND FAIL!!!!\n\n";
                return nullptr;
            }
            auto ty = localNode->getType();
            if (ty->isA(TypeTypePointer)) {
                auto ptrTy = std::static_pointer_cast<LILPointerType>(ty);
                ty = ptrTy->getArgument();
            }
            if (!ty->isA(TypeTypeFunction)) {
                std::cerr << "LOCAL VAR WAS NOT FUNCTION FAIL!!!!\n\n";
                return nullptr;
            }
            return std::static_pointer_cast<LILFunctionType>(ty);
        }
        case FunctionCallTypeValuePath:
        {
            auto vp = fc->getSubject();
            auto subjTy = this->findTypeForValuePath(vp);
            if (!subjTy || !subjTy->isA(TypeTypeObject)) {
                std::cerr << "VAR PATH DOES NOT POINT TO OBJECT FAIL!!!!\n\n";
                return nullptr;
            }
            
            auto classValue = this->findClassWithName(subjTy->getName());
            if (!classValue) {
                std::cerr << "CLASS NOT NOT FOUND FAIL!!!!\n\n";
                return nullptr;
            }
            auto methodNode = classValue->getMethodNamed(fc->getName());
            if (!methodNode) {
                std::cerr << "METHOD NOT NOT FOUND FAIL!!!!\n\n";
                return nullptr;
            }
            if (!methodNode->isA(NodeTypeFunctionDecl)) {
                std::cerr << "METHOD NODE WAS NOT FUNCTION DECL FAIL!!!!\n\n";
                return nullptr;
            }
            auto method = std::static_pointer_cast<LILFunctionDecl>(methodNode);
            return method->getFnType();
        }
        default:
        {
            break;
        }
    }
    return nullptr;
}

void LILTypeGuesser::setTypeOnAncestorIfNeeded(std::shared_ptr<LILNode> value, std::shared_ptr<LILType> ty)
{
    auto parent = value->getParentNode();
    if (parent) {
        switch (parent->getNodeType()) {
            case NodeTypeExpression:
            {
                auto exp = std::static_pointer_cast<LILExpression>(parent);
                if (exp->isA(ExpressionTypeCast)) {
                    break;
                }
                auto expTy = exp->getType();
                if (!expTy || expTy->getIsWeakType()) {
                    exp->setType(ty);
                    this->setTypeOnAncestorIfNeeded(exp, ty);
                }
                break;
            }
            case NodeTypeVarDecl:
            {
                auto vd = std::static_pointer_cast<LILVarDecl>(parent);
                auto vdTy = vd->getType();
                if (!vdTy || vdTy->getIsWeakType()) {
                    vd->setType(ty);
                }
                break;
            }
                
            default:
                break;
        }
    }
}

std::shared_ptr<LILFunctionDecl> LILTypeGuesser::recursiveFindFunctionDecl(std::shared_ptr<LILNode> node) const
{
    std::shared_ptr<LILNode> parent = node->getParentNode();
    if (parent) {
        if (parent->isA(NodeTypeFunctionDecl)) {
            return std::static_pointer_cast<LILFunctionDecl>(parent);
        } else {
            return this->recursiveFindFunctionDecl(parent);
        }
    } else {
        std::cerr << "Error: node has no parent\n";
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::getNodeType(std::shared_ptr<LILNode> node) const
{
    switch (node->getNodeType())
    {
        case NodeTypeBoolLiteral:
        {
            std::shared_ptr<LILType> type = std::make_shared<LILType>();
            type->setName("bool");
            return type;
        }
        case NodeTypeNumberLiteral:
        {
            const auto & tyNode = std::static_pointer_cast<LILNumberLiteral>(node)->getType();
            if (tyNode) {
                return std::static_pointer_cast<LILType>(tyNode);
            } else {
                std::shared_ptr<LILMultipleType> type = std::make_shared<LILMultipleType>();
                std::shared_ptr<LILType> type1 = std::make_shared<LILType>();
                type1->setName("i64");
                type->addType(type1);
                std::shared_ptr<LILType> type2 = std::make_shared<LILType>();
                type2->setName("f64");
                type->addType(type2);
                type->setIsWeakType(true);
                return type;
            }
        }
        case NodeTypeStringLiteral:
        {
            return node->getType();
        }
        case NodeTypeStringFunction:
        {
            std::shared_ptr<LILObjectType> type = std::make_shared<LILObjectType>();
            type->setName("string");
            return type;
        }
        case NodeTypeObjectDefinition:
        {
            std::shared_ptr<LILObjectDefinition> objdef = std::static_pointer_cast<LILObjectDefinition>(node);
            std::shared_ptr<LILType> objdefType = objdef->getType();
            if (objdefType)
            {
                return std::static_pointer_cast<LILType>(objdefType);
            }
            return nullptr;
        }
        case NodeTypeExpression:
        {
            std::shared_ptr<LILExpression> exp = std::static_pointer_cast<LILExpression>(node);
            if (exp->isA(ExpressionTypeCast)) {
                auto right = exp->getRight();
                if (right && right->isA(NodeTypeType)) {
                    return std::static_pointer_cast<LILType>(right);
                }
            }
            return this->getExpType(exp);
        }
        case NodeTypeFunctionDecl:
        {
            std::shared_ptr<LILFunctionDecl> fd = std::static_pointer_cast<LILFunctionDecl>(node);
            switch (fd->getFunctionDeclType()) {
                case FunctionDeclTypeFn:
                {
                    auto ty = fd->getType();
                    if (!ty) {
                        ty = this->getFnType(fd);
                        fd->setType(ty);
                    }
                    return ty;
                }

                default:
                    break;
            }
            break;
        }
        case NodeTypeValuePath:
        {
            std::shared_ptr<LILValuePath> vp = std::static_pointer_cast<LILValuePath>(node);
            return this->findTypeForValuePath(vp);
        }
        case NodeTypeVarName:
        {
            return this->findTypeForVarName(std::static_pointer_cast<LILVarName>(node));
        }
        case NodeTypeVarDecl:
        {
            std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(node);
            std::shared_ptr<LILNode> vdTy = vd->getType();
            if (vdTy && vdTy->isA(NodeTypeType)) {
                return std::static_pointer_cast<LILType>(vdTy);
            }
            break;
        }
        case NodeTypeFunctionCall:
        {
            std::shared_ptr<LILFunctionCall> fc = std::static_pointer_cast<LILFunctionCall>(node);
            std::shared_ptr<LILType> ty = fc->getReturnType();
            if(!ty) {
                ty = this->findReturnTypeForFunctionCall(fc);
            }

            if (ty) {
                return ty;
            } else {
                return this->recursiveFindTypeFromAncestors(fc);
            }

            break;
        }
        case NodeTypeFlowControl:
        {
            return this->recursiveFindTypeFromAncestors(node);
            break;
        }
        case NodeTypePropertyName:
        {
            return this->recursiveFindTypeFromAncestors(node);
        }
        case NodeTypeValueList:
        {
            auto vl = std::static_pointer_cast<LILValueList>(node);
            auto ty = this->findTypeForValueList(vl);
            if (ty && !ty->getIsWeakType()) {
                auto objTy = LILObjectType::make("array");
                objTy->addTmplParam(ty);
                return objTy;
            }
            return nullptr;
        }
        case NodeTypeEnum:
        {
            auto enm = std::static_pointer_cast<LILEnum>(node);
            auto ty = enm->getType();
            if (!ty) {
                ty = this->findTypeForEnum(enm.get());
                if (ty) {
                    enm->setType(ty);
                }
            }
            return ty;
        }
        default:
            return nullptr;
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::getExpType(std::shared_ptr<LILExpression> exp) const
{
    switch (exp->getExpressionType()) {
        case ExpressionTypeSmallerComparison:
        case ExpressionTypeSmallerOrEqualComparison:
        case ExpressionTypeBiggerComparison:
        case ExpressionTypeBiggerOrEqualComparison:
        case ExpressionTypeEqualComparison:
        case ExpressionTypeNotEqualComparison:
        case ExpressionTypeLogicalOr:
        case ExpressionTypeLogicalAnd:
        {
            return LILType::make("bool");
        }
            
        default:
            break;
    }

    //try to find from contents
    std::shared_ptr<LILNode> left = exp->getLeft();
    std::shared_ptr<LILType> leftType = this->getNodeType(left);
    if (left->isA(NodeTypeExpression)) {
        leftType = this->getExpType(std::static_pointer_cast<LILExpression>(left));
    }
    std::shared_ptr<LILNode> right = exp->getRight();
    std::shared_ptr<LILType> rightType = this->getNodeType(right);
    if (right->isA(NodeTypeExpression)) {
        rightType = this->getExpType(std::static_pointer_cast<LILExpression>(right));
    }
    if (leftType && rightType){
        if( leftType->equalTo(rightType)) {
            return leftType;
        }
        if (
            leftType->isA(TypeTypePointer)
            && (
                rightType->getIsWeakType()
                || LILType::combinesWithPointer(rightType.get())
            )
        ) {
            return leftType;
        }
        if (
            rightType->isA(TypeTypePointer)
            && (
                leftType->getIsWeakType()
                || LILType::combinesWithPointer(leftType.get())
            )
        ) {
            return rightType;
        }
        auto mergedTy = LILType::merge(leftType, rightType);
        return mergedTy;
    }
    else if (!leftType && rightType) {
        return rightType;
    }
    if (leftType && !rightType) {
        return leftType;
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeForArg(std::shared_ptr<LILVarDecl> vd, std::shared_ptr<LILFunctionDecl> fd, size_t argCount) const
{
    auto ret = this->getNodeType(vd);
    if (!ret) {
        auto body = fd->getBody();
        ret = this->findTypeFromAssignments(body, vd);
        if (!ret) {
            ret = this->findTypeFromFunctionCalls(body, vd);
            if (!ret) {
                ret = this->findTypeFromExpressions(body, vd);
                if (!ret) {
                    auto fnTy = std::static_pointer_cast<LILFunctionType>(fd->getType());
                    ret = this->findTypeFromCallers(fnTy->getCallers(), vd, argCount);
                    
                    if (!ret) {
                        std::shared_ptr<LILType> anyTy = std::make_shared<LILType>();
                        anyTy->setName("any");
                        ret = anyTy;
                    }
                }
            }
        }
    }
    return ret;
}

std::shared_ptr<LILType> LILTypeGuesser::getFnType(std::shared_ptr<LILFunctionDecl> fd) const
{
    std::shared_ptr<LILFunctionType> ret = std::make_shared<LILFunctionType>();
    auto ty = std::static_pointer_cast<LILFunctionType>(fd->getType());
    ret->setName("fn");
    size_t argCount = 0;
    for (const auto & arg : ty->getArguments()) {
        if (!arg->LILNode::isA(NodeTypeVarDecl)) {
            ++argCount;
            continue;
        }
        std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(arg);
        auto argType = this->findTypeForArg(vd, fd, argCount);
        if (argType) {
            argType = this->nullsToNullableTypes(argType);
            ret->addArgument(argType);
            vd->setType(argType);
        }
        ++argCount;
    }

    auto evals = fd->getBody();
    auto existingReturnType = fd->getReturnType();
    if (existingReturnType) {
        existingReturnType = this->nullsToNullableTypes(existingReturnType);
        ret->setReturnType(existingReturnType);
    } else {
        auto returnType = this->getFnReturnType(evals);
        ret->setReturnType(returnType);
    }
    return ret;
}

std::shared_ptr<LILType> LILTypeGuesser::getFnReturnType(const std::vector<std::shared_ptr<LILNode>> & nodes) const
{
    std::vector<std::shared_ptr<LILType>> returnTypes;
    std::shared_ptr<LILType> returnType;
    for (auto it = nodes.rbegin(); it!=nodes.rend(); ++it) {
        this->recursiveFindReturnTypes(returnTypes, *it);
    }
    if (returnTypes.size() > 1) {
        returnType = returnTypes.front();
        for (size_t i=1,j=returnTypes.size(); i<j; ++i){
            returnType = LILType::merge(returnType, returnTypes[i]);
        }
    } else if (returnTypes.size() == 1){
        returnType = returnTypes.back();
    }
    
    if (returnType) {
        if (returnType->getIsWeakType()) {
            auto intType = returnType->getDefaultType();
            returnType = intType;
        }
        
        returnType = this->nullsToNullableTypes(returnType);
    }
    return returnType;
}

void LILTypeGuesser::recursiveFindReturnTypes(std::vector<std::shared_ptr<LILType>> & returnTypes, std::shared_ptr<LILNode> eval) const
{
    switch (eval->getNodeType()) {
        case NodeTypeFlowControlCall:
        {
            std::shared_ptr<LILFunctionCall> fc = std::static_pointer_cast<LILFunctionCall>(eval);
            if (fc->getFlowControlCallType() == FlowControlCallTypeReturn) {
                auto args = fc->getArguments();
                for (auto it = args.rbegin(); it != args.rend(); ++it) {
                    auto arg = *it;
                    switch (arg->getNodeType()) {
                        case NodeTypeBoolLiteral:
                        {
                            std::shared_ptr<LILType> type = std::make_shared<LILType>();
                            type->setName("bool");
                            returnTypes.push_back(type);
                            break;
                        }
                        case NodeTypeNumberLiteral:
                        {
                            std::shared_ptr<LILType> type = std::static_pointer_cast<LILNumberLiteral>(arg)->getType();
                            if (type) {
                                returnTypes.push_back(type);
                            }
                            break;
                        }
                        case NodeTypeStringLiteral:
                        {
                            std::shared_ptr<LILObjectType> type = std::make_shared<LILObjectType>();
                            type->setName("string");
                            returnTypes.push_back(type);
                            break;
                        }
                        case NodeTypeExpression:
                        {
                            std::shared_ptr<LILType> type = this->getExpType(std::static_pointer_cast<LILExpression>(arg));
                            if (type) {
                                this->addTypeToReturnTypes(returnTypes, type);
                            }
                            break;
                        }
                        case NodeTypeVarName:
                        {
                            std::shared_ptr<LILVarName> vn = std::static_pointer_cast<LILVarName>(arg);
                            auto type = this->findTypeForVarName(vn);
                            if (type) {
                                this->addTypeToReturnTypes(returnTypes, type);
                            }
                            break;
                        }
                        case NodeTypeValuePath:
                        {
                            std::shared_ptr<LILValuePath> vp = std::static_pointer_cast<LILValuePath>(arg);
                            auto vpTy = this->findTypeForValuePath(vp);
                            if (vpTy) {
                                this->addTypeToReturnTypes(returnTypes, vpTy);
                            }
                            break;
                        }
                        case NodeTypeFunctionCall:
                        {
                            std::shared_ptr<LILType> type = this->findReturnTypeForFunctionCall(std::static_pointer_cast<LILFunctionCall>(arg));
                            if (type) {
                                this->addTypeToReturnTypes(returnTypes, type);
                            }
                            break;
                        }
                        case NodeTypeObjectDefinition:
                        {
                            returnTypes.push_back(arg->getType()->clone());
                            break;
                        }
                        default:
                            std::cerr << "!!!!!!!!!!GUESSER FAIL!!!!!!!!!!!!!!!!\n";
                            break;
                    }
                }
            }
            break;
        }

        case NodeTypeFlowControl:{
            std::shared_ptr<LILFlowControl> fc = std::static_pointer_cast<LILFlowControl>(eval);
            for (const auto & eval : fc->getThen()) {
                this->recursiveFindReturnTypes(returnTypes, eval);
            }
            for (const auto & eval : fc->getElse()) {
                this->recursiveFindReturnTypes(returnTypes, eval);
            }
        }
        default:
            break;
    }
}

void LILTypeGuesser::addTypeToReturnTypes(std::vector<std::shared_ptr<LILType>> & returnTypes, std::shared_ptr<LILType> ty) const
{
    switch (ty->getTypeType()) {
        case TypeTypeSingle:
        case TypeTypeObject:
        case TypeTypePointer:
        case TypeTypeStaticArray:
        {
            returnTypes.push_back(ty);
            break;
        }
        case TypeTypeFunction:
        {
            auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
            auto retTy = fnTy->getReturnType();
            if (retTy) {
                returnTypes.push_back(retTy);
            } else {
                std::cerr << "!!!!!!!!!!GUESSER FAIL!!!!!!!!!!!!!!!!\n";
            }
            break;
        }
        case TypeTypeMultiple:
        {
            if (!ty->getIsWeakType()) {
                for ( auto retSubTy : std::static_pointer_cast<LILMultipleType>(ty)->getTypes() ) {
                    returnTypes.push_back(retSubTy);
                }
            } else {
                std::cerr << "!!!!!!!!!!GUESSER FAIL!!!!!!!!!!!!!!!!\n";
            }
        }
        case TypeTypeNone:
            break;
    }
}

std::shared_ptr<LILType> LILTypeGuesser::findReturnTypeForFunctionCall(std::shared_ptr<LILFunctionCall> fc) const
{
    switch (fc->getFunctionCallType()) {
        case FunctionCallTypeNone:
        {
            auto localNode = this->findNodeForName(fc->getName(), fc->getParentNode().get());
            if (
                localNode
                && (localNode->isA(NodeTypeVarDecl) || localNode->isA(NodeTypeFunctionDecl))
            ) {
                auto ty = localNode->getType();
                if (ty && ty->isA(TypeTypePointer)) {
                    auto fnPtr = std::static_pointer_cast<LILPointerType>(ty);
                    ty = fnPtr->getArgument();
                }
                if (!ty || !ty->isA(TypeTypeFunction)) {
                    break;
                }
                auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                return fnTy->getReturnType();
            }
            break;
        }
        case FunctionCallTypeValuePath:
        {
            auto parent = fc->getParentNode();
            if (parent && parent->isA(NodeTypeValuePath)) {
                return this->getNodeType(parent);
            }
            break;
        }
            
        case FunctionCallTypeValueOf:
        {
            auto firstArg = fc->getArguments().front();
            auto firstArgType = this->getNodeType(firstArg);
            if (firstArgType->isA(TypeTypePointer)) {
                auto pointerTy = std::static_pointer_cast<LILPointerType>(firstArgType);
                return pointerTy->getArgument();
            }
            
            break;
        }
            
        case FunctionCallTypeSizeOf:
        {
            return LILType::make("i64");
        }
            
        case FunctionCallTypePointerTo:
        {
            auto firstArg = fc->getArguments().front();
            auto firstArgType = this->getNodeType(firstArg);
            auto newPtrTy = std::make_shared<LILPointerType>();
            newPtrTy->setName("ptr");
            newPtrTy->setArgument(firstArgType);
            return newPtrTy;
        }
            
        default:
            break;
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeForVarName(std::shared_ptr<LILVarName> name) const
{
    std::shared_ptr<LILNode> parent = name->getParentNode();
    LILString nameStr = name->getName();
    std::shared_ptr<LILType> ret;
    while (parent) {
        if(parent->isVarNode()){
            std::shared_ptr<LILVarNode> vn = std::static_pointer_cast<LILVarNode>(parent);
            std::shared_ptr<LILNode> localVar = vn->getLocalVariable(nameStr);
            if (localVar) {
                ret = this->getNodeType(localVar);
                break;
            }
        }
        parent = parent->getParentNode();
    }
    if (ret && !this->inhibitSearchingForIfCastType && (ret->isA(TypeTypeMultiple) || ret->getIsNullable())) {
        parent = name->getParentNode();
        while (parent) {
            if (parent->isA(FlowControlTypeIfCast)) {
                auto fc = std::static_pointer_cast<LILFlowControl>(parent);
                auto args = fc->getArguments();
                if (args.size() != 2) {
                    break;
                }
                auto firstArg = args.front();
                if (name->equalTo(firstArg)) {
                    auto ifCastTy = args.back();
                    ret = std::static_pointer_cast<LILType>(ifCastTy);
                }
            }
            parent = parent->getParentNode();
        }
    }
    return ret;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeForValuePath(std::shared_ptr<LILValuePath> vp) const
{
    auto nodes = vp->getNodes();
    std::shared_ptr<LILNode> currentNode;
    std::shared_ptr<LILType> currentTy;
    if (nodes.size() == 1) {
        currentNode = nodes.front();
        if (currentNode->isA(NodeTypeVarName)) {
            auto remoteNode = this->recursiveFindNode(currentNode);
            if (remoteNode) {
                return this->getNodeType(remoteNode);
            }
        }
    } else if (nodes.size() > 1){
        size_t startIndex = 1;
        std::shared_ptr<LILType> ifCastType;
        if (!this->inhibitSearchingForIfCastType) {
            ifCastType = this->findIfCastType(vp.get(), startIndex);
        }
        if (ifCastType) {
            currentTy = ifCastType;
            if (startIndex == nodes.size()) {
                return currentTy;
            }

        } else {
            currentNode = nodes.front();
            if (currentNode->isA(NodeTypeVarName)) {
                currentNode = this->recursiveFindNode(currentNode);
                if (currentNode) {
                    currentTy = currentNode->getType();
                    if (!currentTy) {
                        currentTy = this->getNodeType(currentNode);
                    }
                }
                if (!currentTy) {
                    std::cerr << "SUBJ TY WAS NULL FAIL!!!!\n";
                    return nullptr;
                }
            } else if (currentNode->isA(NodeTypePropertyName)) {
                auto parentNode = vp->getParentNode();
                if (parentNode->isA(NodeTypeAssignment)) {
                    auto grandpaNode = parentNode->getParentNode();
                    if (grandpaNode->isA(NodeTypeObjectDefinition)) {
                        auto objDef = std::static_pointer_cast<LILObjectDefinition>(grandpaNode);
                        currentTy = objDef->getType();
                        startIndex = 0;
                    }
                }
                if (!currentTy) {
                    std::cerr << "COULD NOT FIND TYPE FOR PROPERTY NAME FAIL!!!!\n";
                    return nullptr;
                }
            }
            else if (currentNode->isA(SelectorTypeSelfSelector)) {
                auto classDecl = this->findAncestorClass(currentNode);
                currentNode = classDecl;
                currentTy = classDecl->getType();
            }
        }

        bool isLast = false;
        for (size_t i=startIndex, j=nodes.size(); i<j; ++i) {
            isLast = i==j-1;
            auto node = nodes[i];
            switch (node->getNodeType()) {
                case NodeTypePropertyName:
                {
                    auto pn = std::static_pointer_cast<LILPropertyName>(node);
                    auto pnName = pn->getName();
                    if (currentTy->isA(TypeTypePointer)) {
                        auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
                        currentTy = ptrTy->getArgument();
                    }
                    auto currentNodeTy = currentNode->getNodeType();
                    if (currentNodeTy == NodeTypeClassDecl || currentNodeTy == NodeTypeVarDecl ) {
                        if (currentTy->getTypeType() != TypeTypeObject) {
                            std::cerr << "CURRENT TY WAS NOT OBJECT TY FAIL!!!!\n";
                            return nullptr;
                        }
                        auto className = currentTy->getName().data();
                        auto classDecl = this->findClassWithName(className);
                        if (!classDecl) {
                            std::cerr << "CLASS "+className+" NOT FOUND FAIL!!!!\n";
                            return nullptr;
                        }
                        auto field = classDecl->getFieldNamed(pnName);
                        if (!field) {
                            field = this->findExpandedField(classDecl, pnName);
                            if(field) {
                                classDecl = this->findAncestorClass(field);
                            }
                        }
                        if (!field) {
                            std::cerr << "FIELD WAS NULL FAIL!!!!\n";
                            return nullptr;
                        }
                        if (field->getNodeType() == NodeTypeVarDecl) {
                            auto vd = std::static_pointer_cast<LILVarDecl>(field);
                            if (vd->getIsVVar()) {
                                auto retTy = vd->getReturnType();
                                if (retTy) {
                                    currentTy = retTy;
                                    break;
                                }
                            }
                        }
                        auto fieldTy = this->getNodeType(field);
                        currentTy = fieldTy;
                        
                    } else if (currentNode->getNodeType() == NodeTypeEnum) {
                        auto enm = std::static_pointer_cast<LILEnum>(currentNode);
                        auto enumTy = enm->getType();
                        if (!enumTy) {
                            enumTy = this->findTypeForEnum(enm.get());
                        }
                        return enumTy;
                    }
                    
                    break;
                }
                case NodeTypeFunctionCall:
                {
                    auto fc = std::static_pointer_cast<LILFunctionCall>(node);
                    if (currentTy->isA(TypeTypePointer)) {
                        auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
                        currentTy = ptrTy->getArgument();
                    }
                    auto className = currentTy->getName().data();
                    if (!currentTy->isA(TypeTypeObject)) {
                        std::cerr << "TYPE " << className << " IS NOT OBJECT TYPE FAIL!!!!\n";
                        return nullptr;
                    }
                    auto classDecl = this->findClassWithName(className);
                    if (!classDecl) {
                        std::cerr << "CLASS " << className << " NOT FOUND FAIL!!!!\n";
                        return nullptr;
                    }
                    auto methodName = fc->getName();
                    auto methodNode = classDecl->getMethodNamed(methodName);
                    if (!methodNode) {
                        std::cerr << "METHOD " << methodName.data() << " NOT FOUND FAIL!!!!\n";
                        return nullptr;
                    }
                    if (!methodNode->isA(NodeTypeFunctionDecl)) {
                        std::cerr << "METHOD NODE IS NOT FUNCTION DECL FAIL!!!!\n";
                        return nullptr;
                    }
                    auto method = std::static_pointer_cast<LILFunctionDecl>(methodNode);
                    auto fnTy = method->getFnType();
                    auto retTy = fnTy->getReturnType();
                    if (retTy) {
                        currentTy = retTy;
                    } else {
                        retTy = this->getFnReturnType(method->getBody());
                        if (retTy) {
                            fnTy->setReturnType(retTy);
                            currentTy = retTy;
                        } else {
                            currentTy = nullptr;
                            if (!isLast) {
                                std::cerr << "Trying to traverse through function "+methodName.data()+"() which does not return any value\n";
                            }
                        }
                    }
                    break;
                }
                case NodeTypeIndexAccessor:
                {
                    if (currentTy->isA(TypeTypeObject)) {
                        const auto & className = currentTy->getName();
                        auto cd = this->findClassWithName(className);
                        if (!cd) {
                            std::cerr << "CLASS " + className.data() + " NOT FOUND FAIL!!!!\n";
                            return nullptr;
                        }
                        auto method = cd->getMethodNamed("at");
                        if (!method) {
                            std::cerr << "CLASS " + className.data() + " HAD NOT at METHOD FAIL!!!!\n";
                            return nullptr;
                        }
                        auto methodTy = method->getType();
                        if (!methodTy || !methodTy->isA(TypeTypeFunction)) {
                            std::cerr << "BAD AT METHOD FAIL!!!!\n";
                            return nullptr;
                        }
                        auto mFnTy = std::static_pointer_cast<LILFunctionType>(methodTy);
                        auto retTy = mFnTy->getReturnType();
                        if (!retTy) {
                            std::cerr << "FN TYPE HAD NO RETURN TY FAIL!!!!\n";
                            return nullptr;
                        }
                        currentTy = retTy;
                        break;
                    }
                    else if (!currentTy->isA(TypeTypeStaticArray))
                    {
                        std::cerr << "FIELD TYPE IS NOT ARRAY TYPE FAIL!!!!\n";
                        return nullptr;
                    }
                    auto saTy = std::static_pointer_cast<LILStaticArrayType>(currentTy);
                    currentTy = saTy->getType();
                    break;
                }

                default:
                    std::cerr << "!!!!!!!!!!VALUE PATH NODE TYPE FAIL!!!!!!!!!!!!!!!!\n";
                    break;
            }
            if (isLast) {
                return currentTy;
            }
        }
    }
    
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeForPropertyName(std::shared_ptr<LILPropertyName> name) const
{
    auto remoteNode = this->findNodeForPropertyName(name.get());
    if (remoteNode) {
        return remoteNode->getType();
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeForSelectorChain(std::shared_ptr<LILSelectorChain> selCh) const
{
    const auto & selNodes = selCh->getNodes();
    if (selNodes.size() == 1) {
        const auto & sSelNode = selNodes.front();
        if (sSelNode->getNodeType() != NodeTypeSimpleSelector) {
            std::cerr << "NODE IN SELECTOR CHAIN WAS NOT SIMPLE SELECTOR FAIL!!!!!!!!!!!!!!!!!!!!\n\n";
            return nullptr;
        }
        
        auto simpleSel = std::static_pointer_cast<LILSimpleSelector>(sSelNode);
        const auto & firstSimpleSel = simpleSel->getNodes().front();
        switch (firstSimpleSel->getSelectorType()) {
            case SelectorTypeRootSelector:
            {
                auto objTy = std::make_shared<LILObjectType>();
                objTy->setName("container");
                return objTy;
            }
                
            default:
                //std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n";
                break;
        }
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeFromAssignments(std::vector<std::shared_ptr<LILNode>> nodes, const std::shared_ptr<LILVarDecl> & vd) const
{
    for (auto node : nodes) {
        if (!node->isA(NodeTypeAssignment)) {
            continue;
        }
        auto asgmt = std::static_pointer_cast<LILAssignment>(node);
        auto value = asgmt->getValue();
        auto ty = this->getNodeType(value);
        if (ty) {
            return ty;
        }
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeFromFunctionCalls(std::vector<std::shared_ptr<LILNode>> nodes, const std::shared_ptr<LILVarDecl> & vd) const
{
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeFromExpressions(std::vector<std::shared_ptr<LILNode>> nodes, const std::shared_ptr<LILVarDecl> & vd) const
{
    std::vector<std::shared_ptr<LILType>> types;
    for (const auto & node : nodes)
    {
        switch (node->getNodeType()) {
            case NodeTypeFunctionCall:
            {
                auto fc = std::static_pointer_cast<LILFunctionCall>(node);
                auto fcType = this->findTypeFromExpressions(fc->getArguments(), vd);
                if (fcType) {
                    return fcType;
                }
                break;
            }
            case NodeTypeExpression:
            {
                bool isTypeGiver = false;
                std::shared_ptr<LILExpression> exp = std::static_pointer_cast<LILExpression>(node);
                std::shared_ptr<LILNode> left = exp->getLeft();
                std::shared_ptr<LILType> leftType;
                if (left->isA(NodeTypeExpression)) {
                    auto leftExp = std::static_pointer_cast<LILExpression>(left);
                    leftType = this->findTypeFromExpressions(leftExp->getNodes(), vd);
                } else if (left->isA(NodeTypeValuePath)){
                    std::shared_ptr<LILValuePath> vp = std::static_pointer_cast<LILValuePath>(left);
                    std::shared_ptr<LILNode> firstVPNode = vp->getNodes().front();
                    if (firstVPNode->isA(NodeTypeVarName)) {
                        std::shared_ptr<LILVarName> varName = std::static_pointer_cast<LILVarName>(firstVPNode);
                        if (varName->getName() == vd->getName()) {
                            isTypeGiver = true;
                        }
                    }
                } else {
                    leftType = this->getNodeType(left);
                }
                std::shared_ptr<LILNode> right = exp->getRight();
                std::shared_ptr<LILType> rightType;
                if (right->isA(NodeTypeExpression)) {
                    auto rightExp = std::static_pointer_cast<LILExpression>(right);
                    rightType = this->findTypeFromExpressions(rightExp->getNodes(), vd);
                } else if (right->isA(NodeTypeValuePath)){
                    std::shared_ptr<LILValuePath> vp = std::static_pointer_cast<LILValuePath>(right);
                    std::shared_ptr<LILNode> firstVPNode = vp->getNodes().front();
                    if (firstVPNode->isA(NodeTypeVarName)) {
                        std::shared_ptr<LILVarName> varName = std::static_pointer_cast<LILVarName>(firstVPNode);
                        if (varName->getName() == vd->getName()) {
                            isTypeGiver = true;
                        }
                    }
                } else {
                    rightType = this->getNodeType(right);
                }

                if (isTypeGiver) {
                    if (leftType) {
                        types.push_back(leftType);
                    } else if (rightType){
                        types.push_back(rightType);
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    if (types.size() == 1) {
        return types.back();
    } else if (types.size() > 1){
        //convert to multiple type
    }

    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeFromCallers(const std::vector<std::shared_ptr<LILNode>> & nodes, const std::shared_ptr<LILVarDecl> & vd, size_t argCount) const
{
    std::shared_ptr<LILType> ret;
    for (auto & node : nodes) {
        if (!node->isA(NodeTypeFunctionCall)) {
            continue;
        }
        auto fc = std::static_pointer_cast<LILFunctionCall>(node);
        size_t i = 0;
        for (auto & arg : fc->getArguments()) {
            if (arg->isA(NodeTypeAssignment)) {
                auto asgmt = std::static_pointer_cast<LILAssignment>(arg);
                auto subj = asgmt->getSubject();
                if (!subj->isA(NodeTypeVarName)) {
                    std::cerr << "!!!!!!!!!!SUBJECT OF ASSIGNMENT WAS NOT VAR NAME FAIL!!!!!!!!!!!!!!!!\n";
                }
                auto vn = std::static_pointer_cast<LILVarName>(subj);
                if (vd->getName() == vn->getName()) {
                    ret = this->getNodeType(asgmt->getValue());
                }
            } else if (i == argCount) {
                ret = this->getNodeType(arg);
            }
            ++i;
        }
        if (ret) {
            break;
        }
    }
    return ret;
}

std::shared_ptr<LILType> LILTypeGuesser::nullsToNullableTypes(std::shared_ptr<LILType> ty) const
{
    if (ty->isA(TypeTypeMultiple)) {
        auto multiTy = std::static_pointer_cast<LILMultipleType>(ty);
        bool isNullable = false;
        std::vector<std::shared_ptr<LILType>> newTypes;
        for (auto mTy : multiTy->getTypes()) {
            if (mTy->getName() == "null") {
                isNullable = true;
            } else {
                newTypes.push_back(mTy);
            }
        }
        if (isNullable) {
            if (newTypes.size() == 1) {
                auto theType = newTypes.front();
                
                if (theType->isA(TypeTypePointer) || (theType->getName() == "bool")) {
                    theType->setIsNullable(true);
                    ty = theType;
                } else {
                    multiTy->setIsNullable(true);
                    multiTy->setTypes(std::move(newTypes));
                }
            } else {
                multiTy->setIsNullable(true);
                multiTy->setTypes(std::move(newTypes));
            }
        }
    }
    return ty;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeForValueList(std::shared_ptr<LILValueList> value) const
{
    std::vector<std::shared_ptr<LILType>> types;
    for (const auto & val : value->getValues()) {
        const auto & ty = val->getType();
        bool found = false;
        for (const auto existingTy : types) {
            if (existingTy->equalTo(ty)) {
                found = true;
                break;
            }
        }
        if (!found) {
            types.push_back(ty);
        }
    }
    if (types.size() == 1) {
        return *(types.begin());
    }
    auto mTy = std::make_shared<LILMultipleType>();
    for (const auto ty : types) {
        mTy->addType(ty);
    }
    
    return mTy;
}


std::shared_ptr<LILType> LILTypeGuesser::findTypeForEnum(LILEnum * value) const
{
    std::shared_ptr<LILType> ret;
    for (const auto & val : value->getValues()) {
        auto ty = val->getType();
        if (!ty) {
            auto valTy = this->getNodeType(val);
            if (valTy) {
                ty = valTy;
            }
        }
        if (ty) {
            if (!ret) {
                ret = ty;
            } else if (!ret->equalTo(ty)) {
                if (ty->getBitWidth() > ret->getBitWidth()) {
                    ret = ty;
                }
            }
        }
    }
    return ret;
}
