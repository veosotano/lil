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
 *      This file contains the LILNodeTypeToString function
 *
 ********************************************************************/

#include "LILShared.h"

using namespace LIL;

std::string LILNodeTypeToString(NodeType nodeType)
{
    switch (nodeType) {
        case NodeTypeRoot:
            return "root";
        case NodeTypeNull:
            return "null";
        case NodeTypeNegation:
            return "negation";
        case NodeTypeBoolLiteral:
            return "bool";
        case NodeTypeNumberLiteral:
            return "number literal";
        case NodeTypeStringLiteral:
            return "string";
        case NodeTypeStringFunction:
            return "string function";
        case NodeTypeCStringLiteral:
            return "C string";
        case NodeTypePercentage:
            return "percentage";
        case NodeTypeExpression:
            return "expression";
        case NodeTypeUnaryExpression:
            return "unary expression";
        case NodeTypeVarName:
            return "var name";
        case NodeTypeType:
            return "type";
        case NodeTypeFunctionType:
            return "funtion type";
        case NodeTypeObjectType:
            return "object type";
        case NodeTypePointerType:
            return "pointer type";
        case NodeTypeMultipleType:
            return "multiple type";
        case NodeTypeStaticArrayType:
            return "static array type";
        case NodeTypeVarDecl:
            return "var declaration";
        case NodeTypeAliasDecl:
            return "alias declaration";
        case NodeTypeTypeDecl:
            return "type declaration";
        case NodeTypeConversionDecl:
            return "conversion declaration";
        case NodeTypeAssignment:
            return "assignment";
        case NodeTypeValuePath:
            return "value path";
        case NodeTypePropertyName:
            return "propertyName";
        case NodeTypeSelector:
            return "selector";
        case NodeTypeCombinator:
            return "combinator";
        case NodeTypeFilter:
            return "filter";
        case NodeTypeFlag:
            return "flag";
        case NodeTypeSimpleSelector:
            return "simple selector";
        case NodeTypeSelectorChain:
            return "selector chain";
        case NodeTypeRule:
            return "rule";
        case NodeTypeClassDecl:
            return "class declaration";
        case NodeTypeObjectDefinition:
            return "object definition";
        case NodeTypeComment:
            return "comment";
        case NodeTypeInstruction:
            return "instruction";
        case NodeTypeIfInstruction:
            return "if instruction";
        case NodeTypeFunctionDecl:
            return "function declaration";
        case NodeTypeFunctionCall:
            return "function call";
        case NodeTypeIndexAccessor:
            return "index accessor";
        case NodeTypeFlowControl:
            return "flow control";
        case NodeTypeFlowControlCall:
            return "flow control call";
        case NodeTypeForeignLang:
            return "foreign language";
        case NodeTypeValueList:
            return "value list";
            
        default:
            return "ERROR: unknown node type";
    }
}
