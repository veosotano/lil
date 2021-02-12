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


#ifndef LILDOCUMENTATION_H
#define LILDOCUMENTATION_H


#include "LILNode.h"

namespace LIL
{
    class LILDocumentation : public LILNode
    {
    public:
        LILDocumentation();
        LILDocumentation(const LILDocumentation &other);
        std::shared_ptr<LILDocumentation> clone() const;
        virtual ~LILDocumentation();
        void receiveNodeData(const LIL::LILString &data) override;

        void add(const std::shared_ptr<LILNode> & node);
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        void setNodes(const std::vector<std::shared_ptr<LILNode>> && nodes);
        
        const LILString & getContent() const;
        void setContent(const LILString & value);

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const override;
    private:
        LILString content;

    };
}

#endif
