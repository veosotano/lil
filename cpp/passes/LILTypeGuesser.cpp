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
: _debug(false)
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
        this->processChildren(node->getChildNodes());
    }
    if (this->_debug) {
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
    if (ty1 && ty1->getIsWeakType()) {
        std::shared_ptr<LILType>ty2 = this->recursiveFindTypeFromAncestors(value->shared_from_this());
        auto ty3 = LILType::merge(ty1, ty2);
        if (ty3) {
            if (ty3->getIsWeakType()) {
                auto multiTy = std::static_pointer_cast<LILMultipleType>(ty3);
                value->setType(multiTy->getTypes().front());
            } else {
                value->setType(ty3);
            }
        }
    }
}

void LILTypeGuesser::_process(LILPercentageLiteral * value)
{

}

void LILTypeGuesser::_process(LILExpression * value)
{
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
        } else {
            //this->searchForType(value);
        }
    }
}

void LILTypeGuesser::_process(LILClassDecl * value)
{

}

void LILTypeGuesser::_process(LILObjectDefinition * value)
{
    auto ty = value->getType();
    if (!ty->isA(TypeTypeObject)) {
        return;
    }
    auto objTy = std::static_pointer_cast<LILObjectType>(ty);
    //populate the fields of the object type from the class
    if (objTy->getFields().size() == 0) {
        for (auto classVal : this->getRootNode()->getClasses()) {
            if (classVal->getType()->getName() == objTy->getName()) {
                for (auto method : classVal->getMethods()) {
                    auto methTy = method->getType();
                    if (!methTy->isA(TypeTypeFunction)) {
                        objTy->addField(methTy->clone());
                    }
                }
                break;
            }
        }
    }
}

void LILTypeGuesser::_process(LILAssignment * value)
{

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
    if (!value->getReturnType()) {
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
}

void LILTypeGuesser::_process(LILFlowControl * value)
{

}

void LILTypeGuesser::_process(LILFlowControlCall * value)
{
    std::vector<std::shared_ptr<LILType>> newTypes;
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
    value->setTypes(std::move(newTypes));
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
                if (fc->getFunctionCallType() == FunctionCallTypeNone) {
                    //get value path
                    auto grandpa = fc->getParentNode();
                    if (grandpa && grandpa->isA(NodeTypeValuePath)) {
                        auto vp = std::static_pointer_cast<LILValuePath>(grandpa);
                        //first node is the var name
                        auto firstNode = vp->getNodes().front();
                        if (firstNode->isA(NodeTypeVarName)) {
                            auto varName = std::static_pointer_cast<LILVarName>(firstNode);

                            //find the var containing the function
                            auto remoteNode = this->findNodeForVarName(varName.get());
                            std::shared_ptr<LILNode> fnNode;
                            if (remoteNode->isA(NodeTypeVarDecl)) {
                                fnNode = std::static_pointer_cast<LILVarDecl>(remoteNode)->getInitVal();
                            } else {
                                std::cerr << "LOCAL VAR IS NOT VAR DECL FAIL!!!!";
                                return nullptr;
                            }

                            if (fnNode && fnNode->isA(NodeTypeFunctionDecl)) {
                                auto fd = std::static_pointer_cast<LILFunctionDecl>(fnNode);

                                //find the index of the type
                                size_t typeIndex = 0;
                                auto args = fd->getArguments();
                                for (size_t i=0,j=args.size(); i<j; ++i) {
                                    auto argVd = std::static_pointer_cast<LILVarDecl>(args[i]);
                                    if (argVd->getName() == varName->getName()) {
                                        typeIndex = i;
                                        break;
                                    }
                                }
                                auto ty = fd->getType();
                                if (ty && ty->getTypeType() == TypeTypeFunction) {
                                    auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                                    if (fnTy->getArguments().size() < typeIndex + 1) {
                                        std::cerr << "FAIL!!!!";
                                        return nullptr;
                                    }

                                    //ta daaaaa
                                    return fnTy->getArguments()[typeIndex];

                                } else {
                                    std::cerr << "FAIL!!!!";
                                    return nullptr;
                                }

                            } else {
                                std::cerr << "FAIL!!!!";
                                return nullptr;
                            }
                        } else {
                            std::cerr << "FAIL!!!!";
                            return nullptr;
                        }

                    } else {
                        std::cerr << "FAIL!!!!";
                        return nullptr;
                    }
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
                            std::cerr << "FAIL!!!!";
                            return nullptr;
                        }
                        
                    } else {
                        std::cerr << "FAIL!!!!";
                        return nullptr;
                    }

                    auto objdef = std::static_pointer_cast<LILObjectDefinition>(grandpa);
                    auto objTy = objdef->getType();
                    auto className = objTy->getName();
                    auto classValue = this->findClassWithName(className);
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
                            std::cerr << "FAIL!!!!";
                            return nullptr;
                        }
                    }
                } else {
                    std::cerr << "FAIL!!!!";
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
            const auto & childNodes = vp->getNodes();
            if (childNodes.size() == 1) {
                std::shared_ptr<LILNode> firstNode = childNodes.front();
                if (!firstNode->isA(NodeTypeVarName)) {
                    return nullptr;
                }
                return this->findTypeForVarName(std::static_pointer_cast<LILVarName>(firstNode));
            } else {
                auto firstNode = childNodes.front();
                if (firstNode->isA(NodeTypeVarName)) {
                    auto vn = std::static_pointer_cast<LILVarName>(firstNode);
                    auto ty = this->findTypeForVarName(vn);
                    if (ty) {
                        if (ty->isA(TypeTypeFunction)) {
                            if (ty->isA(TypeTypeFunction)) {
                                auto returnTy = std::static_pointer_cast<LILFunctionType>(ty)->getReturnType();
                                if (returnTy) {
                                    return returnTy;
                                }
                            }
                        }
                        return ty;
                    }
                }
            }
            break;
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
            std::shared_ptr<LILType> ty = fc->getType();
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
            if (leftType->getIsWeakType()) {
                return this->recursiveFindTypeFromAncestors(exp);
            } else {
                return leftType;
            }
        }
        std::shared_ptr<LILType> leftTy = std::static_pointer_cast<LILType>(leftType);
        std::shared_ptr<LILType> rightTy = std::static_pointer_cast<LILType>(rightType);
        auto mergedTy = LILType::merge(leftTy, rightTy);
        if (mergedTy->getIsWeakType()) {
            return this->recursiveFindTypeFromAncestors(exp);
        }
    }
    else if (!leftType && rightType) {
        return rightType;
    }
    if (leftType && !rightType) {
        return leftType;
    }
    return nullptr;
}

