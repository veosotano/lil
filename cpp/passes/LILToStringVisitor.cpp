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
 *      This file prints nodes as text in a hierarchical tree
 *
 ********************************************************************/

#include "LILToStringVisitor.h"

using namespace LIL;

namespace LIL {
    class LILToStrInfo {
    public:
        LILString value;
        std::vector<LILToStrInfo> children;
    };
}

std::vector<size_t> LILToStringVisitorNoItems;

LILToStringVisitor::LILToStringVisitor()
{
}

LILToStringVisitor::~LILToStringVisitor()
{
}



void LILToStringVisitor::initializeVisit()
{
    if (this->getVerbose()) {
        if (this->getPrintHeadline()) {
            std::cerr << "\n\n";
            std::cerr << "============================\n";
            std::cerr << "=== ABSTRACT SYNTAX TREE ===\n";
            std::cerr << "============================\n\n";
        } else {
            std::cerr << "\n";
        }
    }
}

void LILToStringVisitor::visit(LILNode *node)
{
    LILToStrInfo info = this->stringify(node);
    this->printInfo(info, 0, LILToStringVisitorNoItems);
}

void LILToStringVisitor::printInfo(LILToStrInfo info, size_t indents, std::vector<size_t> moreItems)
{
    std::cerr << this->stringForIndent(indents, moreItems).chardata();
    std::cerr << info.value.chardata();
    std::cerr << "\n";
    for (auto it = info.children.begin(); it!=info.children.end(); ++it){
        std::vector<size_t> moreItemsChildren = moreItems;
        if (it+1 != info.children.end() && std::find(moreItems.begin(), moreItems.end(), indents) == moreItems.end()) {
            moreItemsChildren.push_back(indents);
        }
        this->printInfo(*it, indents+1, moreItemsChildren);
    }
}

