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
 *      This file takes parsed llvm IR and adds it to the IREmitter
 *
 ********************************************************************/

#ifndef LLVMIRPARSER_H
#define LLVMIRPARSER_H

#include "LLVMIRLexer.h"

namespace llvm {

    enum LLVMIRParserEvent
    {
        LLVMIRParserEventNone = 0,
        LLVMIRParserEventInvalid,
        LLVMIRParserEventGlobalID,
        LLVMIRParserEventGlobalVar,
        LLVMIRParserEventLocalVarID,
        LLVMIRParserEventLocalVar,
        LLVMIRParserEventPunctuation,
        LLVMIRParserEventKeyword,
        LLVMIRParserEventWhitespace,
        LLVMIRParserEventComment,
        LLVMIRParserEventInstruction,
        LLVMIRParserEventUnreachable,
        LLVMIRParserEventDeclare,
        LLVMIRParserEventDefine,
        LLVMIRParserEventFunctionHeader,
        LLVMIRParserEventSection,
        LLVMIRParserEventPartition,
        LLVMIRParserEventGc,
        LLVMIRParserEventPrologue,
        LLVMIRParserEventPrefix,
        LLVMIRParserEventPersonality,
        LLVMIRParserEventFunctionName,
        LLVMIRParserEventFunctionID,
        LLVMIRParserEventArgumentList,
        LLVMIRParserEventVariadic,
        LLVMIRParserEventAttributeName,
        LLVMIRParserEventFunctionBody,
        LLVMIRParserEventBasicBlock,
        
        LLVMIRParserEventLinkage,
        LLVMIRParserEventDSOLocal,
        LLVMIRParserEventDSOPreemptable,
        LLVMIRParserEventDLLImport,
        LLVMIRParserEventDLLExport,
        LLVMIRParserEventUnnamedAddr,
        LLVMIRParserEventComdat,
        LLVMIRParserEventComdatVar,
        LLVMIRParserEventGlobalObjectMetadata,
        LLVMIRParserEventMetadata,
        LLVMIRParserEventRet,
        LLVMIRParserEventBr,
        LLVMIRParserEventSwitch,
        LLVMIRParserEventIndirectBr,
        LLVMIRParserEventInvoke,
        LLVMIRParserEventResume,
        LLVMIRParserEventCleanupRet,
        LLVMIRParserEventCatchRet,
        LLVMIRParserEventCatchSwitch,
        LLVMIRParserEventCatchPad,
        LLVMIRParserEventCleanupPad,
        LLVMIRParserEventCallBr,
        LLVMIRParserEventFneg,
        LLVMIRParserEventAdd,
        LLVMIRParserEventSub,
        LLVMIRParserEventMul,
        LLVMIRParserEventShl,
        LLVMIRParserEventFAdd,
        LLVMIRParserEventFSub,
        LLVMIRParserEventFMul,
        LLVMIRParserEventFDiv,
        LLVMIRParserEventFRem,
        LLVMIRParserEventSDiv,
        LLVMIRParserEventUDiv,
        LLVMIRParserEventLShr,
        LLVMIRParserEventAShr,
        LLVMIRParserEventCast,
        LLVMIRParserEventLogical,
        LLVMIRParserEventCompare,
        LLVMIRParserEventCmpPredicate,
        LLVMIRParserEventExtractElement,
        LLVMIRParserEventInsertElement,
        LLVMIRParserEventShuffleVector,
        LLVMIRParserEventPhi,
        LLVMIRParserEventCall,
        LLVMIRParserEventExtractValue,
        LLVMIRParserEventInsertValue,
        LLVMIRParserEventIndexList,
        LLVMIRParserEventAlloca,
        LLVMIRParserEventLoad,
        LLVMIRParserEventStore,
        LLVMIRParserEventScope,
        LLVMIRParserEventOrdering,
        LLVMIRParserEventCmpXchng,
        LLVMIRParserEventAtomicRMW,
        LLVMIRParserEventAtomicRMWInst,
        LLVMIRParserEventFence,
        LLVMIRParserEventGetElementPtr,
        LLVMIRParserEventInbounds,

