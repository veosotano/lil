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
#include "LILFunctionCall.h"
#include "LILObjectDefinition.h"
#include "LILRootNode.h"
#include "LILVarDecl.h"

namespace LIL
{
    class LILTypeValidator : public LILVisitor
    {
    public:
        LILTypeValidator();
        virtual ~LILTypeValidator();
        void initializeVisit() override;
        void visit(LILNode * node) override { };
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        
        void validate(std::shared_ptr<LILNode> node);
        void _validate(std::shared_ptr<LILFunctionCall> fc);
        bool _isDefinitionOf(std::shared_ptr<LILType> nativeTy, std::shared_ptr<LILType> customTy);
        void _validate(std::shared_ptr<LILObjectDefinition> od);
        void _validate(std::shared_ptr<LILVarDecl> vd);
        inline void validateChildren(const std::vector<std::shared_ptr<LILNode>> & children);
        bool typesCompatible(const std::shared_ptr<LILType> & ty1, const std::shared_ptr<LILType> & ty2);
    };
}

#endif /* LILTYPEVALIDATOR_H */
