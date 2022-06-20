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

#ifndef LILFORLOWERER_H
#define LILFORLOWERER_H

#include "LILVisitor.h"

namespace LIL
{
    class LILEnum;

class LILForLowerer : public LILVisitor
    {
    public:
        LILForLowerer();
        virtual ~LILForLowerer();
        void initializeVisit() override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void process(LILNode * node);
        void processForBlocks(LILNode * node);
        
    private:
        void _process(LILEnum * enm);
        void _createForArgsNumber(LILFlowControl * fc, LILNode * arg) const;
        void _createForArgsObject(LILFlowControl * fc, LILNode * arg, LILType * ty) const;
    };
}

#endif /* LILFORLOWERER_H */
