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
 *      This file encapsulates an alias declaration
 *
 ********************************************************************/

#ifndef LILALIASDECL_H
#define LILALIASDECL_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILAliasDecl : public LILTypedNode
    {
    public:
        LILAliasDecl();
        LILAliasDecl(const LILAliasDecl &other);
        std::shared_ptr<LILAliasDecl> clone() const;
        virtual ~LILAliasDecl();

        LILString getName() const;
        void setName(LILString value);

        void receiveNodeData(const LIL::LILString &data) override;

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;

    private:
        LILString _name;
    };
}

#endif
