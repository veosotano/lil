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
    
    class LILClassDecl;
    class LILFunctionDecl;
    class LILInstruction;

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

        void addClass(std::shared_ptr<LILClassDecl> value);
        std::vector<std::shared_ptr<LILClassDecl>> getClasses() const;
        
        void addDependency(std::shared_ptr<LILInstruction> value);
        std::vector<std::shared_ptr<LILInstruction>> getDependencies() const;

    private:
        std::map<LILString, std::shared_ptr<LILNode>> _localVars;
        std::shared_ptr<LILFunctionDecl> _mainFunction;
        std::vector<std::shared_ptr<LILClassDecl>> _classes;
        std::vector<std::shared_ptr<LILInstruction>> _dependencies;
    };
}

#endif
