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
 *      This file replaces custom types with the types they resolve to
 *
 ********************************************************************/

#ifndef LILTYPERESOLVER_H
#define LILTYPERESOLVER_H

#include "LILVisitor.h"

namespace LIL
{
    class LILTypeResolver : public LILVisitor
    {
    public:
        
        LILTypeResolver();
        virtual ~LILTypeResolver();
        
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;

        inline void processChildren(const std::vector<std::shared_ptr<LILNode>> & children);
        void process(std::shared_ptr<LILNode> node);
        std::shared_ptr<LILType> _process(std::shared_ptr<LILType> value);
    };
}

#endif
