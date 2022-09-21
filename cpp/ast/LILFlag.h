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
 *      This file is a flag in a selector chain
 *
 ********************************************************************/

#ifndef LILFLAG_H
#define LILFLAG_H

#include "LILNode.h"

namespace LIL
{
    class LILFlag : public LILNode
    {
    public:
        LILFlag();
        LILFlag(const LILFlag &other);
        std::shared_ptr<LILFlag> clone() const;
        virtual ~LILFlag();

        virtual void receiveNodeData(const LILString & data);

        void setName(LILString newName);
        const LILString getName() const;
        void setIsOnByDefault(bool value);
        bool getIsOnByDefault() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;

    private:
        LILString _name;
        bool _isOnByDefault;
    };
}

#endif
