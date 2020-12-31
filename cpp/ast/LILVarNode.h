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

#ifndef LILVARNODE_H
#define LILVARNODE_H

#include "LILNode.h"

namespace LIL {

    class LILVarNode : public LILNode
    {
    public:
        LILVarNode(NodeType type);
        LILVarNode(const LILVarNode & other);
        virtual ~LILVarNode();
        virtual bool isVarNode() const;
        //searches itself and ancestors
        std::shared_ptr<LILNode> getVar(LILString name);
        //only searches itself
        virtual std::shared_ptr<LILNode> getVariable(LILString name);
        bool hasLocalVar(LILString name);
        virtual bool hasLocalVariable(LILString name);
        virtual void setLocalVariable(LILString name, std::shared_ptr<LILNode> value);
        virtual void unsetLocalVariable(LILString name);
        void clearLocalVars();

    private:
        std::map<LILString, std::shared_ptr<LILNode>> _localVars;
    };
}

#endif