std::shared_ptr<LILType> LILTypeGuesser::getFnType(std::shared_ptr<LILFunctionDecl> fd) const
{
    std::vector<std::shared_ptr<LILType>> returnTypes;
    std::shared_ptr<LILFunctionType> ret = std::make_shared<LILFunctionType>();
    ret->setName("fn");
    for (const auto & arg : fd->getArguments()) {
        if (!arg->isA(NodeTypeVarDecl)) {
            continue;
        }
        std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(arg);
        auto argType = this->getNodeType(arg);
        if (!argType) {
            auto body = fd->getBody();
            argType = this->findTypeFromAssignments(body, vd);
            if (!argType) {
                argType = this->findTypeFromFunctionCalls(body, vd);
                if (!argType) {
                    argType = this->findTypeFromExpressions(body, vd);
                    if (!argType) {
                        std::shared_ptr<LILType> anyTy = std::make_shared<LILType>();
                        anyTy->setName("any");
                        argType = anyTy;
                    }
                }
            }
        }
        if (argType) {
            ret->addArgument(argType);
            vd->setType(argType);
        }
    }

    auto evals = fd->getBody();
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
                            if (vp->getNodes().size() > 0) {
                                std::shared_ptr<LILNode> firstNode = vp->getNodes().front();
                                if (firstNode->isA(NodeTypeVarName)) {
                                    std::shared_ptr<LILType> type = this->findTypeForVarName(std::static_pointer_cast<LILVarName>(firstNode));
                                    if (type) {
                                        switch (type->getTypeType()) {
                                            case TypeTypeSingle:
                                            {
                                                returnTypes.push_back(type);
                                                break;
                                            }
                                            case TypeTypeFunction:
                                            {
                                                auto fnTy = std::static_pointer_cast<LILFunctionType>(type);
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
                                                if (!type->getIsWeakType()) {
                                                    for ( auto retSubTy : std::static_pointer_cast<LILMultipleType>(type)->getTypes() ) {
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
                                }
                            } else {
                                std::cerr << "!!!!!!!!!!GUESSER FAIL!!!!!!!!!!!!!!!!\n";
                            }
                        }
                        default:
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

std::shared_ptr<LILType> LILTypeGuesser::findTypeFromAssignments(std::vector<std::shared_ptr<LILNode>> nodes, const std::shared_ptr<LILVarDecl> & vd) const
{
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

void LILTypeGuesser::setDebug(bool value)
{
    this->_debug = value;
}
