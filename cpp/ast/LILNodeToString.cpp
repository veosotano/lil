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
 *      This file knows how to convert a node from the AST to a string
 *
 ********************************************************************/

#include "LILNodeToString.h"
#include "LILNode.h"

#include "LILAliasDecl.h"
#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCombinator.h"
#include "LILConversionDecl.h"
#include "LILEnum.h"
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILForeignLang.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILIndexAccessor.h"
#include "LILInstruction.h"
#include "LILMultipleType.h"
#include "LILNullLiteral.h"
#include "LILNumberLiteral.h"
#include "LILObjectDefinition.h"
#include "LILObjectType.h"
#include "LILPercentageLiteral.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILRule.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILSIMDType.h"
#include "LILSimpleSelector.h"
#include "LILStaticArrayType.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILType.h"
#include "LILTypeDecl.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"
#include "LILVarName.h"


using namespace LIL;

LILString LILNodeToString::stringify(LILNode * node)
{
    switch (node->getNodeType()) {
        case NodeTypeBoolLiteral:
        {
            auto value = static_cast<LILBoolLiteral *>(node);
            if (value->getValue()) {
                return "true";
            } else {
                return "false";
            }
        }
        case NodeTypeNumberLiteral:
        {
            auto value = static_cast<LILNumberLiteral *>(node);
            return value->getValue();
        }
        case NodeTypePercentage:
        {
            auto value = static_cast<LILPercentageLiteral *>(node);
            return value->getValue() + "%";
        }
        case NodeTypeExpression:
        {
            auto value = static_cast<LILExpression *>(node);
            LILString tempstr("");
            auto left = value->getLeft();
            auto right = value->getRight();
            if (left)
            {
                tempstr += LILNodeToString::stringify(left.get());
            }
            switch (value->getExpressionType())
            {
                case ExpressionTypeSum:
                {
                    tempstr += " + ";
                    break;
                }
                case ExpressionTypeSubtraction:
                {
                    tempstr += " - ";
                    break;
                }
                case ExpressionTypeMultiplication:
                {
                    tempstr += " * ";
                    break;
                }
                case ExpressionTypeDivision:
                {
                    tempstr += " / ";
                    break;
                }
                case ExpressionTypeEqualComparison:
                {
                    tempstr += " = ";
                    break;
                }
                case ExpressionTypeNotEqualComparison:
                {
                    tempstr += " != ";
                    break;
                }
                case ExpressionTypeBiggerComparison:
                {
                    tempstr += " > ";
                    break;
                }
                case ExpressionTypeBiggerOrEqualComparison:
                {
                    tempstr += " >= ";
                    break;
                }
                case ExpressionTypeSmallerComparison:
                {
                    tempstr += " < ";
                    break;
                }
                case ExpressionTypeSmallerOrEqualComparison:
                {
                    tempstr += " <= ";
                    break;
                }
                case ExpressionTypeLogicalAnd:
                {
                    tempstr += " && ";
                    break;
                }
                case ExpressionTypeLogicalOr:
                {
                    tempstr += " || ";
                    break;
                }
                case ExpressionTypeBitwiseAnd:
                {
                    tempstr += " & ";
                    break;
                }
                case ExpressionTypeBitwiseOr:
                {
                    tempstr += " | ";
                    break;
                }
                case ExpressionTypeCast:
                {
                    tempstr += " => ";
                    break;
                }
                default:
                    break;
            }
            if (right)
            {
                tempstr += LILNodeToString::stringify(right.get());
            }
            return tempstr;
        }
        case NodeTypeUnaryExpression:
        {
            auto value = static_cast<LILUnaryExpression *>(node);
            LILString tempstr("");
            switch (value->getUnaryExpressionType())
            {
                case UnaryExpressionTypeSum:
                {
                    tempstr += " +: ";
                    break;
                }
                case UnaryExpressionTypeSubtraction:
                {
                    tempstr += " -: ";
                    break;
                }
                case UnaryExpressionTypeMultiplication:
                {
                    tempstr += " *: ";
                    break;
                }
                case UnaryExpressionTypeDivision:
                {
                    tempstr += " /: ";
                    break;
                }
                default:
                    break;
            }
            auto val = value->getValue();
            if (val)
            {
                tempstr += LILNodeToString::stringify(val.get());
            }
            return tempstr;
        }
        case NodeTypeStringLiteral:
        {
            auto value = static_cast<LILStringLiteral *>(node);
            return value->getValue();
        }
        case NodeTypeStringFunction:
        {
            auto value = static_cast<LILStringFunction *>(node);
            return value->getStartChunk() + "...";
        }
        case NodeTypeNull:
        {
            return "null";
        }
        case NodeTypeType:
        {
            auto value = static_cast<LILType *>(node);
            switch (value->getTypeType()) {
                case TypeTypeFunction:
                {
                    auto fnTy = static_cast<LILFunctionType *>(node);
                    LILString name = fnTy->getName();
                    
                    name += "(";
                    auto args = fnTy->getArguments();
                    for (size_t i=0, j=args.size(); i<j; ++i) {
                        std::shared_ptr<LILNode> arg = args[i];
                        if (arg) {
                            std::shared_ptr<LILType> ty;
                            if (arg->isA(NodeTypeType)) {
                                ty = std::static_pointer_cast<LILType>(arg);
                            } else if (arg->isA(NodeTypeVarDecl)){
                                ty = std::static_pointer_cast<LILVarDecl>(arg)->getType();
                            }
                            if (ty) {
                                name += LILNodeToString::stringify(ty.get());

                                if ((i+1)<j) {
                                    name += ",";
                                }
                            }
                        }
                    }
                    if (fnTy->getIsVariadic()) {
                        name += "...";
                    }
                    name += ")";
                    std::shared_ptr<LILFunctionType> retTy = std::static_pointer_cast<LILFunctionType>(fnTy->getReturnType());
                    if (retTy) {
                        name += "=>";
                        name += LILNodeToString::stringify(retTy.get());
                    }
                    
                    return name;
                }
                case TypeTypeStaticArray:
                {
                    auto sa = static_cast<LILStaticArrayType *>(node);
                    auto ty = sa->getType();
                    LILString name;
                    if (ty) {
                        name = LILNodeToString::stringify(ty.get());
                    }
                    name += "[";
                    auto arg = sa->getArgument();
                    if (arg) {
                        name += LILNodeToString::stringify(arg.get());
                    }
                    name += "]";
                    if (sa->getIsNullable()) {
                        name += "|null";
                    }
                    return name;
                }
                case TypeTypePointer:
                {
                    auto ptrTy = static_cast<LILPointerType *>(node);
                    LILString name = ptrTy->getName();
                    
                    auto arg = ptrTy->getArgument();
                    if (arg) {
                        name += "(" + LILNodeToString::stringify(arg.get()) + ")";
                    }
                    if (value->getIsNullable()) {
                        name += "|null";
                    }
                    return name;
                }
                    
                case TypeTypeObject:
                {
                    LILString ret = "@" + value->getName();
                    if (value->isA(TypeTypeObject)) {
                        ret = "@" + value->getName();
                    } else {
                        ret = value->getName();
                    }
                    auto objTy = static_cast<LILObjectType *>(value);
                    auto paramTypes = objTy->getTmplParams();
                    if (paramTypes.size() > 0) {
                        ret += "(";
                        for (size_t i = 0, j = paramTypes.size(); i<j; i+=1) {
                            auto paramTy = paramTypes.at(i);
                            ret += LILNodeToString::stringify(paramTy.get());
                            if (i<j-1) {
                                ret += ",";
                            }
                        }
                        ret += ")";
                    }
                    if (value->getIsNullable()) {
                        ret += "|null";
                    }
                    return ret;
                }
                case TypeTypeSingle:
                {
                    LILString ret = value->getName();
                    if (value->getIsNullable()) {
                        ret += "|null";
                    }
                    return ret;
                }
                case TypeTypeMultiple:
                {
                    auto mt = static_cast<LILMultipleType *>(node);
                    LILString tempstr;
                    const auto & types = mt->getTypes();
                    for (size_t i=0, j=types.size(); i<j; ++i) {
                        tempstr += LILNodeToString::stringify(types[i].get());
                        if (i<j-1) {
                            tempstr += "|";
                        }
                    }
                    if (mt->getIsWeakType()) {
                        tempstr = "<"+tempstr+">";
                    }
                    
                    if (mt->getIsNullable()) {
                        return tempstr + "|null";
                    } else {
                        return tempstr;
                    }
                }
                case TypeTypeSIMD:
                {
                    auto simdTy = static_cast<LILSIMDType *>(node);
                    LILString ret = simdTy->getType()->getName();
                    ret += "x";
                    ret += LILString::number((LILUnitI32)simdTy->getWidth());
                    if (value->getIsNullable()) {
                        ret += "|null";
                    }
                    return ret;
                }
                case TypeTypeNone:
                    break;
            }
            
        }
        case NodeTypeVarDecl:
        {
            auto value = static_cast<LILVarDecl *>(node);
            LILString kw;
            if (value->getIsIVar()) {
                kw = "ivar";
            } else if (value->getIsVVar()){
                kw = "vvar";
            } else if (value->getIsConst()){
                kw = "const";
            } else {
                kw = "var";
            }
            
            if (value->getInitVal())
            {
                auto initVal = value->getInitVal();
                return kw + " " + value->getName() + " = " + LILNodeToString::stringify(initVal.get());
            }
            return kw + " " + value->getName();
        }
        case NodeTypeAliasDecl:
        {
            auto value = static_cast<LILAliasDecl *>(node);
            auto ty = value->getSrcType();
            return ty->getName();
        }
        case NodeTypeTypeDecl:
        {
            auto value = static_cast<LILTypeDecl *>(node);
            auto ty = value->getSrcType();
            return ty->getName();
        }
        case NodeTypeConversionDecl:
        {
            auto value = static_cast<LILConversionDecl *>(node);
            return value->_srcTyName + " => " + value->_destTyName;
        }
        case NodeTypeEnum:
        {
            auto value = static_cast<LILEnum *>(node);
            LILString tempstr;
            auto children = value->getChildNodes();
            for (size_t i = 0, j = children.size(); i<j; i+=1) {
                tempstr += LILNodeToString::stringify(children[i].get());
                if (i<j-1) {
                    tempstr += ", ";
                }
            }
            return tempstr;
        }
        case NodeTypeClassDecl:
        {
            auto value = static_cast<LILClassDecl *>(node);
            auto ty = value->getType();
            if (ty) {
                return ty->getName();
            }
            break;
        }
        case NodeTypeObjectDefinition:
        {
            auto value = static_cast<LILObjectDefinition *>(node);
            auto ty = value->getType();
            if (ty) {
                return "@" + ty->getName();
            }
            break;
        }
        case NodeTypeAssignment:
        {
            auto value = static_cast<LILAssignment *>(node);
            auto subject = value->getSubject();
            auto asgmtValue = value->getValue();
            return LILNodeToString::stringify(subject.get()) + ": " + LILNodeToString::stringify(asgmtValue.get());
        }
        case NodeTypeValuePath:
        {
            auto value = static_cast<LILValuePath *>(node);
            LILString ret;
            auto nodes = value->getNodes();
            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                auto node = *it;
                ret += LILNodeToString::stringify(node.get());
                if (it+1 != nodes.end()) {
                    ret += ".";
                }
            }
            return ret;
        }
        case NodeTypePropertyName:
        {
            auto value = static_cast<LILPropertyName *>(node);
            return value->getName();
        }
        case NodeTypeVarName:
        {
            auto value = static_cast<LILVarName *>(node);
            return value->getName();
        }
        case NodeTypeRule:
        {
            break;
        }
        case NodeTypeSimpleSelector:
        {
            break;
        }
        case NodeTypeSelectorChain:
        {
            auto value = static_cast<LILSelectorChain *>(node);
            LILString temp;
            for (auto node : value->getNodes()) {
                temp += LILNodeToString::stringify(node.get());
            }
            return temp;
        }
        case NodeTypeSelector:
        {
            auto value = static_cast<LILSelector *>(node);
            return value->getName();
        }
        case NodeTypeCombinator:
        {
            auto value = static_cast<LILCombinator *>(node);
            switch (value->getCombinatorType())
            {
                case CombinatorTypeDescendants:
                {
                    return "descendants (..)";
                    break;
                }
                case CombinatorTypeSiblings:
                {
                    return "siblings (=)";
                    break;
                }
                case CombinatorTypeNextSiblings:
                {
                    return "next siblings (+)";
                    break;
                }
                case CombinatorTypePreviousSiblings:
                {
                    return "previous siblings (-)";
                    break;
                }
                case CombinatorTypeChildren:
                {
                    return "children";
                    break;
                }
                    
                default:
                    break;
            }
            break;
        }
        case NodeTypeFilter:
        {
            auto value = static_cast<LILFilter *>(node);
            return value->getName();
        }
        case NodeTypeFlag:
        {
            auto value = static_cast<LILFlag *>(node);
            return value->getName();
        }
        case NodeTypeFunctionDecl:
        {
            auto value = static_cast<LILFunctionDecl *>(node);
            return "fn " + value->getName();
        }
        case NodeTypeFunctionCall:
        {
            auto value = static_cast<LILFunctionCall *>(node);
            switch (value->getFunctionCallType())
            {
                case FunctionCallTypeRef:
                {
                    return "ref";
                }
                case FunctionCallTypeSel:
                {
                    return "sel";
                }
                case FunctionCallTypeFlag:
                {
                    return "flag";
                }
                case FunctionCallTypeUnflag:
                {
                    return "unflag";
                }
                case FunctionCallTypeAddFlag:
                {
                    return "addFlag";
                }
                case FunctionCallTypeTakeFlag:
                {
                    return "takeFlag";
                }
                case FunctionCallTypeReplaceFlag:
                {
                    return "replaceFlag";
                }
                case FunctionCallTypePointerTo:
                {
                    return "pointerTo";
                }
                case FunctionCallTypeValueOf:
                {
                    return "valueOf";
                }
                case FunctionCallTypeSet:
                {
                    return "set";
                }
                case FunctionCallTypeSizeOf:
                {
                    return "sizeOf";
                }
                default:
                    break;
            }
            return value->getName();
        }
        case NodeTypeFlowControl:
        {
            auto value = static_cast<LILFlowControl *>(node);
            switch (value->getFlowControlType())
            {
                case FlowControlTypeNone: return "";
                case FlowControlTypeIf: return "if";
                case FlowControlTypeIfCast: return "if cast";
                case FlowControlTypeElse: return "else";
                case FlowControlTypeSwitch: return "switch";
                case FlowControlTypeCase: return "case";
                case FlowControlTypeDefault: return "default";
                case FlowControlTypeLoop: return "loop";
                case FlowControlTypeFor: return "for";
                case FlowControlTypeFinally: return "finally";
            }
            break;
        }
        case NodeTypeFlowControlCall:
        {
            auto value = static_cast<LILFlowControlCall *>(node);
            switch (value->getFlowControlCallType())
            {
                case FlowControlCallTypeReturn:
                {
                    return "return";
                }
                case FlowControlCallTypeRepeat:
                {
                    return "repeat";
                }
                case FlowControlCallTypeContinue:
                {
                    return "continue";
                }
                case FlowControlCallTypeBreak:
                {
                    return "break";
                }
                    
                default:
                    break;
            }
            break;
        }
        case NodeTypeInstruction:
        {
            auto value = static_cast<LILInstruction *>(node);
            auto arg = value->getArgument().get();
            if (arg) {
                return LILInstruction::instructionTypeToString(value->getInstructionType()) + " " + LILNodeToString::stringify(arg);
            }
            return LILInstruction::instructionTypeToString(value->getInstructionType());
        }
        case NodeTypeForeignLang:
        {
            auto value = static_cast<LILForeignLang *>(node);
            return "<" + value->getLanguage() + ">";
        }
        case NodeTypeValueList:
        {
            auto value = static_cast<LILValueList *>(node);
            LILString tempstr;
            auto children = value->getChildNodes();
            for (size_t i = 0, j = children.size(); i<j; i+=1) {
                tempstr += LILNodeToString::stringify(children[i].get());
                if (i<j-1) {
                    tempstr += ", ";
                }
            }
            return tempstr;
        }
        case NodeTypeIndexAccessor:
        {
            return "[]";
        }
        default:
            break;
    }
    return "Error";
}
