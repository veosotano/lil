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
 *      This file is the selector in a selector chain
 *
 ********************************************************************/

#ifndef LILSELECTOR_H
#define LILSELECTOR_H

#include "LILNode.h"

namespace LIL
{
    class LILSelector : public LILNode
    {
    public:
        LILSelector();
        LILSelector(const LILSelector &other);
        std::shared_ptr<LILSelector> clone() const;
        virtual ~LILSelector();

        void receiveNodeData(const LILString & data) override;

        LILString stringRep() override;
        bool equalTo(std::shared_ptr<LILNode> otherNode) override;
        
        virtual SelectorType getSelectorType() const override;
        bool isA(SelectorType otherType) const override;
        virtual void setSelectorType(SelectorType newType);
        void setName(LILString newName);
        LILString getName() const;
        
    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        SelectorType _selectorType;
        LILString _name;
    };
}

#endif
