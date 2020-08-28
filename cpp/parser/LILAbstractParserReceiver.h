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
 *      This file defines the protocol to receive parser events
 *
 ********************************************************************/

#ifndef LILABSTRACTPARSERRECEIVER_H
#define LILABSTRACTPARSERRECEIVER_H

#include "LILTypeEnums.h"
#include "LILString.h"

namespace LIL
{
    class LILBuffer;

    class LILAbstractParserReceiver
    {
    public:
        virtual void reset() {};
        virtual void receiveNodeStart(NodeType nodeType) {};
        virtual void receiveNodeEnd(NodeType nodeType) {};
        virtual void receiveNodeCommit() {};
        virtual void receiveNodeData(ParserEvent eventType, const LILString & data) = 0;
        virtual void receiveSourceLocation(LILString file, size_t startLine, size_t startCol, LILRange newRange) {};
        virtual void receiveError(LILString message, LILString file, size_t startLine, size_t startCol) {};
    };
}

#endif
