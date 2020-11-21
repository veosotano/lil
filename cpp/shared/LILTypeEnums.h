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
        TokenTypeEqualComparator,
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
        NodeTypeObjectType,
        //---
        NodeTypeVarName,
        NodeTypeVarDecl,
        NodeTypeAliasDecl,
        NodeTypeTypeDecl,
        NodeTypeConversionDecl,
        NodeTypeAssignment,
        NodeTypePropertyName,
        NodeTypeValuePath,
        NodeTypeSimpleSelector,
        NodeTypeSelector,
        NodeTypeRootSelector,
        NodeTypeCombinator,
        NodeTypeFilter,
        NodeTypeFlag,
        NodeTypeSelectorChain,
        NodeTypeRule,
        NodeTypeClassDecl,
        NodeTypeObjectDefinition,
        NodeTypeComment,
        NodeTypeInstruction,
        NodeTypeFunctionDecl,
        NodeTypeFunctionCall,
        NodeTypeFlowControl,
        NodeTypeFlowControlCall,
        NodeTypeArgument,
        NodeTypeArray,
        NodeTypeIndexAccessor,
        NodeTypeNativeMethod,
        NodeTypeWhitespaceNode,
        NodeTypeCommentNode,
        NodeTypeForeignLang,
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
        ExpressionTypeCast,
    };
    
    enum UnaryExpressionType
    {
        UnaryExpressionTypeNone = 0,
        UnaryExpressionTypeSum,
        UnaryExpressionTypeSubtraction,
        UnaryExpressionTypeMultiplication,
        UnaryExpressionTypeDivision,
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
        FlowControlTypeIfIs,
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
        InstructionTypeNeeds,
        InstructionTypeFinally,
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

    enum RuleState
    {
        RuleStateOff = 0,
        RuleStateOn = 1,
        RuleStateActivate,
        RuleStatePurge,
    };

    enum ShapeType
    {
        ShapeTypeNone = 0,
        ShapeTypeRectangle,
        ShapeTypeRoundedRect,
        ShapeTypeCircle,
        ShapeTypePolygon,
        ShapeTypeStar,
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

    enum BorderType
    {
        BorderTypeNone = 0,
        BorderTypeLine,
    };

    enum BorderPosition
    {
        BorderPositionNone = 0,
        BorderPositionInside,
        BorderPositionCenter,
        BorderPositionOutside,
    };

    enum SelectionType
    {
        SelectionTypeNone = 0,
        SelectionTypeSimpleSelection,
        SelectionTypeMultipleSelection,
    };

    enum GradientType
    {
        GradientTypeNone = 0,
        GradientTypeLinear,
        GradientTypeRadial,
    };

    enum DirectionValue
    {
        DirectionLeftToRight,
        DirectionRightToLeft,
        DirectionTopToBottom,
        DirectionBottomToTop,
    };

    enum PathCommandType
    {
        PathCommandNone,
        PathCommandTypeMoveTo,
        PathCommandTypeLineTo,
        PathCommandTypeArcTo,
        PathCommandTypeSubtract,
        PathCommandTypeCloseSubpath,
        PathCommandTypeAddPolygon,
        PathCommandTypeAddEllipse,
    };

    enum FlowMode
    {
        FlowModeRunOnce,
        FlowModeRepeat,
        FlowModeBreak,
        FlowModeContinue,
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
        ParserEventType,
        ParserEventReturnType,
        ParserEventVarDecl,
        ParserEventAliasDecl,
        ParserEventTypeDecl,
        ParserEventConversionDecl,
        ParserEventVarName,
        ParserEventExtern,
        ParserEventObjectDefinition,
        ParserEventFunction,
        ParserEventFunctionTypeFn,
        ParserEventFunctionBody,
        ParserEventFunctionModifier,
        ParserEventFunctionShorthand,
        ParserEventFunctionVariadic,
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
        ParserEventForeignLang,
        ParserEventInvalid,
    };

    enum TextTransformType
    {
        TextTransformTypeNone = 0,
        TextTransformTypeLowercase,
        TextTransformTypeUppercase,
        TextTransformTypeCapitalize,
        TextTransformTypeHumanize,
    };

    enum TextAlignType
    {
        TextAlignTypeNone = 0,
        TextAlignTypeLeft,
        TextAlignTypeRight,
        TextAlignTypeCenter,
        TextAlignTypeJustify,
    };

    enum LoggerChannel
    {
        LoggerChannelNone = 0,
        LoggerChannelUserError = 1 << 0,
        LoggerChannelUserWarning = 1 << 1,
        LoggerChannelOverview = 1 << 2,
        LoggerChannelGeneral = 1 << 3,
        LoggerChannelGeneralSpecific = 1 << 4,
        LoggerChannelIO = 1 << 5,
        LoggerChannelNetwork = 1 << 6,
        LoggerChannelLogFunction = 1 << 7,
        LoggerChannelParser = 1 << 8,
        LoggerChannelLexer = 1 << 9,
        LoggerChannelLayout = 1 << 10,
        LoggerChannelRendering = 1 << 11,
        LoggerChannelObserving = 1 << 12,
        LoggerChannelEvents = 1 << 13,
        LoggerChannelEventsSpecific = 1 << 14,
        LoggerChannelDocument = 1 << 15,
        LoggerChannelDocumentSpecific = 1 << 16,
        LoggerChannelMax = 1 << 30,
        LoggerChannelAll = 0x7fffffff,
    };

    enum VisitorFilterFlags
    {
        LILVisitorFilterNone = 0,
        LILVisitorFilterAll = 0x01,
        LILVisitorFilterSkip = 0x02,
        LILVisitorFilterRendering = 0x04,
        LILVisitorFilterDiagnostic = 0x08,
        LILVisitorFilterCascading = 0x10,
        LILVisitorFilterLayout = 0x20,
        LILVisitorFilterTraverse = 0x100,
    };
}

#endif
