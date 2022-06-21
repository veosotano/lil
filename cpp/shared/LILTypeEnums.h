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
 *      This file contains all the enums used to identify types and
 *      behaviors of the system
 *
 ********************************************************************/

#ifndef LILTYPES_H
#define LILTYPES_H

namespace LIL
{
    enum TokenType
    {
        TokenTypeNone = 0,
        TokenTypeIdentifier,
        TokenTypeNumberInt,
        TokenTypeNumberFP,
        TokenTypePercentageNumberInt,
        TokenTypePercentageNumberFP,
        TokenTypeHexNumber,
        TokenTypeDoubleQuoteString,
        TokenTypeSingleQuoteString,
        TokenTypeCString,
        TokenTypeWhitespace,
        TokenTypeInstructionSign,
        TokenTypeObjectSign,
        TokenTypeBlockOpen,
        TokenTypeBlockClose,
        TokenTypeComma,
        TokenTypeColon,
        TokenTypeSemicolon,
        TokenTypeParenthesisOpen,
        TokenTypeParenthesisClose,
        TokenTypeLineComment,
        TokenTypeBlockComment,
        TokenTypeNegator,
        TokenTypeAmpersand,
        TokenTypeDot,
        TokenTypeDoubleDot,
        TokenTypeEllipsis,
        TokenTypeEqualSign,
        TokenTypeBiggerComparator,
        TokenTypeBiggerOrEqualComparator,
        TokenTypeSmallerComparator,
        TokenTypeSmallerOrEqualComparator,
        TokenTypeSquareBracketOpen,
        TokenTypeSquareBracketClose,
        TokenTypePercentSign,
        TokenTypeMinusSign,
        TokenTypePlusSign,
        TokenTypeAsterisk,
        TokenTypeSlash,
        TokenTypeVerticalBar,
        TokenTypeThinArrow,
        TokenTypeFatArrow,
        TokenTypeForeignLang,
        TokenTypeDocumentation,
    };

    enum NodeType
    {
        NodeTypeRoot,
        NodeTypeNull,
        NodeTypeNegation,
        NodeTypeBoolLiteral,
        NodeTypeNumberLiteral,
        NodeTypePercentage,
        NodeTypeExpression,
        NodeTypeUnaryExpression,
        NodeTypeStringLiteral,
        NodeTypeStringFunction,
        NodeTypeCStringLiteral,
        NodeTypeType,
        //these are not actual node types, just for instantiating the concrete subclass:
        NodeTypeMultipleType,
        NodeTypeFunctionType,
        NodeTypePointerType,
        NodeTypeStaticArrayType,
        NodeTypeObjectType,
        NodeTypeSIMDType,
        //---
        NodeTypeVarName,
        NodeTypeVarDecl,
        NodeTypeConstDecl,
        NodeTypeAliasDecl,
        NodeTypeTypeDecl,
        NodeTypeConversionDecl,
        NodeTypeEnum,
        NodeTypeAssignment,
        NodeTypePropertyName,
        NodeTypeValuePath,
        NodeTypeSimpleSelector,
        NodeTypeSelector,
        NodeTypeCombinator,
        NodeTypeFilter,
        NodeTypeFlag,
        NodeTypeSelectorChain,
        NodeTypeRule,
        NodeTypeClassDecl,
        NodeTypeObjectDefinition,
        NodeTypeComment,
        NodeTypeInstruction,
        NodeTypeIfInstruction,
        NodeTypeSnippetInstruction,
        NodeTypeFunctionDecl,
        NodeTypeFunctionCall,
        NodeTypeFlowControl,
        NodeTypeFlowControlCall,
        NodeTypeIndexAccessor,
        NodeTypeValueList,
        NodeTypeForeignLang,
        NodeTypeDocumentation,
        NodeTypeInvalid,
    };

    enum TypeType
    {
        TypeTypeNone = 0,
        TypeTypeSingle,
        TypeTypeMultiple,
        TypeTypeFunction,
        TypeTypePointer,
        TypeTypeObject,
        TypeTypeStaticArray,
        TypeTypeSIMD,
    };

