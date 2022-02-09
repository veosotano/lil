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
 *      This file instantiates nodes to make an abstract syntax tree
 *
 ********************************************************************/

#include "LILASTBuilder.h"
#include "LILNode.h"
#include "LILAliasDecl.h"
#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCombinator.h"
#include "LILConversionDecl.h"
#include "LILDocumentation.h"
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFunctionType.h"
#include "LILFunctionDecl.h"
#include "LILFunctionCall.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILForeignLang.h"
#include "LILIfInstruction.h"
#include "LILIndexAccessor.h"
#include "LILInstruction.h"
#include "LILMultipleType.h"
#include "LILNumberLiteral.h"
#include "LILNullLiteral.h"
#include "LILObjectDefinition.h"
#include "LILObjectType.h"
#include "LILPercentageLiteral.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILRootNode.h"
#include "LILRule.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILSnippetInstruction.h"
#include "LILStaticArrayType.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILType.h"
#include "LILTypeDecl.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

using namespace LIL;

LILASTBuilder::LILASTBuilder()
: _verbose(false)
, _buildFlatList(false)
, _debugAST(false)
{
    this->rootNode = std::make_shared<LILRootNode>();
    this->state.push_back(BuilderStateRoot);
}

LILASTBuilder::~LILASTBuilder()
{

}

void LILASTBuilder::reset()
{
    currentNode.reset();
    rootNode.reset();
    state.clear();
    this->errors.clear();

    this->rootNode = std::make_shared<LILRootNode>();
    this->state.push_back(BuilderStateRoot);
}

