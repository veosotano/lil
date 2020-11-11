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
 *      This file encapsulates an expression
 *
 ********************************************************************/

#ifndef LILUNARYEXPRESSION_H
#define LILUNARYEXPRESSION_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILUnaryExpression : public LILTypedNode
    {
    public:
        static LILString expressionTypeToString(UnaryExpressionType type);
        static ExpressionType uexpToExpType(UnaryExpressionType type);
        LILUnaryExpression();
        LILUnaryExpression(const LILUnaryExpression & other);
        std::shared_ptr<LILUnaryExpression> clone() const;
        virtual ~LILUnaryExpression();
        LILString stringRep() override;

        bool isA(UnaryExpressionType otherType) const;
        UnaryExpressionType getUnaryExpressionType() const;
        void setUnaryExpressionType(UnaryExpressionType newType);

        void setValue(std::shared_ptr<LILNode> newValue);
        std::shared_ptr<LILNode> getValue();

        void setSubject(std::shared_ptr<LILNode> newSubject);
        std::shared_ptr<LILNode> getSubject();
        void receiveNodeData(const LILString &data) override;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const override;

    private:
        UnaryExpressionType _uexpType;
        std::shared_ptr<LILNode> _value;
        std::shared_ptr<LILNode> _subject;
    };
}

#endif