    enum ExpressionType
    {
        ExpressionTypeNone = 0,
        ExpressionTypeSum,
        ExpressionTypeSubtraction,
        ExpressionTypeMultiplication,
        ExpressionTypeDivision,
        ExpressionTypeEqualComparison,
        ExpressionTypeNotEqualComparison,
        ExpressionTypeBiggerComparison,
        ExpressionTypeBiggerOrEqualComparison,
        ExpressionTypeSmallerComparison,
        ExpressionTypeSmallerOrEqualComparison,
        ExpressionTypeLogicalAnd,
        ExpressionTypeLogicalOr,
        ExpressionTypeBitwiseAnd,
        ExpressionTypeBitwiseOr,
        ExpressionTypeXor,
        ExpressionTypeShiftLeft,
        ExpressionTypeShiftRight,
        ExpressionTypeMod,
        ExpressionTypeCast
    };
    
    enum UnaryExpressionType
    {
        UnaryExpressionTypeNone = 0,
        UnaryExpressionTypeSum,
        UnaryExpressionTypeSubtraction,
        UnaryExpressionTypeMultiplication,
        UnaryExpressionTypeDivision,
        UnaryExpressionTypeNot,
    };

    enum FunctionCallType
    {
        FunctionCallTypeNone = 0,
        //when used inside a value path:
        FunctionCallTypeValuePath,
        FunctionCallTypeRef,
        FunctionCallTypeSel,
        FunctionCallTypeFlag,
        FunctionCallTypeUnflag,
        FunctionCallTypeAddFlag,
        FunctionCallTypeTakeFlag,
        FunctionCallTypeReplaceFlag,
        FunctionCallTypePointerTo,
        FunctionCallTypeValueOf,
        FunctionCallTypeSet,
        FunctionCallTypeSizeOf,
        FunctionCallTypeConversion,
    };

    enum FunctionDeclType
    {
        FunctionDeclTypeNone = 0,
        FunctionDeclTypeFn,
        FunctionDeclTypeInsert,
        FunctionDeclTypeOverride,
        FunctionDeclTypeMacro,
    };

    enum FlowControlType
    {
        FlowControlTypeNone = 0,
        FlowControlTypeIf,
        FlowControlTypeIfCast,
        FlowControlTypeElse,
        FlowControlTypeSwitch,
        FlowControlTypeCase,
        FlowControlTypeDefault,
        FlowControlTypeLoop,
        FlowControlTypeFor,
        FlowControlTypeFinally,
    };

    enum FlowControlCallType
    {
        FlowControlCallTypeNone = 0,
        FlowControlCallTypeReturn,
        FlowControlCallTypeRepeat,
        FlowControlCallTypeContinue,
        FlowControlCallTypeBreak,
    };

    enum InstructionType
    {
        InstructionTypeNone = 0,
        InstructionTypeNew,
        InstructionTypeMove,
        InstructionTypeDelete,
        InstructionTypeGrayscale1,
        InstructionTypeGrayscale2,
        InstructionTypeRGB,
        InstructionTypeRGBA,
        InstructionTypeRGBAA,
        InstructionTypeRRGGBB,
        InstructionTypeRRGGBBA,
        InstructionTypeRRGGBBAA,
        InstructionTypeConfigure,
        InstructionTypeGetConfig,
        InstructionTypeNeeds,
        InstructionTypeImport,
        InstructionTypeExport,
        InstructionTypeIf,
        InstructionTypeSnippet,
        InstructionTypePaste,
        InstructionTypeBug,
        InstructionTypeArg,
        InstructionTypeExpand,
        InstructionTypeGPU,
        InstructionTypeResource,
    };

    enum SelectorType
    {
        SelectorTypeNone = 0,
        SelectorTypeNameSelector,
        SelectorTypeUniversalSelector,
        SelectorTypeThisSelector,
        SelectorTypeRootSelector,
        SelectorTypeParentSelector,
        SelectorTypeSuperSelector,
        SelectorTypeSelfSelector,
        SelectorTypeCombinator,
        SelectorTypeSimpleSelector,
        SelectorTypeMainMenu,
        SelectorTypeKey,
        SelectorTypeValue,
    };

