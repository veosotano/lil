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
 *      This file inserts default methods into classes
 *
 ********************************************************************/

#ifndef LILMETHODINSERTER_H
#define LILMETHODINSERTER_H

#include "LILVisitor.h"
#include "LILNode.h"

#include "LILClassDecl.h"


namespace LIL
{
    class LILMethodInserter : public LILVisitor
    {
    public:
        LILMethodInserter();
        virtual ~LILMethodInserter();
        void initializeVisit();
        virtual void visit(LILNode * node);
        
        void process(LILNode * node);
        
        void setDebug(bool value);
        
    private:
        bool _debug;

        std::shared_ptr<LILNode> _findMethod(bool getter, LILClassDecl * value, LILString name);
        std::vector<std::shared_ptr<LILNode>> _findReturnStatements(const std::vector<std::shared_ptr<LILNode>> & body);
        std::vector<std::shared_ptr<LILNode>> _findSetterStatements(LILString name, const std::vector<std::shared_ptr<LILNode>> & body);
    };
}

#endif
