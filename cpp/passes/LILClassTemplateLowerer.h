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
 *      This file makes concrete classes when using class parameters
 *
 ********************************************************************/

#ifndef LILCLASSTEMPLATELOWERER_H
#define LILCLASSTEMPLATELOWERER_H

#include "LILVisitor.h"
#include "LILRootNode.h"


namespace LIL
{
    class LILClassTemplateLowerer : public LILVisitor
    {
    public:
        LILClassTemplateLowerer();
        virtual ~LILClassTemplateLowerer();
        void initializeVisit() override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        std::vector<std::shared_ptr<LILNode>> findClassSpecializations(const std::vector<std::shared_ptr<LILNode>> & nodes) const;
        std::shared_ptr<LILClassDecl> makeSpecializedClass(std::shared_ptr<LILClassDecl> cd, std::shared_ptr<LILType> specializedType) const;
        void replaceTypeWithSpecializedType(const std::vector<std::shared_ptr<LILNode>> & nodes, std::shared_ptr<LILType> templateType, std::shared_ptr<LILType> specializedType) const;
    };
}

#endif
