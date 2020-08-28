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
 *      This file represents an assignment
 *
 ********************************************************************/

#ifndef LILASSIGNMENT_H
#define LILASSIGNMENT_H

#include "LILNode.h"

namespace LIL
{
    class LILAssignment : public LILNode
    {
    public:
        LILAssignment();
        LILAssignment(const LILAssignment &other);
        std::shared_ptr<LILAssignment> clone() const;
        virtual ~LILAssignment();

        void setSubject(std::shared_ptr<LILNode> pp);
        std::shared_ptr<LILNode> getSubject() const;
        void setValue(std::shared_ptr<LILNode> val);
        std::shared_ptr<LILNode> getValue() const;
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::shared_ptr<LILNode> _subject;
        std::shared_ptr<LILNode> _value;
    };
}

#endif
