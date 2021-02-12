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
#include "LILNodeToString.h"
#include "LILIfInstruction.h"
#include "LILSnippetInstruction.h"

using namespace LIL;

namespace LIL {
    class LILToStrInfo {
    public:
        LILString value;
        std::vector<LILToStrInfo> children;
        bool isExported;
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
    if (!node->hidden) {
        LILToStrInfo info = this->stringify(node);
        this->printInfo(info, 0, LILToStringVisitorNoItems);
    }
}

void LILToStringVisitor::printInfo(LILToStrInfo info, size_t indents, std::vector<size_t> moreItems)
{
    std::cerr << this->stringForIndent(indents, moreItems).chardata();
    if(info.isExported) std::cerr << "[exp] ";
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
        case NodeTypeBoolLiteral:
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
        case NodeTypeUnaryExpression:
        {
            LILUnaryExpression * value = static_cast<LILUnaryExpression *>(node);
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
        case NodeTypeAliasDecl:
        {
            LILAliasDecl * value = static_cast<LILAliasDecl *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeTypeDecl:
        {
            LILTypeDecl * value = static_cast<LILTypeDecl *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeConversionDecl:
        {
            LILConversionDecl * value = static_cast<LILConversionDecl *>(node);
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
        case NodeTypeIfInstruction:
        case NodeTypeSnippetInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeForeignLang:
        {
            LILForeignLang * value = static_cast<LILForeignLang *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeDocumentation:
        {
            LILDocumentation * value = static_cast<LILDocumentation *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeValueList:
        {
            LILValueList * value = static_cast<LILValueList *>(node);
            info = this->_stringify(value);
            break;
        }
        case NodeTypeIndexAccessor:
        {
            LILIndexAccessor * value = static_cast<LILIndexAccessor *>(node);
            info = this->_stringify(value);
            break;
        }

        default:
            std::cerr << "Error: unkonwn node type to stringify\n";
            break;
    }
    info.isExported = node->getIsExported();
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
        ret.value = "Number literal (" + LILNodeToString::stringify(type) + "): " + value->getValue();
    } else {
        ret.value = "Number literal: " + value->getValue();
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILPercentageLiteral * value)
{
    LILToStrInfo ret;
    LILNode * type = value->getType().get();
    if (type) {
        ret.value = "Percentage literal (" + LILNodeToString::stringify(type) + "): " + LILNodeToString::stringify(value);
    } else {
        ret.value = "Percentage literal: " + LILNodeToString::stringify(value);
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILExpression * value)
{
    LILToStrInfo ret;
    LILNode * type = value->getType().get();
    LILString typestr;
    if (type) {
        typestr = " (" + LILNodeToString::stringify(type) + ")";
    }
    LILString expstr = LILString("Expression"+ typestr +": ") + LILExpression::expressionTypeToString(value->getExpressionType());
    ret.value = expstr;
    this->stringifyChildren(value->getNodes(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILUnaryExpression * value)
{
    LILToStrInfo ret;
    LILNode * type = value->getType().get();
    LILString typestr;
    if (type) {
        typestr = " (" + LILNodeToString::stringify(type) + ")";
    }
    LILString expstr = LILString("Unary expression"+ typestr +": ") + LILUnaryExpression::expressionTypeToString(value->getUnaryExpressionType());
    ret.value = expstr;
    ret.children.push_back(this->stringify(value->getValue().get()));
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
    startInfo.isExported = false;
    startInfo.value = value->getStartChunk()+"\"";
    ret.children.push_back(startInfo);

    auto children = value->getNodes();
    auto midChunks = value->getMidChunks();
    for (size_t i=0, j=children.size(); i<j; ++i) {
        ret.children.push_back(this->stringify(children[i].get()));

        if (midChunks.size() > i) {
            LILToStrInfo midInfo;
            midInfo.isExported = false;
            midInfo.value = LILString("\"")+midChunks[i]+"\"";
            ret.children.push_back(midInfo);
        }
        
    }
    LILToStrInfo endInfo;
    endInfo.isExported = false;
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
    ret.value = "Type: " + LILNodeToString::stringify(value);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILVarDecl * value)
{
    LILToStrInfo ret;
    ret.isExported = value->getIsExported();

    LILNode * type = value->getType().get();
    LILString externStr = value->getIsExtern() ? " extern" : "";

    LILString vdType = "";
    LILString firstWord;
    if (value->getIsIVar()) {
        vdType = "(ivar) ";
    } else if (value->getIsVVar()) {
        vdType = "(vvar) ";
    }
    if (value->getIsConst()) {
        firstWord = "Const";
    } else {
        firstWord = "Var";
    }
    if (type) {
        ret.value = firstWord + " declaration " + vdType + "(" + LILNodeToString::stringify(type) + "): " + value->getName() + externStr;
    } else {
        ret.value = firstWord + " declaration: " + vdType + value->getName() + externStr;
    }
    auto initVal = value->getInitVal();
    if (initVal) {
        ret.children.push_back(this->stringify(initVal.get()));
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILAliasDecl *value)
{
    LILToStrInfo ret;

    auto srcTy = value->getSrcType();
    auto dstTy = value->getDstType();
    
    if (dstTy) {
        ret.value = "Alias declaration: "+LILNodeToString::stringify(srcTy.get())+" => "+ LILNodeToString::stringify(dstTy.get());
    } else {
        ret.value = "Alias declaration: "+LILNodeToString::stringify(srcTy.get());
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILTypeDecl *value)
{
    LILToStrInfo ret;
    
    auto source = value->getSrcType();
    auto target = value->getDstType();
    
    if (target) {
        ret.value = "Type declaration: "+LILNodeToString::stringify(source.get())+" => "+LILNodeToString::stringify(target.get());
    } else {
        ret.value = "Type declaration: "+LILNodeToString::stringify(source.get());
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILConversionDecl *value)
{
    LILToStrInfo ret;

    auto varDecl = value->getVarDecl();
    if (!varDecl) {
        return ret;
    }
    ret.value = "Conversion declaration " + LILNodeToString::stringify(value);

    LILToStrInfo argumentsInfo;
    argumentsInfo.isExported = false;
    argumentsInfo.value = "Arguments:";
    argumentsInfo.children.push_back(this->_stringify(varDecl.get()));
    ret.children.push_back(argumentsInfo);

    LILToStrInfo bodyInfo;
    bodyInfo.value = "Body:";
    auto body = value->getBody();
    for (auto it = body.begin(); it!=body.end(); ++it)
    {
        bodyInfo.children.push_back(this->stringify((*it).get()));
    };
    ret.children.push_back(bodyInfo);

    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILClassDecl * value)
{
    LILToStrInfo ret;
    LILString externStr = value->getIsExtern() ? "extern " : "";
    bool isTmpl = value->isTemplate();
    LILString templateStr = "";
    if (isTmpl) {
        ret.value = "Class template declaration: " + externStr;
        ret.children.push_back(this->stringify(value->getType().get()));
    } else {
        ret.value = "Class declaration: " + externStr;
    }

    std::shared_ptr<LILNode> inheritTypeNode = value->getInheritType();
    if (inheritTypeNode) {
        LILToStrInfo inheritInfo;
        inheritInfo.isExported = false;
        inheritInfo.value = "Inherited type: "+LILNodeToString::stringify(inheritTypeNode.get());
        ret.children.push_back(inheritInfo);
    }
    std::vector<std::shared_ptr<LILAliasDecl>> aliases = value->getAliases();
    if (aliases.size() > 0) {
        for (auto it = aliases.begin(); it!=aliases.end(); ++it)
        {
            ret.children.push_back(this->stringify((*it).get()));
        };
    }
    std::vector<std::shared_ptr<LILNode>> fields = value->getFields();
    if (fields.size() > 0) {
        LILToStrInfo fieldsInfo;
        fieldsInfo.isExported = false;
        fieldsInfo.value = "Fields:";
        this->stringifyChildren(fields, fieldsInfo);
        ret.children.push_back(fieldsInfo);
    }
    std::vector<std::shared_ptr<LILNode>> methods = value->getMethods();
    if (methods.size() > 0) {
        LILToStrInfo methodsInfo;
        methodsInfo.isExported = false;
        methodsInfo.value = "Methods:";
        this->stringifyChildren(methods, methodsInfo);
        ret.children.push_back(methodsInfo);
    }
    std::vector<std::shared_ptr<LILDocumentation>> docs = value->getDocs();
    if (docs.size() > 0) {
        for (auto it = docs.begin(); it!=docs.end(); ++it)
        {
            ret.children.push_back(this->stringify((*it).get()));
        };
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILObjectDefinition * value)
{
    LILToStrInfo ret;
    auto ty = value->getType();
    if (ty) {
        ret.value = "Object definition (" + LILNodeToString::stringify(ty.get()) + "): ";
    } else {
        ret.value = "Object definition: ";
    }
    this->stringifyChildren(value->getNodes(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILAssignment * value)
{
    LILToStrInfo ret;
    auto ty = value->getType();
    if (ty) {
        ret.value = "Assignment (" + LILNodeToString::stringify(ty.get()) + "): ";
    } else {
        ret.value = "Assignment: ";
    }
    LILToStrInfo subjInfo;
    subjInfo.isExported = false;
    subjInfo.value = "Subject:";
    subjInfo.children.push_back(this->stringify(value->getSubject().get()));
    ret.children.push_back(subjInfo);
    auto valueNode = value->getValue();
    if (valueNode) {
        LILToStrInfo valInfo;
        valInfo.isExported = false;
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
    ret.value = "Simple selector: "+LILNodeToString::stringify(value);
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
    ret.value = "Combinator: "+LILNodeToString::stringify(value);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFilter * value)
{
    LILToStrInfo ret;
    ret.value = "Filter: "+LILNodeToString::stringify(value);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFlag * value)
{
    LILToStrInfo ret;
    ret.value = "Flag: "+LILNodeToString::stringify(value);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILVarName * value)
{
    LILToStrInfo ret;
    ret.value = "Var name: "+LILNodeToString::stringify(value);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFunctionDecl * value)
{
    LILToStrInfo ret;
    
    auto name = value->getName();
    
    auto ty = std::static_pointer_cast<LILFunctionType>(value->getType());
    ret.value = "Function declaration: " + name;

    if (ty) {
        LILToStrInfo argsInfo;
        argsInfo.isExported = false;
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
        const auto retTy = ty->getReturnType();
        if (retTy) {
            LILToStrInfo retInfo;
            retInfo.isExported = false;
            retInfo.value = "return =>";
            retInfo.children.push_back(this->stringify(retTy.get()));
            ret.children.push_back(retInfo);
        }
    }
    
    auto body = value->getBody();
    if (body.size() > 0) {
        LILToStrInfo bodyInfo;
        bodyInfo.isExported = false;
        bodyInfo.value = "Body:";
        for (auto it = body.begin(); it!=body.end(); ++it)
        {
            bodyInfo.children.push_back(this->stringify((*it).get()));
        };
        ret.children.push_back(bodyInfo);
    }
    auto impls = value->getImpls();
    if (impls.size() > 0) {
        LILToStrInfo implsInfo;
        implsInfo.isExported = false;
        implsInfo.value = "Implementations:";
        for (auto it = impls.begin(); it!=impls.end(); ++it)
        {
            implsInfo.children.push_back(this->stringify((*it).get()));
        };
        ret.children.push_back(implsInfo);
    }
    auto finally = value->getFinally();
    if (finally) {
        LILToStrInfo finallyInfo;
        finallyInfo.isExported = false;
        finallyInfo.value = "Finally:";
        finallyInfo.children.push_back(this->stringify(finally.get()));
        ret.children.push_back(finallyInfo);
    }

    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFunctionCall * value)
{
    LILToStrInfo ret;
    LILString typeStr;
    auto fcTypes = value->getArgumentTypes();
    for (size_t i=0, j=fcTypes.size(); i<j; ++i) {
        auto type = fcTypes[i];
        typeStr += LILNodeToString::stringify(type.get());
        if (i<j-1) {
            typeStr += ",";
        }
    }
    if (typeStr.length() > 0) {
        typeStr = " (" + typeStr + ")";
    }
    ret.value = "Function call:"+ typeStr +" "+LILNodeToString::stringify(value);
    this->stringifyChildren(value->getArguments(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILFlowControl * value)
{
    LILToStrInfo ret;
    ret.value = "Flow control: "+LILNodeToString::stringify(value);
    LILToStrInfo argsInfo;
    argsInfo.isExported = false;
    argsInfo.value = "Arguments:";
    for (auto it = value->getArguments().begin(); it!=value->getArguments().end(); ++it)
    {
        argsInfo.children.push_back(this->stringify((*it).get()));
    };
    if (argsInfo.children.size() > 0) {
        ret.children.push_back(argsInfo);
    }
    
    LILToStrInfo thenInfo;
    thenInfo.isExported = false;
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
        thenInfo.isExported = false;
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
    ret.value = "Flow control call: "+LILNodeToString::stringify(value);
    if (value->isA(FlowControlCallTypeReturn)) {
        ret.children.push_back(this->stringify(value->getArgument().get()));
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILInstruction * value)
{
    LILToStrInfo ret;
    if (value->getIsColorInstruction()) {
        ret.value = "Color: #"+LILNodeToString::stringify(value);
        return ret;
    } else {
        switch (value->getInstructionType()) {
            case InstructionTypeIf:
            {
                auto ifInstr = static_cast<LILIfInstruction *>(value);
                ret.value = "Instruction: #if";
                LILToStrInfo argsInfo;
                argsInfo.isExported = false;
                argsInfo.value = "Argument:";
                argsInfo.children.push_back(this->stringify(value->getArgument().get()));
                if (argsInfo.children.size() > 0) {
                    ret.children.push_back(argsInfo);
                }
                
                LILToStrInfo thenInfo;
                thenInfo.isExported = false;
                thenInfo.value = "Then:";
                for (auto it = ifInstr->getThen().begin(); it!=ifInstr->getThen().end(); ++it)
                {
                    thenInfo.children.push_back(this->stringify((*it).get()));
                };
                if (thenInfo.children.size() > 0) {
                    ret.children.push_back(thenInfo);
                }
                
                auto elseNodes = ifInstr->getElse();
                if (elseNodes.size() > 0) {
                    LILToStrInfo thenInfo;
                    thenInfo.isExported = false;
                    thenInfo.value = "Else:";
                    for (auto it = ifInstr->getElse().begin(); it!=ifInstr->getElse().end(); ++it)
                    {
                        thenInfo.children.push_back(this->stringify((*it).get()));
                    };
                    if (thenInfo.children.size() > 0) {
                        ret.children.push_back(thenInfo);
                    }
                }
                
                break;
            }
            case InstructionTypeSnippet:
            {
                auto snInstr = static_cast<LILSnippetInstruction *>(value);
                ret.value = "#snippet: "+snInstr->getName();
                for (auto it = snInstr->getBody().begin(); it!=snInstr->getBody().end(); ++it)
                {
                    ret.children.push_back(this->stringify((*it).get()));
                };
                break;
            }
            default:
            {
                LILString nameStr;
                if (value->getName().length() > 0) {
                    nameStr = " " + value->getName();
                } else {
                    nameStr = "";
                }
                ret.value = "Instruction: #" + LILNodeToString::stringify(value) + nameStr;
                auto arg = value->getArgument();
                if (arg) {
                    ret.children.push_back(this->stringify(arg.get()));
                }
                return ret;
                break;
            }
        }
    }
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILForeignLang * value)
{
    LILToStrInfo ret;
    ret.value = "Foreign lang: <"+value->getLanguage()+">";
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILDocumentation * value)
{
    LILToStrInfo ret;
    ret.value = "Documentation: "+value->getContent();
    const auto & children = value->getChildNodes();
    for (auto node : children)
    {
        if (node->isA(NodeTypeDocumentation)) {
            LILToStrInfo child;
            child.isExported = false;
            child.value = std::static_pointer_cast<LILDocumentation>(node)->getContent();
            ret.children.push_back(child);
        }
    };
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILValueList * value)
{
    LILToStrInfo ret;
    ret.value = "Value list";
    this->stringifyChildren(value->getValues(), ret);
    return ret;
}

LILToStrInfo LILToStringVisitor::_stringify(LILIndexAccessor * value)
{
    LILToStrInfo ret;
    ret.value = "Index accessor: [";
    auto argInfo = this->stringify(value->getArgument().get());
    ret.value += argInfo.value;
    ret.value += "]";
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
