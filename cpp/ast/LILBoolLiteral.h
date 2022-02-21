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
        void receiveNodeData(const LIL::LILString &data) override;
        void setValue(bool newValue);
        bool getValue();
        bool equalTo(std::shared_ptr<LILNode> otherNode) override;
        std::shared_ptr<LILType> getType() const override;
        
    protected:
        bool _value;
        
    private:
        LILString _originalRep;
        
    private:
        std::shared_ptr<LILClonable> cloneImpl() const override;
    };
}

#endif
