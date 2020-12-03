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

#ifndef LILEXPRESSION_H
#define LILEXPRESSION_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILExpression : public LILTypedNode
    {
    public:
        static LILString expressionTypeToString(ExpressionType type);
        LILExpression();
        LILExpression(const LILExpression & other);
        std::shared_ptr<LILExpression> clone() const;
        virtual ~LILExpression();
        LILString stringRep() override;

        bool isA(ExpressionType otherType) const override;
        ExpressionType getExpressionType() const override;
        void setExpressionType(ExpressionType newType);

        void setLeft(std::shared_ptr<LILNode> newLeft);
        std::shared_ptr<LILNode> getLeft();
        void setRight(std::shared_ptr<LILNode> newRight);
        std::shared_ptr<LILNode> getRight();
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
        std::shared_ptr<LILNode> _leftNode;
        std::shared_ptr<LILNode> _rightNode;

    private:
        ExpressionType expressionType;
    };
}

#endif
