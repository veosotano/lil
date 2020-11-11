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

#include "LILTypedNode.h"

namespace LIL
{
    class LILTypeDecl : public LILTypedNode
    {
    public:
        LILTypeDecl();
        LILTypeDecl(const LILTypeDecl &other);
        std::shared_ptr<LILTypeDecl> clone() const;
        virtual ~LILTypeDecl();
        
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
