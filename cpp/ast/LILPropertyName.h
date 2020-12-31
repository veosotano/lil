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

#ifndef LILPROPERTYNAME_H
#define LILPROPERTYNAME_H

#include "LILNode.h"

namespace LIL
{
    class LILPropertyName : public LILNode
    {
    public:
        LILPropertyName();
        LILPropertyName(const LILPropertyName &other);
        std::shared_ptr<LILPropertyName> clone() const;
        virtual ~LILPropertyName();

        void receiveNodeData(const LILString & data) override;

        bool equalTo(std::shared_ptr<LILNode> otherNode) override;

        void setName(LILString newName);
        const LILString getName() const;

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        LILString _name;
    };
}

#endif
