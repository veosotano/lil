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
 *      This file adds intermediate parts of paths that come from
 *      expanded fields of classes
 *
 ********************************************************************/

#ifndef LILENUMLOWERER_H
#define LILENUMLOWERER_H

#include "LILVisitor.h"

namespace LIL
{
    class LILEnum;

    class LILEnumLowerer : public LILVisitor
    {
    public:
        LILEnumLowerer();
        virtual ~LILEnumLowerer();
        void initializeVisit() override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void process(LILNode * node);
        
    private:
        void _process(LILEnum * enm);
    };
}

#endif /* LILENUMLOWERER_H */
