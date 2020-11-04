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
 *      This file represents a number written in the source code
 *
 ********************************************************************/

#ifndef LILPERCENTAGELITERAL_H
#define LILPERCENTAGELITERAL_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILPercentageLiteral : public LILTypedNode
    {
    public:
        LILPercentageLiteral();
        LILPercentageLiteral(const LILPercentageLiteral & other);
        std::shared_ptr<LILPercentageLiteral> clone() const;
        virtual ~LILPercentageLiteral();
        virtual void receiveNodeData(const LIL::LILString &data);
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        void setValue(LILString newValue);
        LILString getValue() const;
        LILString stringRep();

    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        LILString _value;
    };
}

#endif
