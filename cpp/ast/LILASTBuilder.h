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
 *      This file instantiates nodes to make an abstract syntax tree
 *
 ********************************************************************/

#ifndef LILASTBUILDER_H
#define LILASTBUILDER_H

#include "LILAbstractParserReceiver.h"
#include "LILErrorMessage.h"

namespace LIL
{
    enum BuilderState
    {
        BuilderStateRoot = 0,
        BuilderStateBool,
        BuilderStateNumber,
        BuilderStatePercentage,
        BuilderStateString,
        BuilderStateStringFunction,
        BuilderStateNull,
        BuilderStateExpression,
        BuilderStateUnaryExpression,
        BuilderStateType,
        BuilderStateMultipleType,
        BuilderStateFunctionType,
        BuilderStatePointerType,
        BuilderStateStaticArrayType,
        BuilderStateObjectType,
        BuilderStateVarDecl,
        BuilderStateAliasDecl,
        BuilderStateTypeDecl,
        BuilderStateConversionDecl,
        BuilderStateVarName,
        BuilderStateRule,
        BuilderStateSimpleSelector,
        BuilderStateSelectorChain,
        BuilderStateSelector,
        BuilderStateCombinator,
        BuilderStateFilter,
        BuilderStateFlag,
        BuilderStateValuePath,
        BuilderStatePropertyName,
        BuilderStateAssignment,
        BuilderStateInstruction,
        BuilderStateClassDecl,
        BuilderStateObjectDefinition,
        BuilderStateFunctionDecl,
        BuilderStateFunctionCall,
        BuilderStateFlowControl,
        BuilderStateFlowControlCall,
        BuilderStateForeignLang,
        BuilderStateValueList,
        BuilderStateIndexAccessor,
    };
    class LILNode;
    class LILRootNode;
    class LILExpression;

    class LILASTBuilder : public LILAbstractParserReceiver
    {
    public:
        std::vector<LILErrorMessage> errors;

        LILASTBuilder();
        virtual ~LILASTBuilder();

        void reset() override;
        void receiveNodeStart(NodeType nodeType) override;
        void receiveNodeEnd(NodeType nodeType) override;
        void receiveNodeCommit() override;
        void receiveNodeData(ParserEvent eventType, const LILString &data) override;
        void receiveSourceLocation(LILString file, size_t startLine, size_t startCol, LILRange newRange) override;
        void receiveError(LILString message, LILString file, size_t startLine, size_t startCol) override;
        void receiveForeignLang(const LILString & language, const LILString & content) override;

        std::shared_ptr<LILRootNode> getRootNode() const;
        bool hasErrors() const;
        void printErrors(const LILString & code) const;
        std::vector<std::string> splitString(std::string code, std::string delimiter) const;
        
        void setIsMain(bool value);
        void setVerbose(bool value);
        void setDebugAST(bool value);

    private:
        std::vector<std::shared_ptr<LILNode>> currentContainer;
        std::shared_ptr<LILNode> currentNode;
        std::vector<BuilderState> state;
        std::shared_ptr<LILRootNode> rootNode;
        bool _isMain;
        bool _verbose;
        bool _debugAST;
    };
}

#endif
