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

#ifndef LILSTATEMENT_H
#define LILSTATEMENT_H

#include "LILNode.h"

namespace LIL
{
    class LILValuePath : public LILNode
    {
    public:
        LILValuePath();
        LILValuePath(const LILValuePath &other);
        std::shared_ptr<LILValuePath> clone() const;
        virtual ~LILValuePath();
        virtual LILString stringRep();
        
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        void addChild(std::shared_ptr<LILNode> child);

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;

    private:

    };
}

#endif
