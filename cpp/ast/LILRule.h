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
 *      This file represents a rule, which has selector chains and properties
 *
 ********************************************************************/

#ifndef LILRULE_H
#define LILRULE_H

#include "LILNode.h"

namespace LIL
{
    class LILRule : public LILNode
    {
    public:
        LILRule();
        LILRule(const LILRule &other);
        std::shared_ptr<LILRule> clone() const;
        virtual ~LILRule();

        void addSelectorChain(std::shared_ptr<LILNode> newSc);
        void addValue(std::shared_ptr<LILNode> newVal);
        void addChildRule(std::shared_ptr<LILRule> rule);
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        std::vector<std::shared_ptr<LILNode>> _selectorChains;
        std::vector<std::shared_ptr<LILNode>> _values;
        std::vector<std::shared_ptr<LILRule>> _childRules;
    };
}

#endif