void LILASTBuilder::receiveNodeStart(NodeType nodeType)
{
    if (this->_debugAST) {
        std::cerr << "==node start== " + LILNode::nodeTypeToString(nodeType).data() + "\n";
    }

    switch (nodeType)
    {
        case NodeTypeNumberLiteral:
        {
            std::shared_ptr<LILNumberLiteral> num = std::make_shared<LILNumberLiteral>();
            this->state.push_back(BuilderStateNumber);
            this->currentContainer.push_back(num);
            break;
        }
        case NodeTypePercentage:
        {
            std::shared_ptr<LILPercentageLiteral> perc = std::make_shared<LILPercentageLiteral>();
            this->currentNode = perc;
            this->state.push_back(BuilderStatePercentage);
            break;
        }
        case NodeTypeStringLiteral:
        case NodeTypeCStringLiteral:
        {
            std::shared_ptr<LILStringLiteral> str = std::make_shared<LILStringLiteral>();
            if (nodeType == NodeTypeCStringLiteral) {
                str->setIsCString(true);
            }
            this->currentNode = str;
            this->state.push_back(BuilderStateString);
            break;
        }
        case NodeTypeStringFunction:
        {
            this->state.push_back(BuilderStateStringFunction);
            std::shared_ptr<LILStringFunction> str = std::make_shared<LILStringFunction>();
            this->currentContainer.push_back(str);
            break;
        }
        case NodeTypeExpression:
        {
            this->state.push_back(BuilderStateExpression);
            if (this->currentNode)
            {
                std::shared_ptr<LILExpression> exp = std::make_shared<LILExpression>();
                exp->setLeft(this->currentNode);
                this->currentContainer.push_back(exp);
                this->currentNode.reset();
            }
            break;
        }
        case NodeTypeUnaryExpression:
        {
            this->state.push_back(BuilderStateUnaryExpression);
            if (this->currentNode)
            {
                std::shared_ptr<LILUnaryExpression> exp = std::make_shared<LILUnaryExpression>();
                exp->setSubject(this->currentNode);
                this->currentContainer.push_back(exp);
                this->currentNode.reset();
            }
            break;
        }
        case NodeTypeType:
        {
            this->state.push_back(BuilderStateType);
            this->currentContainer.push_back(std::make_shared<LILType>());
            break;
        }
        case NodeTypeMultipleType:
        {
            this->state.push_back(BuilderStateMultipleType);
            this->currentContainer.push_back(std::make_shared<LILMultipleType>());
            break;
        }
        case NodeTypeFunctionType:
        {
            this->state.push_back(BuilderStateFunctionType);
            this->currentContainer.push_back(std::make_shared<LILFunctionType>());
            break;
        }
        case NodeTypePointerType:
        {
            this->state.push_back(BuilderStatePointerType);
            this->currentContainer.push_back(std::make_shared<LILPointerType>());
            break;
        }
        case NodeTypeStaticArrayType:
        {
            this->state.push_back(BuilderStateStaticArrayType);
            this->currentContainer.push_back(std::make_shared<LILStaticArrayType>());
            break;
        }
        case NodeTypeObjectType:
        {
            this->state.push_back(BuilderStateObjectType);
            this->currentContainer.push_back(std::make_shared<LILObjectType>());
            break;
        }
        case NodeTypeVarDecl:
        {
            this->state.push_back(BuilderStateVarDecl);
            this->currentContainer.push_back(std::make_shared<LILVarDecl>());
            break;
        }
        case NodeTypeConstDecl:
        {
            this->state.push_back(BuilderStateConstDecl);
            auto vd = std::make_shared<LILVarDecl>();
            vd->setIsConst(true);
            this->currentContainer.push_back(vd);
            break;
        }
        case NodeTypeAliasDecl:
        {
            this->state.push_back(BuilderStateAliasDecl);
            this->currentContainer.push_back(std::make_shared<LILAliasDecl>());
            break;
        }
        case NodeTypeTypeDecl:
        {
            this->state.push_back(BuilderStateTypeDecl);
            this->currentContainer.push_back(std::make_shared<LILTypeDecl>());
            break;
        }
        case NodeTypeConversionDecl:
        {
            this->state.push_back(BuilderStateConversionDecl);
            this->currentContainer.push_back(std::make_shared<LILConversionDecl>());
            break;
        }
        case NodeTypeVarName:
        {
            this->state.push_back(BuilderStateVarName);
            this->currentNode = std::make_shared<LILVarName>();
            break;
        }

        case NodeTypeRule:
        {
            this->state.push_back(BuilderStateRule);
            this->currentContainer.push_back(std::make_shared<LILRule>());
            break;
        }
        case NodeTypeSelectorChain:
        {
            this->state.push_back(BuilderStateSelectorChain);
            this->currentContainer.push_back(std::make_shared<LILSelectorChain>());
            break;
        }
        case NodeTypeSimpleSelector:
        {
            this->state.push_back(BuilderStateSimpleSelector);
            this->currentContainer.push_back(std::make_shared<LILSimpleSelector>());
            break;
        }
        case NodeTypeSelector:
        {
            this->state.push_back(BuilderStateSelector);
            this->currentNode = std::make_shared<LILSelector>();
            break;
        }
        case NodeTypeCombinator:
        {
            this->state.push_back(BuilderStateCombinator);
            this->currentNode = std::make_shared<LILCombinator>();
            break;
        }
        case NodeTypeValuePath:
        {
            this->state.push_back(BuilderStateValuePath);
            std::shared_ptr<LILValuePath> vp = std::make_shared<LILValuePath>();
            if (this->currentNode)
            {
                vp->addChild(this->currentNode);
                this->currentNode.reset();
            }
            this->currentContainer.push_back(vp);
            break;
        }
        case NodeTypePropertyName:
        {
            this->state.push_back(BuilderStatePropertyName);
            this->currentNode = std::make_shared<LILPropertyName>();
            break;
        }
        case NodeTypeAssignment:
        {
            this->state.push_back(BuilderStateAssignment);
            this->currentContainer.push_back(std::make_shared<LILAssignment>());
            break;
        }
        case NodeTypeClassDecl:
        {
            this->state.push_back(BuilderStateClassDecl);
            this->currentContainer.push_back(std::make_shared<LILClassDecl>());
            break;
        }
        case NodeTypeObjectDefinition:
        {
            this->state.push_back(BuilderStateObjectDefinition);
            this->currentContainer.push_back(std::make_shared<LILObjectDefinition>());
            break;
        }
        case NodeTypeFunctionDecl:
        {
            this->state.push_back(BuilderStateFunctionDecl);
            this->currentContainer.push_back(std::make_shared<LILFunctionDecl>());
            break;
        }
        case NodeTypeFunctionCall:
        {
            this->state.push_back(BuilderStateFunctionCall);
            std::shared_ptr<LILFunctionCall> fc = std::make_shared<LILFunctionCall>();
            if (this->currentNode)
            {
                if (this->currentNode->isA(NodeTypePropertyName)) {
                    auto pn = std::static_pointer_cast<LILPropertyName>(this->currentNode);
                    fc->setName(pn->getName());
                } else if (this->currentNode->isA(NodeTypeVarName)) {
                    auto vn = std::static_pointer_cast<LILVarName>(this->currentNode);
                    fc->setName(vn->getName());
                }
                this->currentNode.reset();
            }
            this->currentContainer.push_back(fc);
            break;
        }
        case NodeTypeFlowControl:
        {
            this->state.push_back(BuilderStateFlowControl);
            this->currentContainer.push_back(std::make_shared<LILFlowControl>());
            break;
        }
        case NodeTypeFlowControlCall:
        {
            this->state.push_back(BuilderStateFlowControlCall);
            this->currentContainer.push_back(std::make_shared<LILFlowControlCall>());
            break;
        }
        case NodeTypeNull:
        {
            this->state.push_back(BuilderStateNull);
            this->currentNode = std::make_shared<LILNullLiteral>();
            break;
        }
        case NodeTypeBoolLiteral:
        {
            this->state.push_back(BuilderStateBool);
            this->currentNode = std::make_shared<LILBoolLiteral>();
            break;
        }
        case NodeTypeFilter:
        {
            this->state.push_back(BuilderStateFilter);
            this->currentNode = std::make_shared<LILFilter>();
            break;
        }
        case NodeTypeFlag:
        {
            this->state.push_back(BuilderStateFlag);
            this->currentNode = std::make_shared<LILFlag>();
            break;
        }
        case NodeTypeInstruction:
        {
            this->state.push_back(BuilderStateInstruction);
            this->currentContainer.push_back(std::make_shared<LILInstruction>());
            break;
        }
        case NodeTypeIfInstruction:
        {
            this->state.push_back(BuilderStateIfInstruction);
            this->currentContainer.push_back(std::make_shared<LILIfInstruction>());
            break;
        }
        case NodeTypeSnippetInstruction:
        {
            this->state.push_back(BuilderStateSnippetInstruction);
            this->currentContainer.push_back(std::make_shared<LILSnippetInstruction>());
            break;
        }
        case NodeTypeForeignLang:
        {
            this->state.push_back(BuilderStateForeignLang);
            this->currentNode = std::make_shared<LILForeignLang>();
            break;
        }
        case NodeTypeValueList:
        {
            this->state.push_back(BuilderStateValueList);
            std::shared_ptr<LILValueList> sat = std::make_shared<LILValueList>();
            this->currentContainer.push_back(sat);
            if (this->currentNode)
            {
                sat->addValue(this->currentNode);
                this->currentNode.reset();
            }
            break;
        }
        case NodeTypeIndexAccessor:
        {
            this->state.push_back(BuilderStateIndexAccessor);
            this->currentContainer.push_back(std::make_shared<LILIndexAccessor>());
            break;
        }
        case NodeTypeDocumentation:
        {
            this->state.push_back(BuilderStateDocumentation);
            this->currentContainer.push_back(std::make_shared<LILDocumentation>());
            break;
        }
        default:
            std::cerr << "Error: Unknown node type\n";
            break;
    }
}

