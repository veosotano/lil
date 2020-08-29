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
 *      This file encapsulates the type for a function
 *
 ********************************************************************/

#ifndef LILPOINTERTYPE_H
#define LILPOINTERTYPE_H

#include "LILType.h"

namespace LIL
{
    class LILPointerType : public LILType
    {
    public:
        static std::shared_ptr<LILPointerType> make(LILString typeName);
        LILPointerType();
        LILPointerType(const LILPointerType &other);
        std::shared_ptr<LILPointerType> clone() const;
        virtual ~LILPointerType();
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        virtual void receiveNodeData(const LILString & data);

        LILString stringRep();
        
        void setArgument(std::shared_ptr<LILType> node);
        std::shared_ptr<LILType> getArgument() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::shared_ptr<LILType> _argument;
    };
}

#endif
