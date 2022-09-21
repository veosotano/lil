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
    class LILDocumentation;
    class LILEnum;
    class LILFunctionDecl;
    class LILInstruction;
    class LILTypeDecl;
    class LILRule;
    class LILSnippetInstruction;
    class LILVarDecl;

    class LILRootNode : public LILVarNode
    {
    public:
        friend class LILCodeUnit;

        LILRootNode();
        LILRootNode(const LILRootNode & other);
        virtual ~LILRootNode();
        bool isRootNode() const override;
        
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        void add(std::shared_ptr<LILNode> node, bool addToNodeTree = true);
        void clearNodes();
        void appendNodes(const std::vector<std::shared_ptr<LILNode>> & nodes);

        void addClass(std::shared_ptr<LILClassDecl> value);
        const std::vector<std::shared_ptr<LILClassDecl>> & getClasses() const;
        void removeClass(std::shared_ptr<LILClassDecl> value);

        void addAlias(std::shared_ptr<LILAliasDecl> value);
        const std::vector<std::shared_ptr<LILAliasDecl>> & getAliases() const;

        void addType(std::shared_ptr<LILTypeDecl> value);
        const std::vector<std::shared_ptr<LILTypeDecl>> & getTypes() const;
        
        void addEnum(std::shared_ptr<LILEnum> value);
        const std::vector<std::shared_ptr<LILEnum>> & getEnums() const;
        
        void addConversion(std::shared_ptr<LILConversionDecl> value);
        const std::map<LILString, std::shared_ptr<LILConversionDecl>> & getConversions() const;
        std::shared_ptr<LILConversionDecl> getConversionNamed(LILString name);

        const std::map<LILString, std::shared_ptr<LILSnippetInstruction>> & getSnippets() const;
        std::shared_ptr<LILSnippetInstruction> getSnippetNamed(LILString key);
        void addSnippet(std::shared_ptr<LILSnippetInstruction> snippet);
        void addEvaluable(std::shared_ptr<LILNode> node);

        bool hasInitializers() const;
        const std::vector<std::shared_ptr<LILNode>> & getInitializers() const;
        void clearInitializers();

        void addDoc(std::shared_ptr<LILDocumentation> value);
        const std::vector<std::shared_ptr<LILDocumentation>> & getDocs() const;

        bool hasRules() const;
        void addRule(std::shared_ptr<LILRule> value);
        const std::vector<std::shared_ptr<LILRule>> & getRules() const;

        bool hasMainMenu() const;
        const std::vector<std::shared_ptr<LILNode>> & getMainMenuItems() const;
        void clearMainMenuItems();
        
        void addConfigureInstr(const std::shared_ptr<LILInstruction> & instr);
        const std::vector<std::shared_ptr<LILNode>> & getConfigure() const;

        void addGPUNode(const std::shared_ptr<LILNode> & node);
        const std::vector<std::shared_ptr<LILNode>> & getGPUNodes() const;

        std::shared_ptr<LILClassDecl> findClassWithName(const LILString & name) const;
        std::shared_ptr<LILEnum> findEnumWithName(const LILString & name) const;

    private:
        std::map<LILString, std::shared_ptr<LILNode>> _localVars;
        std::vector<std::shared_ptr<LILClassDecl>> _classes;
        std::vector<std::shared_ptr<LILAliasDecl>> _aliases;
        std::vector<std::shared_ptr<LILTypeDecl>> _types;
        std::vector<std::shared_ptr<LILEnum>> _enums;
        std::map<LILString, std::shared_ptr<LILConversionDecl>> _conversions;
        std::vector<std::shared_ptr<LILVarDecl>> _constants;
        std::map<LILString, std::shared_ptr<LILSnippetInstruction>> _snippets;
        std::vector<std::shared_ptr<LILNode>> _initializers;
        std::vector<std::shared_ptr<LILDocumentation>> _docs;
        std::vector<std::shared_ptr<LILRule>> _rules;
        std::vector<std::shared_ptr<LILNode>> _mainMenu;
        std::vector<std::shared_ptr<LILNode>> _config;
        std::vector<std::shared_ptr<LILNode>> _gpuNodes;
    };
}

#endif
