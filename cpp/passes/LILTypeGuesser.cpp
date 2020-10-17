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
#include "LILVarNode.h"
#include "LILObjectType.h"

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
        std::cerr << "==== VAR TYPE GUESSING  ====\n";
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
        std::cerr << "## propagate strong types of " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }

    if (node->isA(NodeTypeVarDecl)) {
        auto ty = node->getType();
        if (ty && !ty->isA(TypeTypeFunction)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            for (auto initVal : vd->getInitVals()) {
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
            exp->setType(ty);
            this->_propagateStrongType(exp->getLeft(), ty);
            this->_propagateStrongType(exp->getRight(), ty);
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
                auto ivTy = this->getNodeType(initVal);
                if (ivTy) {
                    vd->setType(ivTy);
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
        if (vd->getInitVals().size() == 0) {
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
        std::cerr << "## guessing types " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBool:
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
        case NodeTypeType:
        {
            break;
        }

        default:
            std::cerr << "Error: unkonwn node type to process\n";
            break;
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
        auto ty3 = LILType::merge(ty1, ty2);
        if (ty3) {
            if (ty3->getIsWeakType()) {
                auto multiTy = std::static_pointer_cast<LILMultipleType>(ty3);
                value->setType(multiTy->getTypes().front());
            } else {
                value->setType(ty3);
                this->setTypeOnAncestorIfNeeded(sharedVal, ty3);
            }
        }
    }
}

void LILTypeGuesser::_process(LILPercentageLiteral * value)
{

}

void LILTypeGuesser::_process(LILExpression * value)
{
    auto existingTy = value->getType();
    if (existingTy) {
        return;
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

    if (left->isTypedNode() && leftTy->getIsWeakType()) {
        const auto & leftTN = std::static_pointer_cast<LILTypedNode>(left);
        leftTN->setType(ty);
    }
    if (right->isTypedNode() && rightTy->getIsWeakType()) {
        const auto & rightTN = std::static_pointer_cast<LILTypedNode>(right);
        rightTN->setType(ty);
    }
    value->setType(ty);
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
            auto type = this->getNodeType(initValue);
            if(type)
                value->setType(type);
        }
    }
    auto ty = value->getType();
    if (ty && ty->isA(TypeTypeMultiple)) {
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
                theType->setIsNullable(true);
                value->setType(theType);
            } else {
                multiTy->setIsNullable(true);
                multiTy->setTypes(newTypes);
            }
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

}

void LILTypeGuesser::_process(LILPropertyName * value)
{
}

void LILTypeGuesser::_process(LILRule * value)
{

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
}

void LILTypeGuesser::_process(LILFunctionDecl * value)
{
    auto ty = value->getType();
    if (!ty) {
        std::shared_ptr<LILNode> type = this->getNodeType(value->shared_from_this());
        if(type && type->isA(NodeTypeType)){
            std::shared_ptr<LILFunctionType> ft = std::dynamic_pointer_cast<LILFunctionType>(type);
            if (ft) {
                value->setType(ft);
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

}

void LILTypeGuesser::_process(LILFlowControlCall * value)
{
    
}

void LILTypeGuesser::_process(LILInstruction * value)
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
                
                switch (fc->getFunctionCallType()) {
                    case FunctionCallTypeNone:
                    case FunctionCallTypeValuePath:
                    {
                        auto fnTy = this->findFnTypeForFunctionCall(fc);
                        if (!fnTy) {
                            std::cerr << "FUNCTION TYPE NOT FOUND FAIL!!!!";
                            return nullptr;
                        }
                        auto args = fnTy->getArguments();
                        if (value->isA(NodeTypeVarDecl)) {
                            auto callArgVd = std::static_pointer_cast<LILVarDecl>(value);
                            
                            for (auto arg : args) {
                                if (!arg->LILNode::isA(NodeTypeVarDecl)) {
                                    std::cerr << "ARG IN FN DEFINITION IS NOT VAR DECL FAIL!!!!";
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
                                    return args[i]->getType();
                                }
                            }
                        }
                        break;
                    }
                        
                    case FunctionCallTypeSet:
                    {
                        auto args = fc->getArguments();
                        if (args.size() != 2) {
                            std::cerr << "SET NEEDS 2 ARGUMENTS FAIL!!!!";
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
                            std::cerr << "FIRST NODE WAS NOT PROPERTY NAME FAIL!!!!";
                            return nullptr;
                        }
                        
                    } else if (subject->isA(NodeTypePropertyName)){
                        subjectName = std::static_pointer_cast<LILPropertyName>(subject)->getName();

                    } else {
                        std::cerr << "UNKNOWN NODE IN SUBJECT OF ASSIGNMENT FAIL!!!!";
                        return nullptr;
                    }

                    auto objdef = std::static_pointer_cast<LILObjectDefinition>(grandpa);
                    auto objTy = objdef->getType();
                    auto className = objTy->getName();
                    auto classValue = this->findClassWithName(className);
                    if (!classValue) {
                        std::cerr << "COULD NOT FIND CLASS FAIL!!!!";
                        return nullptr;
                    }
                    for (auto field : classValue->getFields()) {
                        if (field->isA(NodeTypeVarDecl)) {
                            auto vd = std::static_pointer_cast<LILVarDecl>(field);
                            if (vd->getName() == subjectName) {
                                auto vdTy = vd->getType();
                                if (vdTy) {
                                    return vdTy;
                                }
                            }
                            
                        } else {
                            std::cerr << "FIELD WAS NOT VAR DECL FAIL!!!!";
                            return nullptr;
                        }
                    }
                
                } else {
                    //std::cerr << "ASSIGNMENT INSIDE UNKNOWN PARENT FAIL!!!!";
                    return nullptr;
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
                std::cerr << "LOCAL VAR NOT FOUND FAIL!!!!";
                return nullptr;
            }
            auto ty = localNode->getType();
            if (!ty->isA(TypeTypeFunction)) {
                std::cerr << "LOCAL VAR WAS NOT FUNCTION FAIL!!!!";
                return nullptr;
            }
            return std::static_pointer_cast<LILFunctionType>(ty);
        }
        case FunctionCallTypeValuePath:
        {
            auto vp = fc->getSubject();
            auto subjTy = this->findTypeForValuePath(vp);
            if (!subjTy->isA(TypeTypeObject)) {
                std::cerr << "VAR PATH DOES NOT POINT TO OBJECT FAIL!!!!";
                return nullptr;
            }
            
            auto classValue = this->findClassWithName(subjTy->getName());
            if (!classValue) {
                std::cerr << "CLASS NOT NOT FOUND FAIL!!!!";
                return nullptr;
            }
            auto method = classValue->getMethodNamed(fc->getName());
            if (!method) {
                std::cerr << "METHOD NOT NOT FOUND FAIL!!!!";
                return nullptr;
            }
            auto methTy = method->getType();
            if (!methTy->isA(TypeTypeFunction)) {
                std::cerr << "TYPE WAS NOT FUNCTION TYPE FAIL!!!!";
                return nullptr;
            }
            auto fnTy = std::static_pointer_cast<LILFunctionType>(methTy);
            return fnTy;
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
                if (!exp->getType()) {
                    exp->setType(ty);
                    this->setTypeOnAncestorIfNeeded(exp, ty);
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
        case NodeTypeBool:
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
            auto strLit = std::static_pointer_cast<LILStringLiteral>(node);
            std::shared_ptr<LILType> type = std::make_shared<LILType>();
            if (strLit->getIsCString()) {
                type->setName("cstr");
            } else {
                type->setName("str");
            }
            return type;
        }
        case NodeTypeStringFunction:
        {
            std::shared_ptr<LILType> type = std::make_shared<LILType>();
            type->setName("str");
            return type;
        }
        case NodeTypeArray:
        {
            std::shared_ptr<LILType> type = std::make_shared<LILType>();
            type->setName("arr");
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
        default:
            return nullptr;
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::getExpType(std::shared_ptr<LILExpression> exp) const
{
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
    std::vector<std::shared_ptr<LILType>> returnTypes;
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
            ret->addArgument(argType);
            vd->setType(argType);
        }
        ++argCount;
    }

    auto evals = fd->getBody();
    auto existingReturnType = fd->getReturnType();
    if (existingReturnType) {
        ret->setReturnType(existingReturnType);
    } else {
        for (auto it = evals.rbegin(); it!=evals.rend(); ++it) {
            this->recursiveFindReturnTypes(returnTypes, *it);
        }
        std::shared_ptr<LILType> returnType;
        if (returnTypes.size() > 1) {
            returnType = returnTypes.front();
            for (size_t i=1,j=returnTypes.size(); i<j; ++i){
                returnType = LILType::merge(returnType, returnTypes[i]);
            }
        } else if (returnTypes.size() == 1){
            returnType = returnTypes.back();
        } else {
            std::shared_ptr<LILType> nullTy = std::make_shared<LILType>();
            nullTy->setName("null");
            returnType = nullTy;
        }
        
        if (returnType->getIsWeakType()) {
            auto intType = std::make_shared<LILType>();
            intType->setName("i64");
            returnType = intType;
        }
        ret->setReturnType(returnType);
    }
    return ret;
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
                        case NodeTypeBool:
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
                            std::shared_ptr<LILType> type = std::make_shared<LILType>();
                            type->setName("str");
                            returnTypes.push_back(type);
                            break;
                        }
                        case NodeTypeExpression:
                        {
                            std::shared_ptr<LILType> type = this->getExpType(std::static_pointer_cast<LILExpression>(arg));
                            if (type) {
                                returnTypes.push_back(type);
                            }
                            break;
                        }
                        case NodeTypeValuePath:
                        {
                            std::shared_ptr<LILValuePath> vp = std::static_pointer_cast<LILValuePath>(arg);
                            auto vpTy = this->findTypeForValuePath(vp);
                            if (vpTy) {
                                switch (vpTy->getTypeType()) {
                                    case TypeTypeSingle:
                                    {
                                        returnTypes.push_back(vpTy);
                                        break;
                                    }
                                    case TypeTypeFunction:
                                    {
                                        auto fnTy = std::static_pointer_cast<LILFunctionType>(vpTy);
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
                                        if (!vpTy->getIsWeakType()) {
                                            for ( auto retSubTy : std::static_pointer_cast<LILMultipleType>(vpTy)->getTypes() ) {
                                                returnTypes.push_back(retSubTy);
                                            }
                                        } else {
                                            std::cerr << "!!!!!!!!!!GUESSER FAIL!!!!!!!!!!!!!!!!\n";
                                        }
                                    }
                                    default:
                                        break;
                                }
                            }
                            break;
                        }
                        case NodeTypeFunctionCall:
                        {
                            std::shared_ptr<LILType> type = this->findReturnTypeForFunctionCall(std::static_pointer_cast<LILFunctionCall>(arg));
                            if (type) {
                                returnTypes.push_back(type);
                            }
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

std::shared_ptr<LILType> LILTypeGuesser::findReturnTypeForFunctionCall(std::shared_ptr<LILFunctionCall> fc) const
{
    switch (fc->getFunctionCallType()) {
        case FunctionCallTypeNone:
        {
            auto localNode = this->findNodeForName(fc->getName(), fc->getParentNode().get());
            if (localNode && localNode->isA(NodeTypeVarDecl)) {
                auto ty = localNode->getType();
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
            
        case FunctionCallTypePointerTo:
        {
            auto firstArg = fc->getArguments().front();
            auto firstArgType = this->getNodeType(firstArg);
            auto newPtrTy = std::make_shared<LILPointerType>();
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
    while (parent) {
        if(parent->isVarNode()){
            std::shared_ptr<LILVarNode> vn = std::static_pointer_cast<LILVarNode>(parent);
            std::shared_ptr<LILNode> localVar = vn->getVariable(nameStr);
            if (localVar) {
                return this->getNodeType(localVar);
            }
        }
        parent = parent->getParentNode();
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::findTypeForValuePath(std::shared_ptr<LILValuePath> vp) const
{
    auto nodes = vp->getNodes();
    std::shared_ptr<LILNode> firstNode;
    if (nodes.size() == 1) {
        firstNode = nodes.front();
        if (firstNode->isA(NodeTypeVarName)) {
            return this->findTypeForVarName(std::static_pointer_cast<LILVarName>(firstNode));
        }
    } else if (nodes.size() > 1){
        firstNode = nodes.front();
        std::shared_ptr<LILClassDecl> classDecl;

        if (firstNode->isA(NodeTypeVarName)) {
            std::shared_ptr<LILType> subjTy = this->findTypeForVarName(std::static_pointer_cast<LILVarName>(firstNode));
            if (subjTy && subjTy->isA(TypeTypeObject)) {
                auto objTy = std::static_pointer_cast<LILObjectType>(subjTy);
                classDecl = this->findClassWithName(objTy->getName().data());
            }
        }
        else if (firstNode->isA(SelectorTypeSelfSelector)) {
            classDecl = this->findAncestorClass(firstNode);
        }
        if (!classDecl) {
            return nullptr;
        }
        for (size_t i=1, j=nodes.size(); i<j; ++i) {
            auto node = nodes[i];
            switch (node->getNodeType()) {
                case NodeTypePropertyName:
                {
                    auto pn = std::static_pointer_cast<LILPropertyName>(node);
                    auto pnName = pn->getName();
                    auto field = classDecl->getFieldNamed(pnName);
                    auto fieldTy = this->getNodeType(field);
                    if (i==j-1) {
                        return fieldTy;
                    } else {
                        if (fieldTy && fieldTy->isA(TypeTypeObject)) {
                            auto fieldObjTy = std::static_pointer_cast<LILObjectType>(fieldTy);
                            classDecl = this->findClassWithName(fieldObjTy->getName().data());
                        }
                    }
                    
                    break;
                }
                case NodeTypeFunctionCall:
                {
                    auto fc = std::static_pointer_cast<LILFunctionCall>(node);
                    auto method = classDecl->getMethodNamed(fc->getName());
                    auto methTy = method->getType();
                    if (!methTy->isA(TypeTypeFunction)) {
                        std::cerr << "METHOD TYPE IS NOT FUNCTION TYPE FAIL!!!!";
                        return nullptr;
                    }
                    auto fnTy = std::static_pointer_cast<LILFunctionType>(methTy);
                    return fnTy->getReturnType();
                }

                default:
                    std::cerr << "!!!!!!!!!!VALUE PATH NODE TYPE FAIL!!!!!!!!!!!!!!!!\n";
                    break;
            }
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
