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
 *      This file encapsulates the name of a property
 *
 ********************************************************************/

#ifndef LILVARNAME_H
#define LILVARNAME_H

#include "LILNode.h"

namespace LIL
{
    class LILVarName : public LILNode
    {
    public:
        LILVarName();
        LILVarName(const LILVarName &other);
        std::shared_ptr<LILVarName> clone() const;
        virtual ~LILVarName();

        virtual void receiveNodeData(const LILString & data);

        LILString stringRep();

        void setName(LILString newName);
        const LILString getName() const;
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        LILString _name;
    };
}

#endif
