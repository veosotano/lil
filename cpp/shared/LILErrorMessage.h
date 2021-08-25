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
 *      This file defines the structure for error messages
 *
 ********************************************************************/

#ifndef LILERRORMESSAGE_H
#define LILERRORMESSAGE_H

namespace LIL {
    struct LILErrorMessage {
        LILString message;
        LILString file;
        size_t line;
        size_t column;
    };
}

#endif
