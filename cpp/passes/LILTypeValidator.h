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
#include "LILFunctionType.h"
#include "LILObjectDefinition.h"
#include "LILRootNode.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

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
        void _validateFCArguments(std::shared_ptr<LILFunctionType> fnTy, std::shared_ptr<LILFunctionCall> fc, bool isMethod, std::shared_ptr<LILValuePath> vp);
        bool _isDefinitionOf(std::shared_ptr<LILType> nativeTy, std::shared_ptr<LILType> customTy);
        void _validate(std::shared_ptr<LILObjectDefinition> od);
        bool _validateField(std::shared_ptr<LILType> vdTy, std::shared_ptr<LILType> asTy);
        void _validate(std::shared_ptr<LILVarDecl> vd);
        void _validate(std::shared_ptr<LILVarName> vn);
        inline void validateChildren(const std::vector<std::shared_ptr<LILNode>> & children);
    };
}

#endif /* LILTYPEVALIDATOR_H */
