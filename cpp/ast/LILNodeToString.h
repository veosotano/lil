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
 *      This file knows how to convert a node from the AST to a string
 *
 ********************************************************************/

#ifndef LILNODETOSTRING_H
#define LILNODETOSTRING_H

#include "LILShared.h"

namespace LIL {
    class LILNode;
    class LILNodeToString {
    public:
        static LILString stringify(LILNode * node);
    };
}


#endif /* LILNODETOSTRING_H */
