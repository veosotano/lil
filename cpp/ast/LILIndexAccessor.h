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
  *      This file holds the node used to access into an array
 *
 ********************************************************************/

#ifndef LILINDEXACCESSOR_H
#define LILINDEXACCESSOR_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILIndexAccessor : public LILTypedNode
    {
    public:
        LILIndexAccessor();
        LILIndexAccessor(const LILIndexAccessor &other);
        std::shared_ptr<LILIndexAccessor> clone() const;
        virtual ~LILIndexAccessor();

        const std::shared_ptr<LILNode> & getArgument() const;
        void setArgument(std::shared_ptr<LILNode> newValue);

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::shared_ptr<LILNode> _argument;

    };
}

#endif