void LILASTBuilder::receiveNodeEnd(NodeType nodeType)
{
    if (this->_debugAST) {
        std::cerr << "==node end== " + LILNode::nodeTypeToString(nodeType).data() + "\n";
    }
    if (LILNode::isContainerNode(nodeType))
    {
        this->currentNode = this->currentContainer.back();
        this->currentContainer.pop_back();
    }
    state.pop_back();
}

void LILASTBuilder::receiveNodeCommit()
{
    if (this->_debugAST) {
        std::cerr << "==node commit==\n";
    }
    switch (this->state.back())
    {
        case BuilderStateRoot:
        {
            if (this->currentNode)
            {
                if (this->getBuildFlatList()) {
                    this->rootNode->addNode(currentNode);
                } else {
                    this->rootNode->add(this->currentNode);
                }
            }
            break;
        }
        case BuilderStateNumber:
        {
            if (this->currentNode && this->currentNode->isA(NodeTypeType)) {
                auto num = std::static_pointer_cast<LILNumberLiteral>(this->currentContainer.back());
                num->setType(std::static_pointer_cast<LILType>(this->currentNode));
            }
            break;
        }
        case BuilderStateStringFunction:
        {
            std::shared_ptr<LILStringFunction> str = std::static_pointer_cast<LILStringFunction>(this->currentContainer.back());
            str->add(this->currentNode);
            break;
        }
        case BuilderStateExpression:
        {
            std::shared_ptr<LILExpression> exp = std::static_pointer_cast<LILExpression>(this->currentContainer.back());
            std::shared_ptr<LILNode> left = exp->getLeft();
            if (!left)
            {
                exp->setLeft(this->currentNode);
            }
            else
            {
                exp->setRight(this->currentNode);
            }
            break;
        }
        case BuilderStateUnaryExpression:
        {
            std::shared_ptr<LILUnaryExpression> uexp = std::static_pointer_cast<LILUnaryExpression>(this->currentContainer.back());
            uexp->setValue(this->currentNode);
            break;
        }
        case BuilderStateType:
        {
            std::cerr << "CANNOT ADD PARAMETER TYPE TO SINGLE TYPE FAIL !!!!! \n\n";
            break;
        }
        case BuilderStateObjectType:
        {
            if (this->currentNode) {
                std::shared_ptr<LILObjectType> objTy = std::static_pointer_cast<LILObjectType>(this->currentContainer.back());
                objTy->addTmplParam(this->currentNode);
            }
            break;
        }
        case BuilderStateMultipleType:
        {
            if (this->currentNode && this->currentNode->isA(NodeTypeType)) {
                std::shared_ptr<LILMultipleType> ty = std::static_pointer_cast<LILMultipleType>(this->currentContainer.back());
                ty->addType(std::static_pointer_cast<LILType>(this->currentNode));
            }
            break;
        }
        case BuilderStateFunctionType:
        {
            if (this->currentNode) {
                std::shared_ptr<LILFunctionType> ty = std::static_pointer_cast<LILFunctionType>(this->currentContainer.back());
                if (ty->getReceivesReturnType()) {
                    ty->setReturnType(std::static_pointer_cast<LILType>(this->currentNode));
                } else {
                    ty->addArgument(std::static_pointer_cast<LILType>(this->currentNode));
                }
            }
            break;
        }
        case BuilderStatePointerType:
        {
            if (this->currentNode) {
                std::shared_ptr<LILPointerType> ty = std::static_pointer_cast<LILPointerType>(this->currentContainer.back());
                ty->setArgument(std::static_pointer_cast<LILType>(this->currentNode));
            }
            break;
        }
        case BuilderStateStaticArrayType:
        {
            if (this->currentNode) {
                std::shared_ptr<LILStaticArrayType> ty = std::static_pointer_cast<LILStaticArrayType>(this->currentContainer.back());
                if (ty->getReceivesType()) {
                    if (this->currentNode->isA(NodeTypeType)) {
                        ty->setType(std::static_pointer_cast<LILType>(this->currentNode));
                    }
                } else {
                    ty->setArgument(this->currentNode);
                }
            }
            break;
        }
        case BuilderStateVarDecl:
        case BuilderStateConstDecl:
        {
            if (this->currentNode)
            {
                std::shared_ptr<LILVarDecl> varDecl = std::static_pointer_cast<LILVarDecl>(this->currentContainer.back());
                if (this->currentNode->isA(NodeTypeType)) {
                    if (varDecl->getReceivesReturnType()) {
                        varDecl->setReturnType(std::static_pointer_cast<LILType>(this->currentNode));
                    } else {
                        varDecl->setType(std::static_pointer_cast<LILType>(this->currentNode));
                    }
                } else {
                    varDecl->setInitVal(this->currentNode);
                    if (this->currentNode->isA(NodeTypeFunctionDecl)) {
                        auto name = varDecl->getName();
                        auto fd = std::static_pointer_cast<LILFunctionDecl>(this->currentNode);
                        fd->setName(name);
                        if (name == "construct") {
                            fd->setIsConstructor(true);
                        }
                        
                        auto ty = varDecl->getType();
                        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                        for ( auto arg : fnTy->getArguments()) {
                            if (arg->isA(NodeTypeVarDecl)) {
                                auto argVd = std::static_pointer_cast<LILVarDecl>(arg);
                                fd->setLocalVariable(argVd->getName(), argVd);
                            }
                        }
                    }
                }
            }
            break;
        }
        case BuilderStateAliasDecl:
        {
            if (this->currentNode) {
                auto ad = std::static_pointer_cast<LILAliasDecl>(this->currentContainer.back());
                if (this->currentNode->isA(NodeTypeType)) {
                    auto ty = std::static_pointer_cast<LILType>(this->currentNode);
                    if (!ad->getSrcType()) {
                        ad->setSrcType(ty);
                    } else {
                        ad->setDstType(ty);
                    }
                } else {
                    std::cerr << "Error: Unknown node type for alias declaration.\n";
                    break;
                }
            }
            break;
        }
        case BuilderStateTypeDecl:
        {
            if (this->currentNode) {
                auto td = std::static_pointer_cast<LILTypeDecl>(this->currentContainer.back());
                if (this->currentNode->isA(NodeTypeType)) {
                    auto ty = std::static_pointer_cast<LILType>(this->currentNode);
                    if (!td->getSrcType()) {
                        td->setSrcType(ty);
                    } else {
                        td->setDstType(ty);
                    }
                } else {
                    std::cerr << "Error: Unknown node type for type declaration.\n";
                    break;
                }
            }
            break;
        }
        case BuilderStateConversionDecl:
        {
            if (this->currentNode)
            {
                auto cd = std::static_pointer_cast<LILConversionDecl>(this->currentContainer.back());
                if (this->currentNode->isA(NodeTypeVarDecl)) {
                    cd->setVarDecl(std::static_pointer_cast<LILVarDecl>(this->currentNode));
                } else if (this->currentNode->isA(NodeTypeType)){
                    cd->setType(std::static_pointer_cast<LILType>(this->currentNode));
                } else {
                    cd->addEvaluable(this->currentNode);
                }
            }
            break;
        }
        case BuilderStateSimpleSelector:
        {
            std::shared_ptr<LILSimpleSelector> ss = std::static_pointer_cast<LILSimpleSelector>(this->currentContainer.back());
            ss->addNode(this->currentNode);
            break;
        }
        case BuilderStateSelectorChain:
        {
            std::shared_ptr<LILSelectorChain> selCh = std::static_pointer_cast<LILSelectorChain>(this->currentContainer.back());
            selCh->addNode(this->currentNode);
            break;
        }
        case BuilderStateValuePath:
        {
            std::shared_ptr<LILValuePath> vp = std::static_pointer_cast<LILValuePath>(this->currentContainer.back());
            if (this->currentNode->isA(NodeTypeFunctionCall)) {
                auto fc = std::static_pointer_cast<LILFunctionCall>(this->currentNode);
                fc->setFunctionCallType(FunctionCallTypeValuePath);
            }
            vp->addChild(this->currentNode);
            break;
        }
        case BuilderStateRule:
        {
            std::shared_ptr<LILRule> rule = std::static_pointer_cast<LILRule>(this->currentContainer.back());
            switch (this->currentNode->getNodeType()) {
                case NodeTypeSelectorChain:
                {
                    rule->setSelectorChain(this->currentNode);
                    break;
                }
                case NodeTypeAssignment:
                case NodeTypeUnaryExpression:
                {
                    rule->addValue(this->currentNode);
                    break;
                }
                case NodeTypeRule:
                {
                    rule->addChildRule(std::static_pointer_cast<LILRule>(this->currentNode));
                    break;
                }
                case NodeTypeInstruction:
                {
                    rule->setInstruction(this->currentNode);
                    break;
                }
                default:
                    std::cerr << "UNKONWN NODE TYPE TO ADD TO RULE FAIL !!!!!!\n\n";
                    break;
            }
            break;
        }
        case BuilderStateAssignment:
        {
            std::shared_ptr<LILAssignment> pd = std::static_pointer_cast<LILAssignment>(this->currentContainer.back());
            if (!pd->getSubject())
            {
                pd->setSubject(this->currentNode);
            }
            else
            {
                pd->setValue(this->currentNode);
            }
            break;
        }
        case BuilderStateClassDecl:
        {
            std::shared_ptr<LILClassDecl> cd = std::static_pointer_cast<LILClassDecl>(this->currentContainer.back());
            cd->add(this->currentNode);
            break;
        }
        case BuilderStateObjectDefinition:
        {
            std::shared_ptr<LILObjectDefinition> od = std::static_pointer_cast<LILObjectDefinition>(this->currentContainer.back());
            switch (this->currentNode->getNodeType())
            {
                case NodeTypeType:
                {
                    auto ty = od->getType();
                    if (ty && ty->isA(TypeTypeObject)) {
                        auto objTy = std::static_pointer_cast<LILObjectType>(ty);
                        objTy->addTmplParam(this->currentNode);
                    }
                    break;
                }
                default:
                    od->addChild(this->currentNode);
                    break;
            }
            break;
        }
        case BuilderStateFunctionDecl:
        {
            std::shared_ptr<LILFunctionDecl> fd = std::static_pointer_cast<LILFunctionDecl>(this->currentContainer.back());
            if (this->currentNode->isA(FlowControlTypeFinally)) {
                fd->setFinally(this->currentNode);
            } else if (this->currentNode->isA(NodeTypeType)) {
                fd->setType(std::static_pointer_cast<LILType>(this->currentNode));
            } else {
                fd->addEvaluable(this->currentNode);
            }
            break;
        }
        case BuilderStateFunctionCall:
        {
            std::shared_ptr<LILFunctionCall> fc = std::static_pointer_cast<LILFunctionCall>(this->currentContainer.back());
            fc->addArgument(this->currentNode);
            break;
        }
        case BuilderStateFlowControl:
        {
            std::shared_ptr<LILFlowControl> fc = std::static_pointer_cast<LILFlowControl>(this->currentContainer.back());
            if (fc->getReceivesFunctionBody()) {
                if (fc->getReceivesElse()) {
                    fc->addElse(this->currentNode);
                } else {
                    fc->addThen(this->currentNode);
                }
                if (this->currentNode->isA(NodeTypeVarDecl)) {
                    auto vd = std::static_pointer_cast<LILVarDecl>(this->currentNode);
                    fc->setLocalVariable(vd->getName(), vd);
                }
            } else {
                fc->addArgument(this->currentNode);
            }
            break;
        }
        case BuilderStateFlowControlCall:
        {
            std::shared_ptr<LILFlowControlCall> fcc = std::static_pointer_cast<LILFlowControlCall>(this->currentContainer.back());
            fcc->setArgument(this->currentNode);
            break;
        }
        case BuilderStateInstruction:
        {
            auto instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
            switch (instr->getInstructionType()) {
                case InstructionTypeNeeds:
                case InstructionTypeImport:
                case InstructionTypePaste:
                case InstructionTypeBug:
                    instr->setArgument(this->currentNode);
                    break;

                case InstructionTypeExport:
                {
                    this->currentNode->setIsExported(true);
                    instr->addNode(this->currentNode);
                    break;
                }
                case InstructionTypeConfigure:
                {
                    instr->addNode(this->currentNode);
                    break;
                }
                case InstructionTypeGetConfig:
                {
                    instr->setArgument(this->currentNode);
                    break;
                }
                case InstructionTypeArg:
                {
                    instr->addNode(this->currentNode);
                    break;
                }
                case InstructionTypeExpand:
                {
                    if (this->currentNode->isA(NodeTypeVarDecl)) {
                        auto vd = std::static_pointer_cast<LILVarDecl>(this->currentNode);
                        vd->setIsExpanded(true);
                    }
                    instr->addNode(this->currentNode);
                    break;
                }
                default:
                    std::cerr << "UNIMPLEMENTED FAIL !!!! \n\n";
                    break;
            }
            
            break;
        }

        case BuilderStateIfInstruction:
        {
            auto instr = std::static_pointer_cast<LILIfInstruction>(this->currentContainer.back());
            if (instr->getReceivesThen()) {
                instr->addThen(this->currentNode);
            } else if (instr->getReceivesElse()) {
                instr->addElse(this->currentNode);
            } else {
                instr->setArgument(this->currentNode);
            }
            break;
        }
        case BuilderStateSnippetInstruction:
        {
            auto snInstr = std::static_pointer_cast<LILSnippetInstruction>(this->currentContainer.back());
            if (snInstr->getReceivesBody()) {
                snInstr->add(this->currentNode);
            } else {
                snInstr->setArgument(this->currentNode);
            }
            break;
        }
        case BuilderStateValueList:
        {
            auto vl = std::static_pointer_cast<LILValueList>(this->currentContainer.back());
            vl->addValue(this->currentNode);
            break;
        }
        case BuilderStateIndexAccessor:
        {
            auto ia = std::static_pointer_cast<LILIndexAccessor>(this->currentContainer.back());
            ia->setArgument(this->currentNode);
            break;
        }
        case BuilderStateDocumentation:
        {
            auto doc = std::static_pointer_cast<LILDocumentation>(this->currentContainer.back());
            if (this->currentNode->isA(NodeTypeDocumentation)) {
                auto childDoc = std::static_pointer_cast<LILDocumentation>(this->currentNode);
                if (childDoc->getContent().length() > 0) {
                    doc->add(this->currentNode);
                }
            }
            break;
        }
        default:
            std::cerr << "Error: Unknown builder state\n";
            break;
    }
    this->currentNode.reset();
}