    enum CombinatorType
    {
        CombinatorTypeNone = 0,
        CombinatorTypeSiblings,
        CombinatorTypePreviousSiblings,
        CombinatorTypeNextSiblings,
        CombinatorTypeChildren,
        CombinatorTypeDescendants,
        CombinatorTypeTextSelection,
    };

//    enum FilterType
//    {
//        FilterTypeNone = 0,
//        FilterTypeFirst,
//        FilterTypeLast,
//        FilterTypeEven,
//        FilterTypeOdd,
//        FilterTypeNth,
//        FilterTypeHas,
//        FilterTypeContains,
//        FilterTypeEquals,
//        FilterTypeStartsWith,
//        FilterTypeEndsWith,
//        FilterTypeMatch,
//        FilterTypeEmpty,
//        FilterTypeLength,
//        FilterTypeEach,
//        FilterTypeCustom,
//        FilterTypeFlag,
//    };

    enum FlagFunctionType
    {
        FlagFunctionTypeNone = 0,
        FlagFunctionTypeFlag,
        FlagFunctionTypeUnflag,
        FlagFunctionTypeAddFlag,
        FlagFunctionTypeTakeFlag,
        FlagFunctionTypeReplaceFlag,
    };

    enum EventType
    {
        EventTypeNone = 0,
        EventTypeLoad,
        EventTypeFlag,
        EventTypeUnflag,
        EventTypeClick,
        EventTypeDoubleClick,
        EventTypeTripleClick,
        EventTypeMouseDown,
        EventTypeMouseUp,
        EventTypeMouseOver,
        EventTypeMouseOut,
        EventTypeMouseHold,
        EventTypeMouseMove,
        EventTypeClickSecondary,
        EventTypeClickTertiary,
        EventTypeScroll,
        EventTypeExitedWindow,
        EventTypeKeyDown,
        EventTypeKeyUp,
        EventTypeSuccess,
        EventTypeError,
    };

    enum SelectionType
    {
        SelectionTypeNone = 0,
        SelectionTypeSimpleSelection,
        SelectionTypeMultipleSelection,
    };

    enum ParserEvent
    {
        ParserEventGeneric = 0,
        ParserEventPunctuation,
        ParserEventWhitespace,
        ParserEventComment,
        ParserEventNull,
        ParserEventBoolLiteral,
        ParserEventNumberLiteral,
        ParserEventNumberInt,
        ParserEventNumberFP,
        ParserEventPercentageLiteral,
        ParserEventExpression,
        ParserEventExpressionSign,
        ParserEventComparison,
        ParserEventStringLiteral,
        ParserEventStringFunctionStart,
        ParserEventStringFunctionMid,
        ParserEventStringFunctionArgEnd,
        ParserEventStringFunctionEnd,
        ParserEventStringFunctionComplete,
        ParserEventCStringLiteral,
        ParserEventValuePath,
        ParserEventPropertyName,
        ParserEventClassDecl,
        ParserEventInherits,
        ParserEventBody,
        ParserEventArgument,
        ParserEventParamType,
        ParserEventType,
        ParserEventReturnType,
        ParserEventVarDecl,
        ParserEventConstDecl,
        ParserEventAliasDecl,
        ParserEventTypeDecl,
        ParserEventConversionDecl,
        ParserEventEnum,
        ParserEventVarName,
        ParserEventConstName,
        ParserEventExtern,
        ParserEventObjectSign,
        ParserEventFunction,
        ParserEventFunctionName,
        ParserEventFunctionTypeFn,
        ParserEventFunctionModifier,
        ParserEventFunctionShorthand,
        ParserEventFunctionVariadic,
        ParserEventFlowControlIfCast,
        ParserEventFlowControlElse,
        ParserEventAssignment,
        ParserEventNeeds,
        ParserEventRule,
        ParserEventNameSelector,
        ParserEventUniversalSelector,
        ParserEventCombinatorChildren,
        ParserEventCombinatorNearestDescendants,
        ParserEventCombinatorDescendants,
        ParserEventCombinatorSiblings,
        ParserEventCombinatorPreviousSiblings,
        ParserEventCombinatorNextSiblings,
        ParserEventCombinatorTextSelection,
        ParserEventFilter,
        ParserEventFlag,
        ParserEventInstruction,
        ParserEventColorG1,
        ParserEventColorG2,
        ParserEventColorRGB,
        ParserEventColorRGBA,
        ParserEventColorRGBAA,
        ParserEventColorRRGGBB,
        ParserEventColorRRGGBBA,
        ParserEventColorRRGGBBAA,
        ParserEventIfInstruction,
        ParserEventIfInstructionElse,
        ParserEventForeignLang,
        ParserEventDocumentation,
        ParserEventInvalid,
    };
}

#endif
