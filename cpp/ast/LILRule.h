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

        const std::shared_ptr<LILNode> & getSelectorChain() const;
        void setSelectorChain(std::shared_ptr<LILNode> value);
        void addValue(std::shared_ptr<LILNode> newVal);
        const std::vector<std::shared_ptr<LILNode>> & getValues() const;
        void setValues(std::vector<std::shared_ptr<LILNode>> && nodes);
        void addChildRule(std::shared_ptr<LILRule> rule);
        const std::vector<std::shared_ptr<LILRule>> & getChildRules() const;
        void setChildRules(std::vector<std::shared_ptr<LILRule>> && nodes);
        void setInstruction(std::shared_ptr<LILNode> instruction);
        const std::shared_ptr<LILNode> getInstruction() const;
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        std::shared_ptr<LILNode> getFirstSelector() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        std::shared_ptr<LILNode> _selectorChain;
        std::vector<std::shared_ptr<LILNode>> _values;
        std::vector<std::shared_ptr<LILRule>> _childRules;
        std::shared_ptr<LILNode> _instruction;
    };
}

#endif
