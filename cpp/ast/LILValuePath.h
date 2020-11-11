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
 *      This file represents a value path
 *
 ********************************************************************/

#ifndef LILVALUEPATH_H
#define LILVALUEPATH_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILValuePath : public LILTypedNode
    {
    public:
        LILValuePath();
        LILValuePath(const LILValuePath &other);
        std::shared_ptr<LILValuePath> clone() const;
        virtual ~LILValuePath();
        virtual LILString stringRep();
        
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        void addChild(std::shared_ptr<LILNode> child);
        
        void setPreventEmitCallToIVar(bool newValue);
        bool getPreventEmitCallToIVar() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;

    private:
        bool _preventEmitCallToIVar;
    };
}

#endif
