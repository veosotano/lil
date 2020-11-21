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

#ifndef LILFOREIGNLANG_H
#define LILFOREIGNLANG_H

#include "LILNode.h"

namespace LIL
{
    class LILForeignLang : public LILNode
    {
    public:
        LILForeignLang();
        LILForeignLang(const LILForeignLang &other);
        std::shared_ptr<LILForeignLang> clone() const;
        virtual ~LILForeignLang();

        virtual void receiveNodeData(const LILString & data);

        LILString stringRep();

        void setLanguage(LILString value);
        const LILString & getLanguage() const;

        void setContent(LILString value);
        const LILString & getContent() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;

    private:
        LILString _language;
        LILString _content;
    };
}

#endif
