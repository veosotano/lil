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
 *      This file inserts calls to type conversion functions
 *
 ********************************************************************/

#ifndef LILCONVERSIONINSERTER_H
#define LILCONVERSIONINSERTER_H

#include "LILVisitor.h"
#include "LILFunctionCall.h"


namespace LIL
{
    class LILConversionInserter : public LILVisitor
    {
    public:
        
        LILConversionInserter();
        virtual ~LILConversionInserter();
        
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;

        void process(std::shared_ptr<LILNode> node);
        void process(std::shared_ptr<LILFunctionCall> fc);
        inline void processChildren(const std::vector<std::shared_ptr<LILNode>> & children);
    };
}

#endif
