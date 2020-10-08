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
#include "LILExpression.h"
#include "LILFilter.h"
#include "LILFlag.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
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
#include "LILVarDecl.h"
#include "LILVarName.h"

namespace llvm {
    class AllocaInst;
    class Value;
    class Function;
    class Type;
    class Module;
    class raw_ostream;
    class StructType;
}


namespace LIL
{
    class LILClassDecl;
    class LILFunctionType;
    class LILIREmitterPrivate;

    class LILIREmitter : public LILVisitor
    {
    public:
        LILIREmitter(LILString name);
        virtual ~LILIREmitter();
        void reset();
        llvm::Module * getLLVMModule() const;
        void initializeVisit();
        virtual void visit(LILNode * node);

        llvm::Value * emit(LILNode * node);
        llvm::Value * _emit(LILBoolLiteral * value);
        llvm::Value * _emit(LILNumberLiteral * value);
        llvm::Value * _emit(LILPercentageLiteral * value);
        llvm::Value * _emit(LILExpression * value);
        llvm::Value * _emit(LILStringLiteral * value);
        llvm::Value * _emit(LILStringFunction * value);
        llvm::Value * _emit(LILNullLiteral * value);
        llvm::Value * _emit(LILType * value);
        llvm::Value * _emit(LILVarDecl * value);
        llvm::Value * _emit(LILClassDecl * value);
        llvm::Value * _emit(LILObjectDefinition * value);
        llvm::Value * _emit(LILAssignment * value);
        llvm::Value * _emit(LILValuePath * value);
        llvm::Value * _emitGEP(llvm::Value * llvmValue, LILString className, LILUnitI32 fieldIndex, LILString fieldName, LILUnitI32 arrayIndex);
        llvm::Value * _emit(LILPropertyName * value);
        llvm::Value * _emit(LILVarName * value);
        llvm::Value * _emit(LILRule * value);
        llvm::Value * _emit(LILSimpleSelector * value);
        llvm::Value * _emit(LILSelectorChain * value);
        llvm::Value * _emit(LILSelector * value);
        llvm::Value * _emit(LILCombinator * value);
        llvm::Value * _emit(LILFilter * value);
        llvm::Value * _emit(LILFlag * value);
        llvm::Function * _emit(LILFunctionDecl * value);
        llvm::Function * _emitFnSignature(std::string name, const std::shared_ptr<LILFunctionType> fnTy);
        llvm::Function * _emitFnSignature(std::string name, std::vector<llvm::Type*>, const std::shared_ptr<LILFunctionType> fnTy);
        llvm::Function * _emitFn( LILFunctionDecl * value);
        llvm::Function * _emitFnBody(llvm::Function * fun, LILFunctionDecl * value);
        llvm::Function * _emitMethod(LILFunctionDecl * value, LILClassDecl * classValue);

        llvm::Value * _emit(LILFunctionCall * value);
        llvm::Value * _emit(LILFunctionCall * value, LILString name);
        llvm::Value * _emit(LILFlowControl * value);
        llvm::Value * _emitIf(LILFlowControl * value);
        llvm::Value * _emit(LILFlowControlCall * value);
        llvm::Value * _emitReturn(LILFlowControlCall * value);
        llvm::Value * _emit(LILInstruction * value);

        llvm::Value * emitPointer(LILNode * node);
        llvm::Value * _emitPointer(LILValuePath * vp);

        void printIR(llvm::raw_ostream & file) const;
        void setDebug(bool value);

        llvm::StructType * extractStructFromClass(LILClassDecl * value);

    private:
        LILIREmitterPrivate *const d;
        llvm::Type * llvmTypeFromLILType(std::shared_ptr<LILType> type);
        std::shared_ptr<LILFunctionDecl> chooseFnByType(std::shared_ptr<LILVarDecl> vd, std::vector<std::shared_ptr<LILType>> types);
        llvm::AllocaInst * createEntryBlockAlloca(llvm::Function * fun, const std::string & name, llvm::Type * llvmType);
        bool _debug;
    };
}

#endif