        LLVMIRParserEventType,
        LLVMIRParserEventBasicType,
        LLVMIRParserEventPointerType,
        LLVMIRParserEventStructType,
        LLVMIRParserEventStructBody,
        LLVMIRParserEventVectorType,
        LLVMIRParserEventVscale,
        LLVMIRParserEventArrayType,
        LLVMIRParserEventLabelType,
        LLVMIRParserEventMetadataType,
        LLVMIRParserEventLocalVarType,
        LLVMIRParserEventLocalVarIDType,
        LLVMIRParserEventAddrSpace,
        LLVMIRParserEventParameterList,
        LLVMIRParserEventCallingConv,
        LLVMIRParserEventParamAttr,
        LLVMIRParserEventReturnAttr,
        LLVMIRParserEventFnAttribute,
        LLVMIRParserEventAttrGrpID,
        LLVMIRParserEventAlign,
        LLVMIRParserEventAlignStack,
        LLVMIRParserEventAllocSize,
        LLVMIRParserEventOperandBundle,
        LLVMIRParserEventValue,
        LLVMIRParserEventGlobalValue,
        LLVMIRParserEventAPSInt,
        LLVMIRParserEventAPSFloat,
        LLVMIRParserEventStringConstant,
        LLVMIRParserEventBoolConstant,
        LLVMIRParserEventNull,
        LLVMIRParserEventUndef,
        LLVMIRParserEventZero,
        LLVMIRParserEventNoneKw,
        LLVMIRParserEventPackedStruct,
        LLVMIRParserEventVector,
        LLVMIRParserEventAsm,
        LLVMIRParserEventSideEffect,
        LLVMIRParserEventIntelDialect,
        LLVMIRParserEventBlockAddress,
        LLVMIRParserEventSelect,
        LLVMIRParserEventUnaryOp,
        LLVMIRParserEventFast,
        LLVMIRParserEventNnan,
        LLVMIRParserEventNinf,
        LLVMIRParserEventNsz,
        LLVMIRParserEventArcp,
        LLVMIRParserEventContract,
        LLVMIRParserEventReassoc,
        LLVMIRParserEventAfn,
        LLVMIRParserEventNuw,
        LLVMIRParserEventNsw,
        LLVMIRParserEventExact,
        LLVMIRParserEventInRange,
        LLVMIRParserEventInalloca,
        LLVMIRParserEventSwifterror,
        LLVMIRParserEventAtomic,
        LLVMIRParserEventVolatile,
        LLVMIRParserEventSyncscope,
        LLVMIRParserEventUnordered,
        LLVMIRParserEventMonotonic,
        LLVMIRParserEventAcquire,
        LLVMIRParserEventRelease,
        LLVMIRParserEventAcqRel,
        LLVMIRParserEventSeqCst,
        LLVMIRParserEventWeak,
    };
    
    class LLVMIRParserReceiver
    {
    public:
        virtual void receiveLLVMIRNodeStart(LLVMIRParserEvent eventType) {}
        virtual void receiveLLVMIRData(LLVMIRParserEvent eventType, std::string data) = 0;
        virtual void receiveLLVMIRNodeEnd(LLVMIRParserEvent eventType) {}
        virtual void receiveLLVMIRError(std::string message, size_t line, size_t column) {};
        virtual void receiveLLVMIRSourceLocation(size_t line, size_t column, size_t startIndex, size_t endIndex) {};
    };

    class LLVMIRParser
    {
    public:
        typedef LLVMIRLexer::LocTy LocTy;
        unsigned int line;
        unsigned int column;
        unsigned int index;

    public:
        LLVMIRParser(StringRef buffer, SourceMgr & sourceMgr, SMDiagnostic & err, LLVMContext & ctxt, LLVMIRParserReceiver * receiver);

