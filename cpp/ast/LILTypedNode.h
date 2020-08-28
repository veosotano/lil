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
 *      This is a node that participates in local variables
 *
 ********************************************************************/

#ifndef LILTYPEDNODE_H
#define LILTYPEDNODE_H

#include "LILNode.h"
#include "LILType.h"

namespace LIL {
    
    class LILTypedNode : public LILNode
    {
    public:
        virtual ~LILTypedNode();
        bool isTypedNode() const;
        virtual bool equalTo(std::shared_ptr<LILNode> otherNode);

        void setType(std::shared_ptr<LILType> value);
        std::shared_ptr<LILType> getType() const;
        
    protected:
        std::shared_ptr<LILType> _type;
        LILTypedNode(NodeType type);
        LILTypedNode(const LILTypedNode & other);
    };
}

#endif
