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
 *      This file is used to express C-style arrays
 *
 ********************************************************************/

#ifndef LILSTATICARRAYTYPE_H
#define LILSTATICARRAYTYPE_H

#include "LILType.h"

namespace LIL
{
    class LILStaticArrayType : public LILType
    {
    public:
        LILStaticArrayType();
        LILStaticArrayType(const LILStaticArrayType &other);
        std::shared_ptr<LILStaticArrayType> clone() const;
        virtual ~LILStaticArrayType();
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        virtual void receiveNodeData(const LILString & data);
        
        LILString stringRep();
        
        void setArgument(std::shared_ptr<LILNode> node);
        std::shared_ptr<LILNode> getArgument() const;

        void setType(std::shared_ptr<LILType> type);
        std::shared_ptr<LILType> getType() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::shared_ptr<LILNode> _argument;
        std::shared_ptr<LILType> _type;
    };
}

#endif