LILToStrInfo LILToStringVisitor::stringify(LILNode * node)
{
    LILToStrInfo info;
    switch (node->getNodeType()) {
        case NodeTypeBool:
        {
            LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeNumberLiteral:
        {
            LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypePercentage:
        {
            LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
            info = this->_stringify(value);
            break;
            break;
        }
        case NodeTypeExpression:
        {
            LILExpression * value = static_cast<LILExpression *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeStringLiteral:
        {
            LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeStringFunction:
        {
            LILStringFunction * value = static_cast<LILStringFunction *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeNull:
        {
            LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeType:
        {
            LILType * value = static_cast<LILType *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeVarDecl:
        {
            LILVarDecl * value = static_cast<LILVarDecl *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeClassDecl:
        {
            LILClassDecl * value = static_cast<LILClassDecl *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeObjectDefinition:
        {
            LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeAssignment:
        {
            LILAssignment * value = static_cast<LILAssignment *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeValuePath:
        {
            LILValuePath * value = static_cast<LILValuePath *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypePropertyName:
        {
            LILPropertyName * value = static_cast<LILPropertyName *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeVarName:
        {
            LILVarName * value = static_cast<LILVarName *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeRule:
        {
            LILRule * value = static_cast<LILRule *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeSimpleSelector:
        {
            LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeSelectorChain:
        {
            LILSelectorChain * value = static_cast<LILSelectorChain *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeSelector:
        {
            LILSelector * value = static_cast<LILSelector *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeCombinator:
        {
            LILCombinator * value = static_cast<LILCombinator *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeFilter:
        {
            LILFilter * value = static_cast<LILFilter *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeFlag:
        {
            LILFlag * value = static_cast<LILFlag *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeFunctionCall:
        {
            LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeFlowControl:
        {
            LILFlowControl * value = static_cast<LILFlowControl *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeFlowControlCall:
        {
            LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            info = this->_stringify(value);
            break;
        }

        default:
            std::cerr << "Error: unkonwn node type to stringify\n";
            break;
    }
    return info;
}

LILToStrInfo LILToStringVisitor::_stringify(LILBoolLiteral * value)
{
    LILToStrInfo ret;
    if (value->getValue()) {
        ret.value = "Bool literal: true";
    } else {
        ret.value = "Bool literal: false";
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILNumberLiteral * value)
{
    LILToStrInfo ret;
    LILNode * type = value->getType().get();
    if (type) {
        ret.value = "Number literal (" + type->stringRep() + "): " + value->getValue();
    } else {
        ret.value = "Number literal: " + value->getValue();
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILPercentageLiteral * value)
{
    LILToStrInfo ret;
    ret.value = "Percentage literal: "+value->stringRep();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILExpression * value)
{
    LILToStrInfo ret;
    LILNode * type = value->getType().get();
    LILString typestr;
    if (type) {
        typestr = " (" + type->stringRep() + ")";
    }
    LILString expstr = LILString("Expression"+ typestr +": ") + LILExpression::expressionTypeToString(value->getExpressionType());
    ret.value = expstr;
    this->stringifyChildren(value->getNodes(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILStringLiteral * value)
{
    LILToStrInfo ret;
    LILString isCStr = value->getIsCString() ? "C " : "";
    ret.value = isCStr + "String literal: " + value->getValue();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILStringFunction * value)
{
    LILToStrInfo ret;
    ret.value = "String function:";
    LILToStrInfo startInfo;
    startInfo.value = value->getStartChunk()+"\"";
    ret.children.push_back(startInfo);

    auto children = value->getNodes();
    auto midChunks = value->getMidChunks();
    for (size_t i=0, j=children.size(); i<j; ++i) {
        ret.children.push_back(this->stringify(children[i].get()));

        if (midChunks.size() > i) {
            LILToStrInfo midInfo;
            midInfo.value = LILString("\"")+midChunks[i]+"\"";
            ret.children.push_back(midInfo);
        }
        
    }
    LILToStrInfo endInfo;
    endInfo.value = LILString("\"")+value->getEndChunk();
    ret.children.push_back(endInfo);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILNullLiteral * value)
{
    LILToStrInfo ret;
    ret.value = "Null literal";
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILType * value)
{
    LILToStrInfo ret;
    ret.value = "Type: " + value->stringRep();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILVarDecl * value)
{
    LILToStrInfo ret;
    
    LILNode * type = value->getType().get();
    LILString externStr = value->getIsExtern() ? "extern " : "";

    LILString vdType = "";
    if (value->getIsIVar()) {
        vdType = "(ivar) ";
    } else if (value->getIsVVar()) {
        vdType = "(vvar) ";
    }
    if (type) {
        ret.value = "Var declaration " + vdType + externStr + "(" + type->stringRep() + "): " + value->getName();
    } else {
        ret.value = "Var declaration: " + vdType + externStr + value->getName();
    }
    for (auto node : value->getInitVals()) {
        ret.children.push_back(this->stringify(node.get()));
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILClassDecl * value)
{
    LILToStrInfo ret;
    LILString externStr = value->getIsExtern() ? "extern " : "";
    ret.value = "Class declaration: " + externStr;
    LILToStrInfo typeInfo;
    std::shared_ptr<LILNode> typeNode = value->getType();
    if (typeNode) {
        ret.children.push_back(this->stringify(typeNode.get()));
    }
    std::shared_ptr<LILNode> inheritTypeNode = value->getInheritType();
    if (inheritTypeNode) {
        LILToStrInfo inheritInfo;
        inheritInfo.value = "Inherited type: "+inheritTypeNode->stringRep();
        ret.children.push_back(inheritInfo);
    }
    std::vector<std::shared_ptr<LILNode>> fields = value->getFields();
    if (fields.size() > 0) {
        LILToStrInfo fieldsInfo;
        fieldsInfo.value = "Fields:";
        this->stringifyChildren(fields, fieldsInfo);
        ret.children.push_back(fieldsInfo);
    }
    std::vector<std::shared_ptr<LILNode>> methods = value->getMethods();
    if (methods.size() > 0) {
        LILToStrInfo methodsInfo;
        methodsInfo.value = "Methods:";
        this->stringifyChildren(methods, methodsInfo);
        ret.children.push_back(methodsInfo);
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILObjectDefinition * value)
{
    LILToStrInfo ret;
    ret.value = "Object definition:";
    this->stringifyChildren(value->getNodes(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILAssignment * value)
{
    LILToStrInfo ret;
    auto ty = value->getType();
    if (ty) {
        ret.value = "Assignment (" + ty->stringRep() + "): ";
    } else {
        ret.value = "Assignment: ";
    }
    LILToStrInfo subjInfo;
    subjInfo.value = "Subject:";
    subjInfo.children.push_back(this->stringify(value->getSubject().get()));
    ret.children.push_back(subjInfo);
    auto valueNode = value->getValue();
    if (valueNode) {
        LILToStrInfo valInfo;
        valInfo.value = "Value:";
        valInfo.children.push_back(this->stringify(valueNode.get()));
        ret.children.push_back(valInfo);
    }

    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILValuePath * value)
{
    LILToStrInfo ret;
    ret.value = "ValuePath:";
    this->stringifyChildren(value->getNodes(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILPropertyName * value)
{
    LILToStrInfo ret;
    ret.value = "Property name: "+value->getName();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILRule * value)
{
    LILToStrInfo ret;
    ret.value = "Rule:";
    this->stringifyChildren(value->getNodes(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILSimpleSelector * value)
{
    LILToStrInfo ret;
    ret.value = "Simple selector: "+value->stringRep();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILSelectorChain * value)
{
    LILToStrInfo ret;
    ret.value = "Selector chain:";
    this->stringifyChildren(value->getNodes(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILSelector * value)
{
    LILToStrInfo ret;
    ret.value = "Selector: "+value->getName();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILCombinator * value)
{
    LILToStrInfo ret;
    ret.value = "Combinator: "+value->stringRep();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFilter * value)
{
    LILToStrInfo ret;
    ret.value = "Filter: "+value->stringRep();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFlag * value)
{
    LILToStrInfo ret;
    ret.value = "Flag: "+value->stringRep();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILVarName * value)
{
    LILToStrInfo ret;
    ret.value = "Var name: "+value->stringRep();
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFunctionDecl * value)
{
    LILToStrInfo ret;
    
    auto name = value->getName();
    
    auto ty = std::static_pointer_cast<LILFunctionType>(value->getType());
    auto type = ty.get();
    if (type) {
        ret.value = "Function declaration " + name +" (" + type->stringRep() + "): " + value->stringRep();
    } else {
        ret.value = "Function declaration " + name + ": " + value->stringRep();
    }

    if (ty) {
        LILToStrInfo argsInfo;
        argsInfo.value = "Arguments:";
        const auto & args = ty->getArguments();
        for (auto it = args.begin(); it!= args.end(); ++it)
        {
            auto node = (*it).get();
            argsInfo.children.push_back(this->stringify(node));
        };
        if (argsInfo.children.size() > 0) {
            ret.children.push_back(argsInfo);
        }
    }
    
    LILToStrInfo bodyInfo;
    bodyInfo.value = "Body:";
    for (auto it = value->getBody().begin(); it!=value->getBody().end(); ++it)
    {
        bodyInfo.children.push_back(this->stringify((*it).get()));
    };
    if (bodyInfo.children.size() > 0) {
        ret.children.push_back(bodyInfo);
    }
    auto finally = value->getFinally();
    if (finally) {
        LILToStrInfo finallyInfo;
        finallyInfo.value = "Finally:";
        finallyInfo.children.push_back(this->stringify(finally.get()));
        ret.children.push_back(finallyInfo);
    }

    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFunctionCall * value)
{
    LILToStrInfo ret;
    ret.value = "Function call: "+value->stringRep();
    this->stringifyChildren(value->getArguments(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFlowControl * value)
{
    LILToStrInfo ret;
    ret.value = "Flow control: "+value->stringRep();
    LILToStrInfo argsInfo;
    argsInfo.value = "Arguments:";
    for (auto it = value->getArguments().begin(); it!=value->getArguments().end(); ++it)
    {
        argsInfo.children.push_back(this->stringify((*it).get()));
    };
    if (argsInfo.children.size() > 0) {
        ret.children.push_back(argsInfo);
    }
    
    LILToStrInfo thenInfo;
    thenInfo.value = "Then:";
    for (auto it = value->getThen().begin(); it!=value->getThen().end(); ++it)
    {
        thenInfo.children.push_back(this->stringify((*it).get()));
    };
    if (thenInfo.children.size() > 0) {
        ret.children.push_back(thenInfo);
    }
    
    auto elseNodes = value->getElse();
    if (elseNodes.size() > 0) {
        LILToStrInfo thenInfo;
        thenInfo.value = "Else:";
        for (auto it = value->getElse().begin(); it!=value->getElse().end(); ++it)
        {
            thenInfo.children.push_back(this->stringify((*it).get()));
        };
        if (thenInfo.children.size() > 0) {
            ret.children.push_back(thenInfo);
        }
    }
    
    return ret;
}



LILToStrInfo LILToStringVisitor::_stringify(LILFlowControlCall * value)
{
    LILToStrInfo ret;
    ret.value = "Flow control call: "+value->stringRep();
    if (value->isA(FlowControlCallTypeReturn)) {
        ret.children.push_back(this->stringify(value->getArgument().get()));
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILInstruction * value)
{
    LILToStrInfo ret;
    if (value->getIsColorInstruction()) {
        ret.value = "Color: #"+value->stringRep();
    } else {
        ret.value = "Instruction: #"+value->stringRep();
    }
    
    return ret;
}

LILString LILToStringVisitor::stringForIndent(size_t indents, std::vector<size_t> moreItems)
{
    LILString tempstr("");
    for (size_t i = 0; i<indents; ++i)
    {
        if (i<indents-1)
        {
            if (std::find(moreItems.begin(), moreItems.end(), i) != moreItems.end()) {
                tempstr += " │   ";
            } else {
                tempstr += "     ";
            }
        } else if (i==indents-1) {
            if (std::find(moreItems.begin(), moreItems.end(), i) != moreItems.end()) {
                tempstr += " ├── ";
            } else {
                tempstr += " └── ";
            }
        }
    }
    return tempstr;
}

void LILToStringVisitor::stringifyChildren(const std::vector<std::shared_ptr<LILNode> > &children, LILToStrInfo &info)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        info.children.push_back(this->stringify((*it).get()));
    };
}
