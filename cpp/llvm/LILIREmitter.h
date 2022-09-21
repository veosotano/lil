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
 *      This file converts the ast into IR representation
 *
 ********************************************************************/

#ifndef LILIREMITTER_H
#define LILIREMITTER_H

#include "LILVisitor.h"
#include "LILErrorMessage.h"
#include "LILNode.h"

#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILClassDecl.h"
#include "LILCombinator.h"
#include "LILConversionDecl.h"
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILForeignLang.h"
#include "LILInstruction.h"
#include "LILNullLiteral.h"
#include "LILNumberLiteral.h"
#include "LILObjectDefinition.h"
#include "LILObjectType.h"
#include "LILPercentageLiteral.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILRule.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILStringFunction.h"
#include "LILStringLiteral.h"
#include "LILType.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

#include "LLVMIRParser.h"

namespace llvm {
    class AllocaInst;
    class Value;
    class Function;
    class FunctionType;
    class Type;
    class Module;
    class raw_ostream;
    class StructType;
}


namespace LIL
{
    class LILClassDecl;
    class LILElement;
    class LILFunctionType;
    class LILMultipleType;
    class LILIREmitterPrivate;

    class LILIREmitter : public LILVisitor , llvm::LLVMIRParserReceiver
    {
    public:
        LILIREmitter(LILString name);
        virtual ~LILIREmitter();
        void reset();
        llvm::Module * getLLVMModule() const;
        void initializeVisit() override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void emitRuleNames(LILRootNode * rootNode);
        void emitRuleName(LILRule * rule);
        void hoistDeclarations(std::shared_ptr<LILRootNode> rootNode);
        void emitRuleFnSignature(LILRule * rule);

        llvm::Value * emit(LILNode * node);
        llvm::Value * _emitBool(LILBoolLiteral * value);
        llvm::Value * _emitNum(LILNumberLiteral * value);
        llvm::Value * _emitPercent(LILPercentageLiteral * value);
        llvm::Value * _emitExp(LILExpression * value);
        llvm::Value * _emitExpression(ExpressionType expType, llvm::Value * leftV, llvm::Value * rightV);
        llvm::Value * _emitUExp(LILUnaryExpression * value);
        llvm::Value * _emitNotExp(LILUnaryExpression * value);
        llvm::Value * _emitCast(LILExpression * value);
        llvm::Value * _emitStr(LILStringLiteral * value);
        llvm::Value * _emitNull(LILNullLiteral * value);
        llvm::Value * _emitType(LILType * value);
        llvm::Value * _emitVarDecl(LILVarDecl * value);
        llvm::Value * _emitConvDecl(LILConversionDecl * value);
        llvm::Value * _emitClassDecl(LILClassDecl * value);
        llvm::Value * _emitObjDef(LILObjectDefinition * value);
        llvm::Value * _emitAsgmt(LILAssignment * value);
        llvm::Value * _emitVP(LILValuePath * value);
        llvm::Value * _emitGEP(llvm::Value * llvmValue, LILString className, LILUnitI32 fieldIndex, LILString fieldName, bool stepThroughPointer);
        llvm::Value * _emitGEP(llvm::Value * llvmValue, llvm::Type * llvmType, bool useField, LILUnitI32 fieldIndex, LILString fieldName, bool stepThroughPointer, bool useArrayIndex, LILUnitI32 arrayIndex);
        llvm::Value * _emitPN(LILPropertyName * value);
        llvm::Value * _emitVN(LILVarName * value);
        llvm::Value * _emitRule(LILRule * value);
        llvm::Value * _emitRuleInner(LILRule * value);
        void _connectRule(LILRule * value, llvm::Value * parentId, llvm::Value * parentIndex, llvm::Value * thisObj);
        llvm::Value* _getContainerNameFromSelectorChain(std::shared_ptr<LILSelectorChain> selCh);
        LILString _newRuleFnName();
        llvm::Value * _emitSSel(LILSimpleSelector * value);
        llvm::Value * _emitSelCh(LILSelectorChain * value, bool & outIsId, bool & outNeedsAlloca, llvm::Value* parentIndex, llvm::Value * thisObj);
        std::shared_ptr<LILRule> findRuleForSelChAndFlag(LILSelectorChain * selCh, LILFlag * flag) const;
        llvm::Value * _emitSel(LILSelector * value);
        llvm::Value * _emitComb(LILCombinator * value);
        llvm::Value * _emitFlt(LILFilter * value);
        llvm::Value * _emitFlag(LILFlag * value);
        llvm::Function * _emitFnDecl(LILFunctionDecl * value);
        llvm::Function * _emitFnSignature(std::string name, LILFunctionType * fnTy);
        llvm::FunctionType * _emitLlvmFnType(LILFunctionType * fnTy);
        llvm::Function * _emitFn( LILFunctionDecl * value);
        llvm::Function * _emitFnBody(llvm::Function * fun, LILFunctionDecl * value);
        void _emitDestructors(llvm::Value * ir, std::shared_ptr<LILClassDecl> cd, const LILString & name);
        llvm::Value * _emitEvaluables(const std::vector<std::shared_ptr<LILNode>> & nodes);
        llvm::Value * _emitFC(LILFunctionCall * value);
        llvm::Value * _emitFCMultipleValues(std::vector<std::shared_ptr<LILFunctionDecl>> funcDecls, LILFunctionCall * value, llvm::Value * instance = nullptr, std::shared_ptr<LILType> instanceTy = nullptr);
        llvm::Value * _emitFunctionCall(LILFunctionCall * value, LILString name, LILFunctionType * fnTy, llvm::Value * instance, bool skipArgument = false, size_t skipArgIndex = 0);
        llvm::Value * _emitFCArg(LILNode * value, LILType * ty);
        llvm::Value * _emitFunctionCallMT(LILFunctionCall * value, LILString name, std::vector<std::shared_ptr<LILType>> types, LILFunctionType * fnTy, llvm::Value * instance);
        llvm::Value * _emitFunctionCallPointer(llvm::Value * fun, LILFunctionCall * value, LILFunctionType * fnTy, llvm::Value * instance);
        llvm::Value * _emitFlowC(LILFlowControl * value);
        llvm::Value * _emitIf(LILFlowControl * value);
        llvm::Value * _emitIfCast(LILFlowControl * value);
        llvm::Value * _emitIfCastConditionForNullable(bool negated, LILType * ty, LILNode * val);
        llvm::Value * _emitIfCastConditionForMT(bool negated, LILType * ty, LILMultipleType * multiTy, LILNode * val);
        llvm::Value * _emitFor(LILFlowControl * value);
        llvm::Value * _emitLoop(LILFlowControl * value);
        llvm::Value * _emitFlowCCall(LILFlowControlCall * value);
        llvm::Value * _emitReturn(LILFlowControlCall * value);
        llvm::Value * _emitRepeat(LILFlowControlCall * value);
        llvm::Value * _emitBreak(LILFlowControlCall * value);
        llvm::Value * _emitInstr(LILInstruction * value);
        llvm::Value * _emitForeignLang(LILForeignLang * value);
        llvm::Value * _emitValList(LILValueList * value);
        
