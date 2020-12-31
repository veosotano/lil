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
 *      This file represents a bool written in the source code
 *
 ********************************************************************/

#ifndef LILBOOLLITERAL_H
#define LILBOOLLITERAL_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILBoolLiteral : public LILTypedNode
    {
    public:
        LILBoolLiteral();
        LILBoolLiteral(const LILBoolLiteral & other);
        std::shared_ptr<LILBoolLiteral> clone() const;
        virtual ~LILBoolLiteral();
        void receiveNodeData(const LIL::LILString &data);
        void setValue(bool newValue);
        void setOriginalStringRep(LILString string);
        LILString originalStringRep();
        bool getValue();
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        
    protected:
        bool _value;
        
    private:
        LILString _originalRep;
        
    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
    };
}

#endif
