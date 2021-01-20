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
 *      This file handles the preprocessing of the AST
 *
 ********************************************************************/

#ifndef LILPREPROCESSOR_H
#define LILPREPROCESSOR_H

#include "LILVisitor.h"
#include "LILAssignment.h"
#include "LILClassDecl.h"
#include "LILConversionDecl.h"
#include "LILExpression.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILIfInstruction.h"
#include "LILIndexAccessor.h"
#include "LILInstruction.h"
#include "LILMultipleType.h"
#include "LILObjectDefinition.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILRule.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILSnippetInstruction.h"
#include "LILStringFunction.h"
#include "LILType.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"

namespace LIL
{
    class LILRootNode;
    class LILPreprocessor : public LILVisitor
    {
    public:
        LILPreprocessor();
        virtual ~LILPreprocessor();
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void processImportingInstr(const std::shared_ptr<LILRootNode> & rootNode);
        void processIfInstr(const std::shared_ptr<LILRootNode> & rootNode);
        bool processIfInstr(std::shared_ptr<LILNode> node);
        void processPasteInstr(const std::shared_ptr<LILRootNode> & rootNode);
        bool processPasteInstr(std::shared_ptr<LILNode> node);
        void removeSnippets(std::shared_ptr<LILRootNode> rootNode);
        void _removeSnippets(std::shared_ptr<LILNode> node);
        void setDir(LILString dir);
        LILString getDir() const;
        bool getDebugAST() const;
        void setDebugAST(bool value);
        void addAlreadyImportedFile(LILString path, std::vector<std::shared_ptr<LILNode>> nodes, bool isNeeds);
        bool isAlreadyImported(const LILString & path, bool isNeeds);
        std::vector<std::shared_ptr<LILNode>> getNodesForAlreadyImportedFile(const LILString & path, bool isNeeds);

    private:
        std::map<LILString, std::vector<std::shared_ptr<LILNode>>> _alreadyImportedFilesNeeds;
        std::map<LILString, std::vector<std::shared_ptr<LILNode>>> _alreadyImportedFilesImport;
        std::vector<std::vector<std::shared_ptr<LILNode>>> _nodeBuffer;
        LILString _dir;
        bool _debugAST;
        bool _needsAnotherPass;

        std::vector<LILString> _resolveFilePaths(LILString argStr) const;
        std::vector<std::string> _glob(const std::string& pattern) const;
        void _importNodeIfNeeded(std::vector<std::shared_ptr<LILNode>> * newNodes, std::shared_ptr<LILNode> node, bool isExported) const;
        LILString _getDir(LILString path) const;
        bool _processIfInstr(std::shared_ptr<LILExpression> value);
        bool _processIfInstr(std::shared_ptr<LILUnaryExpression> value);
        bool _processIfInstr(std::shared_ptr<LILStringFunction> value);
        bool _processIfInstr(std::shared_ptr<LILType> value);
        bool _processIfInstr(std::shared_ptr<LILVarDecl> value);
        bool _processIfInstr(std::shared_ptr<LILClassDecl> value);
        bool _processIfInstr(std::shared_ptr<LILObjectDefinition> value);
        bool _processIfInstr(std::shared_ptr<LILAssignment> value);
        bool _processIfInstr(std::shared_ptr<LILValuePath> value);
        bool _processIfInstr(std::shared_ptr<LILPropertyName> value);
        bool _processIfInstr(std::shared_ptr<LILVarName> value);
        bool _processIfInstr(std::shared_ptr<LILRule> value);
        bool _processIfInstr(std::shared_ptr<LILSelectorChain> value);
        bool _processIfInstr(std::shared_ptr<LILSimpleSelector> value);
        bool _processIfInstr(std::shared_ptr<LILFunctionDecl> value);
        bool _processIfInstr(std::shared_ptr<LILFunctionCall> value);
        bool _processIfInstr(std::shared_ptr<LILFlowControl> value);
        bool _processIfInstr(std::shared_ptr<LILFlowControlCall> value);
        bool _processIfInstr(std::shared_ptr<LILInstruction> value);
        bool _processIfInstrIfInstr(std::shared_ptr<LILIfInstruction> value);
        bool _processIfInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value);
        bool _processIfInstr(std::shared_ptr<LILValueList> value);
        bool _processIfInstr(std::shared_ptr<LILIndexAccessor> value);
        bool _processIfInstr(std::shared_ptr<LILConversionDecl> value);
        bool _evaluate(std::shared_ptr<LILNode> node);

        bool _processPasteInstr(std::shared_ptr<LILExpression> value);
        bool _processPasteInstr(std::shared_ptr<LILUnaryExpression> value);
        bool _processPasteInstr(std::shared_ptr<LILStringFunction> value);
        bool _processPasteInstr(std::shared_ptr<LILType> value);
        bool _processPasteInstr(std::shared_ptr<LILVarDecl> value);
        bool _processPasteInstr(std::shared_ptr<LILClassDecl> value);
        bool _processPasteInstr(std::shared_ptr<LILObjectDefinition> value);
        bool _processPasteInstr(std::shared_ptr<LILAssignment> value);
        bool _processPasteInstr(std::shared_ptr<LILValuePath> value);
        bool _processPasteInstr(std::shared_ptr<LILPropertyName> value);
        bool _processPasteInstr(std::shared_ptr<LILVarName> value);
        bool _processPasteInstr(std::shared_ptr<LILRule> value);
        bool _processPasteInstr(std::shared_ptr<LILSelectorChain> value);
        bool _processPasteInstr(std::shared_ptr<LILSimpleSelector> value);
        bool _processPasteInstr(std::shared_ptr<LILFunctionDecl> value);
        bool _processPasteInstr(std::shared_ptr<LILFunctionCall> value);
        bool _processPasteInstr(std::shared_ptr<LILFlowControl> value);
        bool _processPasteInstr(std::shared_ptr<LILFlowControlCall> value);
        bool _processPasteInstr(std::shared_ptr<LILInstruction> value);
        bool _processPasteInstrIfInstr(std::shared_ptr<LILIfInstruction> value);
        bool _processPasteInstrSnippetInstr(std::shared_ptr<LILSnippetInstruction> value);
        bool _processPasteInstr(std::shared_ptr<LILValueList> value);
        bool _processPasteInstr(std::shared_ptr<LILIndexAccessor> value);
        bool _processPasteInstr(std::shared_ptr<LILConversionDecl> value);
    };
}


#endif /* LILPREPROCESSOR_H */
