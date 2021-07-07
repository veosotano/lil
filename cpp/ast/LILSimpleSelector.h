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
 *      This file represents a simple selector, which groups individual
 *      selectors as a basic group
 *
 ********************************************************************/

#ifndef LILSIMPLESELECTOR_H
#define LILSIMPLESELECTOR_H

#include "LILNode.h"

namespace LIL
{
    class LILSimpleSelector : public LILNode
    {
    public:
        LILSimpleSelector();
        LILSimpleSelector(const LILSimpleSelector &other);
        std::shared_ptr<LILSimpleSelector> clone() const;
        virtual ~LILSimpleSelector();
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
    };
}

#endif
