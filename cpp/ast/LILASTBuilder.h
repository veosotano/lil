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
        BuilderStateType,
        BuilderStateMultipleType,
        BuilderStateFunctionType,
        BuilderStatePointerType,
        BuilderStateObjectType,
        BuilderStateVarDecl,
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

        void reset();
        void receiveNodeStart(NodeType nodeType);
        void receiveNodeEnd(NodeType nodeType);
        void receiveNodeCommit();
        void receiveNodeData(ParserEvent eventType, const LILString &data);
        void receiveSourceLocation(LILString file, size_t startLine, size_t startCol, LILRange newRange);
        void receiveError(LILString message, LILString file, size_t startLine, size_t startCol);

        std::shared_ptr<LILRootNode> getRootNode() const;
        bool hasErrors() const;
        void printErrors(const LILString & code) const;
        std::vector<std::string> splitString(std::string code, std::string delimiter) const;
        
        void setVerbose(bool value);
        void setDebugAST(bool value);

    private:
        std::vector<std::shared_ptr<LILNode>> currentContainer;
        std::shared_ptr<LILNode> currentNode;
        std::vector<BuilderState> state;
        std::shared_ptr<LILRootNode> rootNode;
        bool _verbose;
        bool _debugAST;
    };
}

#endif
