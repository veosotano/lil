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
 *      This is a node that participates in local variables
 *
 ********************************************************************/

#ifndef LILROOTNODE_H
#define LILROOTNODE_H

#include "LILVarNode.h"

namespace LIL {
    
    class LILAliasDecl;
    class LILClassDecl;
    class LILConversionDecl;
    class LILFunctionDecl;
    class LILInstruction;
    class LILTypeDecl;
    class LILSnippetInstruction;
    class LILVarDecl;

    class LILRootNode : public LILVarNode
    {
    public:
        LILRootNode();
        LILRootNode(const LILRootNode & other);
        virtual ~LILRootNode();
        virtual LILString stringRep();
        virtual bool isRootNode() const;
        
        std::shared_ptr<LILFunctionDecl> getMainFn() const;
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        void clearNodes();
        void appendNodes(const std::vector<std::shared_ptr<LILNode>> & nodes);

        void addClass(std::shared_ptr<LILClassDecl> value);
        const std::vector<std::shared_ptr<LILClassDecl>> & getClasses() const;
        void removeClass(std::shared_ptr<LILClassDecl> value);

        void addAlias(std::shared_ptr<LILAliasDecl> value);
        const std::vector<std::shared_ptr<LILAliasDecl>> & getAliases() const;

        void addType(std::shared_ptr<LILTypeDecl> value);
        const std::vector<std::shared_ptr<LILTypeDecl>> & getTypes() const;
        
        void addConversion(std::shared_ptr<LILConversionDecl> value);
        const std::map<LILString, std::shared_ptr<LILConversionDecl>> & getConversions() const;
        std::shared_ptr<LILConversionDecl> getConversionNamed(LILString name);

        const std::vector<std::shared_ptr<LILVarDecl>> & getConstants() const;
        void addConstant(std::shared_ptr<LILVarDecl> vd);
        
        const std::map<LILString, std::shared_ptr<LILSnippetInstruction>> & getSnippets() const;
        std::shared_ptr<LILSnippetInstruction> getSnippetNamed(LILString key);
        void addSnippet(std::shared_ptr<LILSnippetInstruction> snippet);
        void addEvaluable(std::shared_ptr<LILNode> node);
    private:
        std::map<LILString, std::shared_ptr<LILNode>> _localVars;
        std::shared_ptr<LILFunctionDecl> _mainFunction;
        std::vector<std::shared_ptr<LILClassDecl>> _classes;
        std::vector<std::shared_ptr<LILAliasDecl>> _aliases;
        std::vector<std::shared_ptr<LILTypeDecl>> _types;
        std::map<LILString, std::shared_ptr<LILConversionDecl>> _conversions;
        std::vector<std::shared_ptr<LILVarDecl>> _constants;
        std::map<LILString, std::shared_ptr<LILSnippetInstruction>> _snippets;
        std::vector<std::shared_ptr<LILNode>> _initializers;
    };
}

#endif
