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

#include "LILNode.h"

namespace LIL
{
    class LILPercentageLiteral : public LILNode
    {
    public:
        LILPercentageLiteral();
        LILPercentageLiteral(const LILPercentageLiteral & other);
        std::shared_ptr<LILPercentageLiteral> clone() const;
        virtual ~LILPercentageLiteral();
        virtual void receiveNodeData(const LIL::LILString &data);
        void setValue(LILUnitF64 newValue);
        LILUnitF64 getValue() const;
        LILString stringRep();

    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        LILUnitF64 _value;
    };
}

#endif
