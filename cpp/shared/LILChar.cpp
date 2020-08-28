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
 *      This file implements a character in LIL
 *
 ********************************************************************/

#include "LILShared.h"
#include "LILChar.h"

using namespace LIL;

namespace LIL
{
    class LILCharPrivate
    {
        friend class LILChar;
        LILCharPrivate()
        {

        }
        LILUnitI64 value;
    };
}

LILChar::LILChar()
: d(new LILCharPrivate)
{

}

LILChar::LILChar(const LILChar & other)
: d(new LILCharPrivate)
{
    d->value = other.d->value;
}

LILChar::LILChar(LILUnitI64 value)
: d(new LILCharPrivate)
{
    d->value = value;
}

LILChar::~LILChar()
{
    delete d;
}

LILChar & LILChar::operator=(const LILChar &other)
{
    d->value = other.data();
    return *this;
}

LILChar & LILChar::operator=(const char * other)
{
    d->value = static_cast<uint32_t>(*other);
    return *this;
}

LILUnitI64 LILChar::data() const
{
    return d->value;
}

bool LILChar::isSpace() const
{
    return std::isspace(static_cast<unsigned char>(this->data()));
}

bool LILChar::isDigit() const
{
    return std::isdigit(static_cast<unsigned char>(this->data()));
}
