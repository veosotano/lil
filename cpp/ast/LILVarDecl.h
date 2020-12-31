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
 *      This file represents var declaration
 *
 ********************************************************************/

#ifndef LILVARDECL_H
#define LILVARDECL_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILExpression;
    class LILVarDecl : public LILTypedNode
    {
    public:
        LILVarDecl();
        LILVarDecl(const LILVarDecl & orig);
        std::shared_ptr<LILVarDecl> clone() const;
        virtual ~LILVarDecl();

        void receiveNodeData(const LIL::LILString &data);

        const LILString getName() const;
        void setName(LILString newName);

        std::shared_ptr<LILNode> getInitVal() const;
        void setInitVal(std::shared_ptr<LILNode> value);
        void setInitVals(const std::vector<std::shared_ptr<LILNode>> & values);

        bool getIsExtern() const;
        void setIsExtern(bool value);
        bool getIsIVar() const;
        void setIsIVar(bool value);
        bool getIsVVar() const;
        void setIsVVar(bool value);
        bool getIsConst() const;
        void setIsConst(bool value);

    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;

        LILString _name;
        bool _isExtern;
        bool _isIVar;
        bool _isVVar;
        bool _isConst;
    };
}

#endif
