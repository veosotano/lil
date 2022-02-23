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
 *      This file converts color instructions into object definitions
 *
 ********************************************************************/

#ifndef LILCOLORMAKER_H
#define LILCOLORMAKER_H

#include "LILVisitor.h"

namespace LIL
{
    class LILAssignment;
    class LILConversionDecl;
    class LILEnum;
    class LILExpression;
    class LILFlowControl;
    class LILFlowControlCall;
    class LILFunctionCall;
    class LILFunctionDecl;
    class LILInstruction;
    class LILObjectDefinition;
    class LILRootNode;
    class LILUnaryExpression;
    class LILValueList;
    class LILVarDecl;

    class LILColorMaker : public LILVisitor
    {
    public:
        LILColorMaker();
        virtual ~LILColorMaker();
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        bool processColorInstr(std::shared_ptr<LILNode> node);
        
    private:
        std::vector<std::vector<std::shared_ptr<LILNode>>> _nodeBuffer;
        
        bool _processColorInstr(std::shared_ptr<LILExpression> value);
        bool _processColorInstr(std::shared_ptr<LILUnaryExpression> value);
        bool _processColorInstr(std::shared_ptr<LILVarDecl> value);
        bool _processColorInstr(std::shared_ptr<LILObjectDefinition> value);
        bool _processColorInstr(std::shared_ptr<LILAssignment> value);
        bool _processColorInstr(std::shared_ptr<LILFunctionDecl> value);
        bool _processColorInstr(std::shared_ptr<LILFunctionCall> value);
        bool _processColorInstr(std::shared_ptr<LILFlowControl> value);
        bool _processColorInstr(std::shared_ptr<LILFlowControlCall> value);
        bool _processColorInstr(std::shared_ptr<LILInstruction> value);
        bool _processColorInstr(std::shared_ptr<LILValueList> value);
        bool _processColorInstr(std::shared_ptr<LILConversionDecl> value);
        bool _processColorInstr(std::shared_ptr<LILEnum> value);
    };
}

#endif /* LILCOLORMAKER_H */
