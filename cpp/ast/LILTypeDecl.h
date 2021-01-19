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
 *      This file encapsulates a type declaration
 *
 ********************************************************************/

#ifndef LILTYPEDECL_H
#define LILTYPEDECL_H

#include "LILNode.h"

namespace LIL
{
    class LILTypeDecl : public LILNode
    {
    public:
        LILTypeDecl();
        LILTypeDecl(const LILTypeDecl &other);
        std::shared_ptr<LILTypeDecl> clone() const;
        virtual ~LILTypeDecl();

        void receiveNodeData(const LIL::LILString &data) override;

        const std::shared_ptr<LILType> & getSrcType() const;
        void setSrcType(std::shared_ptr<LILType> value);
        const std::shared_ptr<LILType> & getDstType() const;
        void setDstType(std::shared_ptr<LILType> value);

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        std::shared_ptr<LILType> _srcTy;
        std::shared_ptr<LILType> _dstTy;
    };
}

#endif
