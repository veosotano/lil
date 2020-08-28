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

#ifndef LILNUMBERLITERAL_H
#define LILNUMBERLITERAL_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILNumberLiteral : public LILTypedNode
    {
    public:
        LILNumberLiteral();
        LILNumberLiteral(const LILNumberLiteral & other);
        std::shared_ptr<LILNumberLiteral> clone() const;
        virtual ~LILNumberLiteral();
        virtual LILString stringRep();
        void receiveNodeData(const LIL::LILString &data);
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        
        LILString getValue() const;
        void setValue(LILString newValue);

    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        LILString _value;
    };
}

#endif