        bool atEndOfSource();
        void readNextToken();
        void skipWhitespaceAndComments();
        bool run();
        bool parseNext();
        bool parseLocalVar();
        bool parseInstruction(bool & outAteExtraComma);
        bool parseUnreachable();
        bool parseDeclare();
        bool parseDefine();
        bool parseFunctionHeader();
        bool parseOptionalFunctionMetadata();
        bool parseGlobalObjectMetadataAttachment();
        bool parseMetadataAttachment();
        bool parseInstructionMetada();
        bool parseArgumentList();
        bool parseOptionalUnnamedAddr();
        bool parseOptionalComdat();
        bool parseFunctionBody();
        bool parseBasicBlock();
        
        bool parseUseListOrder();
        bool parseRet();
        bool parseBr();
        bool parseSwitch();
        bool parseIndirectBr();
        bool parseInvoke();
        bool parseOptionalLinkage();
        bool parseOptionalLinkageAux(bool & outHasLinkage);
        bool parseOptionalDSOLocal();
        bool parseOptionalDLLStorageClass();
        bool parseOptionalCallingConv();
        bool parseOptionalReturnAttrs();
        bool parseStringAttribute();
        bool parseOptionalDerefAttrBytes();
        bool parseOptionalAlignment();
        bool parseOptionalAddrSpace();
        bool parseParameterList();
        bool parseFnAttributeValuePairs(bool inAttrGrp);
        bool parseOptionalOperandBundles();
        bool parseMetadataAsValue();
        bool parseOptionalParamAttrs();
        bool parseByValWithOptionalType();
        bool parseAllocSizeArguments();
        bool parseResume();
        bool parseCleanupRet();
        bool parseCatchRet();
        bool parseCatchSwitch();
        bool parseCatchPad();
        bool parseCleanupPad();
        bool parseCallBr();
        bool parseFneg();
        bool parseCast();
        bool parseLogical();
        bool parseCompare();
        bool parseCmpPredicate();
        bool parseTypeAndValue(LLVMIRParserEvent & outType);
        bool parseType(LLVMIRParserEvent & outType);
        bool parseAnonStructType();
        bool parseStructBody();
        bool parseArrayVectorType(bool isVector);
        bool parseValue(LLVMIRParserEvent & outType);
        bool parseSelect();
        bool parseOptionalFastMathFlags();
        bool parseUnaryOp(bool isFP);
        bool parseBinaryOp(bool isFP, bool allowWrap, bool allowExact);
        bool parseArithmetic();
        bool parseGlobalValueVector(bool wantsInRange, bool & outIsInRange);
        bool parseGlobalTypeAndValue(LLVMIRParserEvent & outType);
        bool parseGlobalValue(LLVMIRParserEvent & outType);
        bool parseExtractElement();
        bool parseInsertElement();
        bool parseShuffleVector();
        bool parsePhi(bool & outAteExtraComma);
        bool parseLandingPad();
        bool parseCall();
        bool parseExtractValue(bool & outAteExtraComma);
        bool parseInsertValue(bool & outAteExtraComma);
        bool parseIndexList(bool & outAteExtraComma);
        bool parseAlloca(bool & outAteExtraComma);
        bool parseOptionalCommaAddrSpace(bool & outAteExtraComma);
        bool parseOptionalCommaAlign(bool & outAteExtraComma);
        bool parseLoad(bool & outAteExtraComma);
        bool parseStore(bool & outAteExtraComma);
        bool parseScopeAndOrdering();
        bool parseScope();
        bool parseOrdering();
        bool parseCmpXchng(bool & outAteExtraComma);
        bool parseAtomicRMW(bool & outAteExtraComma);
        bool parseFence(bool & outAteExtraComma);
        bool parseGetElementPtr(bool & outAteExtraComma);

        bool isMustTailCall();

    private:
        LLVMIRLexer _lexer;
        LLVMIRParserReceiver * _receiver;
        
        
    };
}
#endif
