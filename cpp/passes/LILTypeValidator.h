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
 *      This file checks if function calls match their prototypes
 *
 ********************************************************************/

#ifndef LILTYPEVALIDATOR_H
#define LILTYPEVALIDATOR_H


#include "LILVisitor.h"
#include "LILNode.h"
#include "LILRootNode.h"


namespace LIL
{
    class LILValuePath;
    class LILTypeValidator : public LILVisitor
    {
    public:
        LILTypeValidator();
        virtual ~LILTypeValidator();
        void initializeVisit() override;
        void visit(LILNode * node) override { };
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        
        void validate(std::shared_ptr<LILNode>);
        inline void validateChildren(const std::vector<std::shared_ptr<LILNode>> & children);
        
    private:
        std::shared_ptr<LILNode> _findNodeForValuePath(std::shared_ptr<LILValuePath> vp) const;
    };
}

#endif /* LILTYPEVALIDATOR_H */
