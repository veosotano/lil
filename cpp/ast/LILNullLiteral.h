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
 *      This file represents a null value in the source code
 *
 ********************************************************************/

#ifndef LILNULLLITERAL_H
#define LILNULLLITERAL_H

#include "LILNode.h"

namespace LIL
{
    class LILNullLiteral : public LILNode
    {
    public:
        LILNullLiteral();
        LILNullLiteral(const LILNullLiteral & other);
        std::shared_ptr<LILNullLiteral> clone() const;
        virtual ~LILNullLiteral();
        void receiveNodeData(const LIL::LILString &data);
        LILString stringRep();
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        
    private:

        
    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
    };
}

#endif
