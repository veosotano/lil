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
 *      This file implements object definitions
 *
 ********************************************************************/


#ifndef LILOBJECTDEFINITION_H
#define LILOBJECTDEFINITION_H


#include "LILTypedNode.h"

namespace LIL
{
    class LILObjectDefinition : public LILTypedNode
    {
    public:
        LILObjectDefinition();
        LILObjectDefinition(const LILObjectDefinition &other);
        std::shared_ptr<LILObjectDefinition> clone() const;
        virtual ~LILObjectDefinition();
        void receiveNodeData(const LIL::LILString &data) override;

        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        void setNodes(const std::vector<std::shared_ptr<LILNode>> && nodes);
        void addChild(std::shared_ptr<LILNode> child);

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const override;

    };
}

#endif
