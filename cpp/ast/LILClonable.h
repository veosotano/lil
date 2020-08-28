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
 *      This file implements cloning behavior
 *
 ********************************************************************/

#ifndef LILCLONABLE_H
#define LILCLONABLE_H

namespace LIL
{
    class LILClonable
    {
    public:
        virtual ~LILClonable();
        std::shared_ptr<LILClonable> clone() const;

    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
    };
}

#endif
