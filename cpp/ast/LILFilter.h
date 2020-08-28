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
 *      This file is a filter in a selector chain
 *
 ********************************************************************/

#ifndef LILFILTER_H
#define LILFILTER_H

#include "LILNode.h"

namespace LIL
{
    class LILFilter : public LILNode
    {
    public:
        LILFilter();
        LILFilter(const LILFilter &other);
        std::shared_ptr<LILFilter> clone() const;
        virtual ~LILFilter();
        
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
