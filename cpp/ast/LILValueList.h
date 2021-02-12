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
 *      This file holds multiple values separated by commas
 *
 ********************************************************************/

#ifndef LILVALUELIST_H
#define LILVALUELIST_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILValueList : public LILTypedNode
    {
    public:
        LILValueList();
        LILValueList(const LILValueList &other);
        std::shared_ptr<LILValueList> clone() const;
        virtual ~LILValueList();
        
        void receiveNodeData(const LILString & data) override;

        void addValue(std::shared_ptr<LILNode> val);
        void setValues(std::vector<std::shared_ptr<LILNode>> vals);
        std::vector<std::shared_ptr<LILNode>> getValues() const;
        void clearValues();

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;

    };
}

#endif