void LILASTBuilder::receiveNodeData(ParserEvent eventType, const LILString &data)
{
    if (this->_verbose) {
        std::cerr << data.data();
    }
    if (this->_debugAST) {
        std::cerr << "\n";
    }

    //ignore punctuation, comments and whitespace
    if (eventType == ParserEventPunctuation || eventType == ParserEventComment || eventType == ParserEventWhitespace)
    {
        return;
    }

    switch (this->state.back())
    {
        case BuilderStateBool:
        {
            auto bl = std::static_pointer_cast<LILBoolLiteral>(this->currentNode);
            std::shared_ptr<LILType> type = std::make_shared<LILType>();
            type->setName("bool");
            bl->setType(type);
            bl->receiveNodeData(data);
            break;
        }
        case BuilderStateNumber:
        {
            auto num = std::static_pointer_cast<LILNumberLiteral>(this->currentContainer.back());
            if (eventType != ParserEventNumberInt && eventType != ParserEventNumberFP) {
                num->receiveNodeData(data);
                break;
            }
            else
            {
                std::shared_ptr<LILType> parentTy;
                std::shared_ptr<LILType> type;
                if (this->currentContainer.size() > 1) {
                    auto cont = this->currentContainer[this->currentContainer.size()-2];
                    if (cont->isA(NodeTypeVarDecl)) {
                        parentTy = cont->getType();
                    }
                }
                
                if (parentTy)
                {
                    if (parentTy->isA(TypeTypeMultiple)) {
                        auto multiTy = std::static_pointer_cast<LILMultipleType>(parentTy);
                        bool intFound = false;
                        std::shared_ptr<LILType> intTy;
                        for (auto ty : multiTy->getTypes()) {
                            if (LILType::isNumberType(ty.get())) {
                                intTy = ty;
                                intFound = true;
                                break;
                            }
                        }
                        if (intFound) {
                            type = intTy;
                        } else {
                            for (auto ty : multiTy->getTypes()) {
                                if (LILType::isNumberType(ty.get())) {
                                    type = ty;
                                    break;
                                }
                            }
                        }
                    } else {
                        if (
                            LILType::isBuiltInType(parentTy.get())
                            && LILType::isNumberType(parentTy.get())
                            ) {
                            type = parentTy;
                        }
                    }
                }
                
                if (type) {
                    num->setType(type);
                }
                else if (eventType == ParserEventNumberInt)
                {
                    std::shared_ptr<LILMultipleType> weakType = std::make_shared<LILMultipleType>();
                    long long int numValue = num->getValue().toLongLong();
                    
                    std::shared_ptr<LILType> type1 = std::make_shared<LILType>();
                    if ((numValue > 2147483647) || (numValue < -2147483647)) {
                        type1->setName("i64");
                    } else if ((numValue > 32767) || (numValue < -32767)) {
                        type1->setName("i32");
                    } else if ((numValue > 127) || (numValue < -127)) {
                        type1->setName("i16");
                    } else {
                        type1->setName("i8");
                    }
                    weakType->addType(type1);

                    std::shared_ptr<LILType> type2 = std::make_shared<LILType>();
                    type2->setName("f64");
                    weakType->addType(type2);
                    std::shared_ptr<LILType> type3 = std::make_shared<LILType>();
                    type3->setName("f32");
                    weakType->addType(type3);
                    weakType->setIsWeakType(true);
                    num->setType(weakType);
                }
                else if (eventType == ParserEventNumberFP)
                {
                    auto ty = std::make_shared<LILType>();
                    ty->setName("f64");
                    num->setType(ty);
                }
            }
            break;
        }
        case BuilderStatePercentage:
        {
            if (this->currentNode)
            {
                if (eventType == ParserEventNumberInt)
                {
                    std::shared_ptr<LILType> ty;
                    if (this->currentContainer.size() > 0) {
                        auto cont = this->currentContainer.back();
                        if (cont->isA(NodeTypeVarDecl)) {
                            ty = cont->getType();
                        }
                    }
                    
                    if (ty)
                    {
                        if (ty->isA(TypeTypeMultiple)) {
                            auto multiTy = std::static_pointer_cast<LILMultipleType>(ty);
                            bool intFound = false;
                            for (auto ty : multiTy->getTypes()) {
                                auto name = ty->getName();
                                if (name == "i64%") {
                                    intFound = true;
                                    break;
                                }
                            }
                            if (intFound) {
                                std::shared_ptr<LILType> intTy = std::make_shared<LILType>();
                                intTy->setName("i64%");
                                std::static_pointer_cast<LILPercentageLiteral>(this->currentNode)->setType(intTy);
                            } else {
                                bool floatFound = false;
                                for (auto ty : multiTy->getTypes()) {
                                    auto name = ty->getName();
                                    if (name == "f64%") {
                                        floatFound = true;
                                        break;
                                    }
                                }
                                if (floatFound) {
                                    std::shared_ptr<LILType> floatTy = std::make_shared<LILType>();
                                    floatTy->setName("f64%");
                                    std::static_pointer_cast<LILPercentageLiteral>(this->currentNode)->setType(floatTy);
                                }
                            }
                        } else {
                            std::static_pointer_cast<LILPercentageLiteral>(this->currentNode)->setType(ty);
                        }
                    }
                    else
                    {
                        std::shared_ptr<LILMultipleType> type = std::make_shared<LILMultipleType>();
                        std::shared_ptr<LILType> type1 = std::make_shared<LILType>();
                        type1->setName("i64%");
                        type->addType(type1);
                        std::shared_ptr<LILType> type2 = std::make_shared<LILType>();
                        type2->setName("f64%");
                        type->addType(type2);
                        type->setIsWeakType(true);
                        std::static_pointer_cast<LILPercentageLiteral>(this->currentNode)->setType(type);
                    }
                }
                else if (eventType == ParserEventNumberFP)
                {
                    auto ty = std::make_shared<LILType>();
                    ty->setName("f64%");
                    std::static_pointer_cast<LILPercentageLiteral>(this->currentNode)->setType(ty);
                } else {
                    this->currentNode->receiveNodeData(data);
                }
            }
            else if (this->currentContainer.size() > 0)
            {
                this->currentContainer.back()->receiveNodeData(data);
            }
            break;
        }
        case BuilderStateStringFunction:
        {
            switch (eventType) {
                case ParserEventStringFunctionStart:
                {
                    std::shared_ptr<LILStringFunction> str = std::static_pointer_cast<LILStringFunction>(this->currentContainer.back());
                    str->setStartChunk(data);
                    break;
                }
                case ParserEventStringFunctionMid:
                {
                    std::shared_ptr<LILStringFunction> str = std::static_pointer_cast<LILStringFunction>(this->currentContainer.back());
                    str->addMidChunk(data);
                    break;
                }
                case ParserEventStringFunctionEnd:
                {
                    std::shared_ptr<LILStringFunction> str = std::static_pointer_cast<LILStringFunction>(this->currentContainer.back());
                    str->setEndChunk(data);
                    break;
                }
                case ParserEventStringFunctionArgEnd:
                {
                    
                    break;
                }
                    
                default:
                    break;
            }
            break;
        }
        case BuilderStateExpression:
        {
            if (eventType == ParserEventExpressionSign)
            {
                std::shared_ptr<LILExpression> exp = std::static_pointer_cast<LILExpression>(this->currentContainer.back());
                if (data == "+") {
                    exp->setExpressionType(ExpressionTypeSum);
                }
                else if (data == "-") {
                    exp->setExpressionType(ExpressionTypeSubtraction);
                }
                else if (data == "*") {
                    exp->setExpressionType(ExpressionTypeMultiplication);
                }
                else if (data == "/") {
                    exp->setExpressionType(ExpressionTypeDivision);
                }
                else if (data == "=") {
                    exp->setExpressionType(ExpressionTypeEqualComparison);
                }
                else if (data == "!=") {
                    exp->setExpressionType(ExpressionTypeNotEqualComparison);
                }
                else if (data == ">") {
                    exp->setExpressionType(ExpressionTypeBiggerComparison);
                }
                else if (data == ">=") {
                    exp->setExpressionType(ExpressionTypeBiggerOrEqualComparison);
                }
                else if (data == "<") {
                    exp->setExpressionType(ExpressionTypeSmallerComparison);
                }
                else if (data == "<=") {
                    exp->setExpressionType(ExpressionTypeSmallerOrEqualComparison);
                }
                else if (data == "AND") {
                    exp->setExpressionType(ExpressionTypeLogicalAnd);
                }
                else if (data == "OR") {
                    exp->setExpressionType(ExpressionTypeLogicalOr);
                }
                else if (data == "BIT_AND") {
                    exp->setExpressionType(ExpressionTypeBitwiseAnd);
                }
                else if (data == "BIT_OR") {
                    exp->setExpressionType(ExpressionTypeBitwiseOr);
                }
                else if (data == "XOR") {
                    exp->setExpressionType(ExpressionTypeXor);
                }
                else if (data == "SHIFT_LEFT") {
                    exp->setExpressionType(ExpressionTypeShiftLeft);
                }
                else if (data == "SHIFT_RIGHT") {
                    exp->setExpressionType(ExpressionTypeShiftRight);
                }
                else if (data == "MOD") {
                    exp->setExpressionType(ExpressionTypeMod);
                }
                else if (data == "=>") {
                    exp->setExpressionType(ExpressionTypeCast);
                }
            }
            else if (eventType == ParserEventNumberLiteral)
            {
                std::shared_ptr<LILExpression> exp = std::static_pointer_cast<LILExpression>(this->currentContainer.back());
                std::shared_ptr<LILNumberLiteral> right = std::dynamic_pointer_cast<LILNumberLiteral>(exp->getRight());
                if (right)
                {
                    right->receiveNodeData(data);
                    break;
                }
                std::shared_ptr<LILNumberLiteral> left = std::dynamic_pointer_cast<LILNumberLiteral>(exp->getLeft());
                if (left)
                {
                    left->receiveNodeData(data);
                    break;
                }
            }
            break;
        }

        case BuilderStateFunctionDecl:
        {
            std::shared_ptr<LILFunctionDecl> fd = std::static_pointer_cast<LILFunctionDecl>(this->currentContainer.back());
            if (eventType == ParserEventFunctionTypeFn) {
                fd->setFunctionDeclType(FunctionDeclTypeFn);
            }
            else if (eventType == ParserEventFunctionName) {
                fd->setName(data);
            }
            else if (eventType == ParserEventBody) {
                fd->setReceivesFunctionBody(true);
            }
            else if (eventType == ParserEventExtern) {
                fd->setIsExtern(true);
            }
            else {
                this->currentContainer.back()->receiveNodeData(data);
            }
            break;
        }
            
        case BuilderStateFlowControl:
        {
            std::shared_ptr<LILFlowControl> fc = std::static_pointer_cast<LILFlowControl>(this->currentContainer.back());
            if (eventType == ParserEventFlowControlIfCast) {
                fc->setFlowControlType(FlowControlTypeIfCast);
            } else if (eventType == ParserEventBody) {
                fc->setReceivesFunctionBody(true);
            } else if (eventType == ParserEventFlowControlElse) {
                fc->setReceivesElse(true);
            } else {
                if (!fc->getReceivesElse()) {
                    fc->receiveNodeData(data);
                }
            }
            break;
        }

        case BuilderStateClassDecl:
        {
            if (eventType == ParserEventInherits)
            {
                std::shared_ptr<LILClassDecl> cd = std::static_pointer_cast<LILClassDecl>(this->currentContainer.back());
                cd->setReceivesInherits(true);
            }
            else if (eventType == ParserEventBody)
            {
                std::shared_ptr<LILClassDecl> cd = std::static_pointer_cast<LILClassDecl>(this->currentContainer.back());
                cd->setReceivesBody(true);
            }
            return;
        }

        case BuilderStateFunctionType:
        {
            if (eventType == ParserEventReturnType)
            {
                std::shared_ptr<LILFunctionType> ty = std::static_pointer_cast<LILFunctionType>(this->currentContainer.back());
                ty->setReceivesReturnType(true);
            }
            else if (eventType == ParserEventFunctionVariadic){
                std::shared_ptr<LILFunctionType> ty = std::static_pointer_cast<LILFunctionType>(this->currentContainer.back());
                ty->setIsVariadic(true);
            }
            else if (eventType == ParserEventFunctionTypeFn)
            {
                this->currentContainer.back()->receiveNodeData("fn");
            }
            else
            {
                //sets the name
                this->currentContainer.back()->receiveNodeData(data);
            }
            break;
        }

        case BuilderStateInstruction:
        {
            switch (eventType) {
                case ParserEventColorG1:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeGrayscale1);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventColorG2:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeGrayscale2);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventColorRGB:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeRGB);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventColorRGBA:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeRGBA);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventColorRGBAA:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeRGBAA);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventColorRRGGBB:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeRRGGBB);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventColorRRGGBBA:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeRRGGBBA);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventColorRRGGBBAA:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    instr->setInstructionType(InstructionTypeRRGGBBAA);
                    instr->setIsColorInstruction(true);
                    break;
                }
                case ParserEventInstruction:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    if (data == "needs") {
                        instr->setInstructionType(InstructionTypeNeeds);
                    } else if (data == "import") {
                        instr->setInstructionType(InstructionTypeImport);
                    } else if (data == "export") {
                        instr->setInstructionType(InstructionTypeExport);
                    } else if (data == "paste") {
                        instr->setInstructionType(InstructionTypePaste);
                    } else if (data == "new") {
                        instr->setInstructionType(InstructionTypeNew);
                    } else if (data == "move") {
                        instr->setInstructionType(InstructionTypeMove);
                    } else if (data == "delete") {
                        instr->setInstructionType(InstructionTypeDelete);
                    } else if (data == "configure") {
                        instr->setInstructionType(InstructionTypeConfigure);
                    } else if (data == "getConfig") {
                        instr->setInstructionType(InstructionTypeGetConfig);
                    } else if (data == "bug") {
                        instr->setInstructionType(InstructionTypeBug);
                    } else if (data == "arg") {
                        instr->setInstructionType(InstructionTypeArg);
                    } else if (data == "expand") {
                        instr->setInstructionType(InstructionTypeExpand);
                    }
                    break;
                }
                case ParserEventConstName:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setName(data);
                    break;
                }
                case ParserEventStringLiteral:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    if (instr->getInstructionType() == InstructionTypeConfigure) {
                        auto arg = std::make_shared<LILStringLiteral>();
                        arg->setValue(data);
                        instr->setArgument(arg);
                    }
                    break;
                }
                case ParserEventType:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    auto ty = LILObjectType::make(data);
                    instr->setType(ty);
                    break;
                }
                case ParserEventArgument:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setReceivesBody(false);
                    break;
                }
                    
                case ParserEventBody:
                {
                    std::shared_ptr<LILInstruction> instr = std::static_pointer_cast<LILInstruction>(this->currentContainer.back());
                    instr->setReceivesBody(true);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case BuilderStateIfInstruction:
        {
            auto instr = std::static_pointer_cast<LILIfInstruction>(this->currentContainer.back());
            if (eventType == ParserEventBody) {
                instr->setReceivesThen(true);
            } else if (eventType == ParserEventIfInstructionElse) {
                instr->setReceivesThen(false);
                instr->setReceivesElse(true);
            }
            break;
        }
            
        case BuilderStateSnippetInstruction:
        {
            auto snInstr = std::static_pointer_cast<LILSnippetInstruction>(this->currentContainer.back());
            if (eventType == ParserEventBody) {
                snInstr->setReceivesBody(true);
            } else if (eventType == ParserEventConstName) {
                snInstr->setName(data);
            }
            break;
        }
            
        case BuilderStateVarDecl:
        {
            auto vd = std::static_pointer_cast<LILVarDecl>(this->currentContainer.back());
            switch (eventType) {
                case ParserEventExtern:
                    vd->setIsExtern(true);
                    break;
                case ParserEventVarDecl:
                    if (data == "ivar") {
                        vd->setIsIVar(true);
                    } else if (data == "vvar") {
                        vd->setIsVVar(true);
                    }
                    break;
                case ParserEventReturnType:
                    vd->setReceivesReturnType(true);
                    break;
                    
                default:
                    this->currentContainer.back()->receiveNodeData(data);
                    break;
            }
            break;
        }
            
        case BuilderStateStaticArrayType:
        {
            auto saTy = std::static_pointer_cast<LILStaticArrayType>(this->currentContainer.back());
            if (eventType == ParserEventType) {
                saTy->setReceivesType(true);
            }
            break;
        }

        default:
        {
            if (this->currentNode)
            {
                this->currentNode->receiveNodeData(data);
            }
            else if (this->currentContainer.size() > 0)
            {
                this->currentContainer.back()->receiveNodeData(data);
            }
            break;
        }
    }
}

