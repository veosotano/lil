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

#include "LILNode.h"

namespace LIL
{
    class LILAliasDecl : public LILNode
    {
    public:
        LILAliasDecl();
        LILAliasDecl(const LILAliasDecl &other);
        std::shared_ptr<LILAliasDecl> clone() const;
        virtual ~LILAliasDecl();

        const std::shared_ptr<LILType> & getSrcType() const;
        void setSrcType(std::shared_ptr<LILType> value);
        const std::shared_ptr<LILType> & getDstType() const;
        void setDstType(std::shared_ptr<LILType> value);

        void receiveNodeData(const LIL::LILString &data) override;

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;

    private:
        std::shared_ptr<LILType> _srcTy;
        std::shared_ptr<LILType> _dstTy;
    };
}

#endif
