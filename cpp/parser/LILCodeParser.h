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
 *      This file reads LIL source code and tells a delegate
 *      about it
 *
 ********************************************************************/

#ifndef LILCODEPARSER_H
#define LILCODEPARSER_H

#include "LILBasicValues.h"
#include "LILString.h"

namespace LIL
{
    class LILCodeParserPrivate;
    class LILAbstractParserReceiver;
    class LILToken;

    class LILCodeParser
    {
    public:
        LILCodeParser(LILAbstractParserReceiver * receiver);
        virtual ~LILCodeParser();
        void parseString(const LILString & theString);
        void readNextToken();
        void updateCurrentToken(std::shared_ptr<LILToken> theToken);
        bool atEndOfSource() const;
        void skip(TokenType type);
        void skipInvalidToken();
        void skipStringArgument();
        void skipUntilSemicolon();
        void skipUntilEndOfValuePath();
        void parseNext();

        bool isBool() const;
        bool isAssignment() const;
        bool isRule() const;
        bool isInstructionRule() const;
        bool isValuePath() const;
        bool isExpression() const;
        bool isUnaryExpression() const;
        //this function only checks the sign before the colon
        bool isUnaryExpressionSign(std::shared_ptr<LILToken> theToken) const;
        bool isCast() const;
        bool isObjectSelector() const;
        bool isColorInstruction() const;
        bool isComparisonSign() const;
        bool isCombinator() const;
        bool isCombinator(std::shared_ptr<LILToken> token) const;
        bool isChildrenCombinator() const;
        bool isNegator() const;
        bool isIndexAccessor() const;
        bool isFunctionCall(bool isPastIdentifier) const;
        bool isBuiltinFunctionCall() const;
        bool isFlowControlCall() const;
        bool isFunctionDecl() const;
        bool isFlowControl() const;
        bool isIfCast() const;
        bool isFlag() const;
        bool isPunctuation(std::shared_ptr<LILToken> token) const;

        bool readClassDecl();
        bool readType();
        bool readTypeSimple();
        bool readFunctionType(bool readFnKw);
        bool readPointerType();
        bool readStaticArrayType();
        bool readVarDecl();
        bool readConstDecl();
        bool readAliasDecl();
        bool readTypeDecl();
        bool readConversionDecl();
        bool readAssignment(bool allowValuePath, bool firstIsVar, bool firstPartAlreadyRead = false);
        //readVals auto commits
        bool readVals();
        bool readVals(bool useComma, bool strict);
        bool readExpression(bool &outIsSingleValue, NodeType & outType);
        bool readExpressionInner();
        bool readUnaryExpression();
        bool readSingleValue(NodeType &nodeType);
        bool readBasicValue(NodeType &nodeType);
        bool readValueList(bool useComma);
        bool readStringLiteral();
        bool readStringFunction();
        bool readStringArgument();
        bool readObjectType();
        bool readObjectPath();
        bool readObjectDefinition();
        bool readRule();
        bool readSimpleSelector();
        bool readObjectSelector();
        bool readFilterOrFlag();
        bool readCombinator();
        bool readChildrenCombinatorOrSkip();
        bool readSymbolCombinator();
        bool readNameSelector(bool isNegating);
        bool readInstruction();
        bool readColor();
        bool readNeedsOrImportInstr();
        bool readExportInstr();
        bool readIfInstr();
        bool readSnippetInstr();
        bool readPasteInstr();
        bool readNewInstr();
        bool readInstructionRule();
        bool readValuePath(bool allowFunctionCall);
        bool readVarName();
        bool readPropertyName();
        bool readIndexAccessor();
        //readSelectorChains auto commits
        bool readSelectorChains(TokenType stopOn);
        bool readSelectorChain(TokenType stopOn);
        bool readComparison();

        bool readFunctionDecl();
        bool readFnFunction();
        bool readOverrideFunction();
        bool readInsertFunction();
        //readEvaluables auto commits
        bool readEvaluables();
        
        bool readFunctionCall();
        bool readFunctionCallSimple();
        bool readStandardFunctionCall(bool readIdentifier);
        bool readNameAndSelectorFunctionCall();
        bool readSelFunction();
        bool readFlagFunction();
        bool readSingleArgumentFunctionCall(const LILString & name);

        bool readFlowControl();
        bool readIfFlowControl();
        bool readSwitchFlowControl();
        bool readSwitchDefault();
        bool readSwitchCase();
        bool readLoopFlowControl();
        bool readForFlowControl();
        bool readFinallyFlowControl();

        bool readFlowControlCall();
        bool readReturnFlowControlCall();
        bool readRepeatFlowControlCall();
        bool readBreakFlowControlCall();
        bool readContinueFlowControlCall();

        bool readForeignLang();
        bool readDocumentation();
    private:
        LILCodeParserPrivate *const d;
    };
}
#endif