void LILASTBuilder::receiveSourceLocation(LILString file, size_t startLine, size_t startCol, LILRange newRange)
{
    if (this->_debugAST) {
        std::cerr << "==source location== line: ";
        std::cerr << startLine;
        std::cerr << " column: ";
        std::cerr << startCol;
        std::cerr << "\n";
    }
    
    LILNode::SourceLocation loc;
    loc.file = file;
    loc.line = startLine;
    loc.column = startCol;
    loc.range = newRange;
    if (this->currentNode) {
        this->currentNode->setSourceLocation(loc);
    } else if (this->currentContainer.size() > 0){
        this->currentContainer.back()->setSourceLocation(loc);
    }
}

void LILASTBuilder::receiveError(LILString message, LILString file, size_t startLine, size_t startCol)
{
    LILErrorMessage ei;
    ei.message = message;
    ei.file = file;
    ei.line = startLine;
    ei.column = startCol;
    this->errors.push_back(ei);
}

void LILASTBuilder::receiveForeignLang(const LILString & language, const LILString & content)
{
    if (this->currentNode && this->currentNode->isA(NodeTypeForeignLang)) {
        auto fl = std::static_pointer_cast<LILForeignLang>(this->currentNode);
        fl->setLanguage(language);
        fl->setContent(content);
    }
}

bool LILASTBuilder::hasErrors() const
{
    return this->errors.size() > 0;
}

void LILASTBuilder::setVerbose(bool value)
{
    this->_verbose = value;
}

void LILASTBuilder::setDebugAST(bool value)
{
    this->_debugAST = value;
}

std::shared_ptr<LILRootNode> LILASTBuilder::getRootNode() const
{
    return this->rootNode;
}

void LILASTBuilder::setBuildFlatList(bool value)
{
    this->_buildFlatList = value;
}

bool LILASTBuilder::getBuildFlatList() const
{
    return this->_buildFlatList;
}
