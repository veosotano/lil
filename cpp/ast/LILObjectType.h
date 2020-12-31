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
 *      This file encapsulates the type for an object
 *
 ********************************************************************/

#ifndef LILOBJECTTYPE_H
#define LILOBJECTTYPE_H

#include "LILType.h"

namespace LIL
{
    class LILObjectType : public LILType
    {
    public:
        static std::shared_ptr<LILObjectType> make(LILString name);
        LILObjectType();
        LILObjectType(const LILObjectType &other);
        std::shared_ptr<LILObjectType> clone() const;
        virtual ~LILObjectType();
        bool equalTo(std::shared_ptr<LILNode> otherNode) override;
        void receiveNodeData(const LILString & data) override;

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
    };
}

#endif
