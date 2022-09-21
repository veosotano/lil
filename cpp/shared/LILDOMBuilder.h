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
 *      This file analyzes the code to determine types automatically
 *
 ********************************************************************/

#ifndef LILDOMBUILDER_H
#define LILDOMBUILDER_H

#include "LILVisitor.h"
#include "LILErrorMessage.h"
#include "../ast/LILNode.h"
#include "../ast/LILType.h"

namespace LIL
{
    class LILElement {
    public:
        std::shared_ptr<LILType> ty;
        LILString name;
        std::vector<std::shared_ptr<LILElement>> children;
        long int id;
        
        const std::shared_ptr<LILElement> & add(LILString name, std::shared_ptr<LILType> ty, long int id);
        void remove(std::shared_ptr<LILElement> elem);
        const std::shared_ptr<LILElement> & at(size_t index) const;
        const std::vector<std::shared_ptr<LILElement>> & getChildren() const;
    };

    class LILDOMBuilder : public LILVisitor
    {
    public:
        
        LILDOMBuilder();
        virtual ~LILDOMBuilder();

        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void createDOM();
        void recursiveAddElement(LILRule * rule);
        const std::shared_ptr<LILElement> & getDOM() const;
    private:
        std::shared_ptr<LILElement> dom;
        LILElement * insertionPoint;
        long int elementCount;
    };
}

#endif
