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

        void addProperty(std::shared_ptr<LILNode> propDef);
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        std::vector<std::shared_ptr<LILNode>> _propDefs;
    };
}

#endif