        void receiveLLVMIRData(llvm::LLVMIRParserEvent eventType, std::string data) override;

        llvm::Value * emitPointer(LILNode * node);
        llvm::Value * _emitPointer(LILValuePath * vp);

        void printIR(llvm::raw_ostream & file) const;
        void setDebug(bool value);

        llvm::StructType * extractStructFromClass(LILClassDecl * value);
        size_t extractSizeFromNumberLiteral(LILNumberLiteral * value) const;
        
        llvm::Value * emitNullable(LILNode * node, LILType * targetTy);
        llvm::Value * emitForMultipleType(LILNode * node, std::shared_ptr<LILMultipleType> multiTy);
        llvm::Value * emitNullableToMultiTyConversion(LILNode * node, LILMultipleType * multiTy, const LILString & name);
        llvm::Value * emitMultiTyToMultiTyConversion(LILNode * node, LILMultipleType * multiTy, const LILString & name);
        llvm::Value * emitUnwrappedFromMT(LILNode * node, LILType * targetTy);
        llvm::Value * emitUnwrappedPointerFromMT(llvm::Value * val, LILType *targetTy, LILMultipleType * multiTy);
        
        std::shared_ptr<LILType> getMostAlignedType(const std::vector<std::shared_ptr<LILType>> & types) const;
        size_t getSizeOfType(std::shared_ptr<LILType> ty) const;
        const std::shared_ptr<LILElement> & getDOM() const;
        void setDOM(const std::shared_ptr<LILElement> & dom);

    private:
        LILIREmitterPrivate *const d;
        llvm::Type * llvmTypeFromLILType(LILType * type);
        std::shared_ptr<LILFunctionDecl> chooseFnByType(std::vector<std::shared_ptr<LILFunctionDecl>> funcDecls, std::vector<std::shared_ptr<LILType>> types);
        llvm::AllocaInst * createEntryBlockAlloca(llvm::Function * fun, const std::string & name, llvm::Type * llvmType);
        bool _isAnyPtrTy(LILType * ty) const;
        void _convertLlvmValueIfNeeded(llvm::Value ** llvmValue, LILType * ty1, LILType * ty2);

        bool _needsTemporaryVariable(LILNode * node);
        bool _debug;
    };
}

#endif
