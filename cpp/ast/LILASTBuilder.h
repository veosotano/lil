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
        BuilderStateConstDecl,
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
        BuilderStateIfInstruction,
        BuilderStateSnippetInstruction,
        BuilderStateClassDecl,
        BuilderStateObjectDefinition,
        BuilderStateFunctionDecl,
        BuilderStateFunctionCall,
        BuilderStateFlowControl,
        BuilderStateFlowControlCall,
        BuilderStateForeignLang,
        BuilderStateValueList,
        BuilderStateIndexAccessor,
        BuilderStateDocumentation,
        BuilderStateEnum,
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

        void setVerbose(bool value);
        void setDebugAST(bool value);
        void setBuildFlatList(bool value);
        bool getBuildFlatList() const;

    private:
        std::vector<std::shared_ptr<LILNode>> currentContainer;
        std::shared_ptr<LILNode> currentNode;
        std::vector<BuilderState> state;
        std::shared_ptr<LILRootNode> rootNode;
        bool _verbose;
        bool _debugAST;
        bool _buildFlatList;
    };
}

#endif
