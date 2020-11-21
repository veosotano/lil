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

#include "LLVMIRParser.h"

#include <iostream>

using namespace llvm;

#define START_NODE(parser_event) \
size_t __startCol = this->column; \
size_t __startLine = this->line; \
size_t __startIndex = this->index; \
LLVMIRParserEvent __parserEvent = parser_event; \
this->_receiver->receiveLLVMIRNodeStart(__parserEvent);

#define SEND_DATA_ \
this->_receiver->receiveLLVMIRData(__parserEvent, this->_lexer.getStrVal());

#define SEND_DATA(parser_event) \
this->_receiver->receiveLLVMIRData(parser_event, this->_lexer.getStrVal());

#define SEND_STR(parser_event, str_value) \
this->_receiver->receiveLLVMIRData(parser_event, str_value);

#define EXPECT(tokenType, send_str, error_message, parser_event) \
if (this->_lexer.getKind() == tokenType){ \
    SEND_STR(parser_event, send_str) \
    this->readNextToken(); \
} else { \
    this->_receiver->receiveLLVMIRError(error_message, this->line, this->column); \
    this->_receiver->receiveLLVMIRNodeEnd(__parserEvent); \
    CANCEL_NODE \
}

#define OPTIONAL(tokenType, parser_event) \
if (this->_lexer.getKind() == tokenType){ \
    SEND_DATA(parser_event) \
    this->readNextToken(); \
}

#define ERROR(error_message) \
this->_receiver->receiveLLVMIRError(error_message, this->line, this->column); \
this->_receiver->receiveLLVMIRNodeEnd(__parserEvent); \
return false;

#define CANCEL_NODE { \
this->_receiver->receiveLLVMIRNodeEnd(__parserEvent); \
return false; \
}

#define END_NODE_NO_RETURN \
this->_receiver->receiveLLVMIRSourceLocation(__startLine, __startCol, __startIndex, this->index - __startIndex); \
this->_receiver->receiveLLVMIRNodeEnd(__parserEvent);

#define END_NODE \
END_NODE_NO_RETURN \
return true;

#define INVALID(error_message) \
this->_receiver->receiveLLVMIRData(LLVMIRParserEventInvalid, this->_lexer.getStrVal()); \
this->_receiver->receiveLLVMIRError(error_message, this->line, this->column);

LLVMIRParser::LLVMIRParser(StringRef buffer, SourceMgr & sourceMgr, SMDiagnostic & err, LLVMContext & ctxt, LLVMIRParserReceiver * receiver)
: _lexer(buffer, sourceMgr, err, ctxt)
, _receiver(receiver)
, line(0)
, column(0)
, index(0)
{
    
}

void LLVMIRParser::skipWhitespaceAndComments()
{
    while (1) {
        auto tokenKind = this->_lexer.getKind();
        if (tokenKind != lltok::Whitespace && tokenKind != lltok::Comment) {
            return;
        }
        const std::string & data = this->_lexer.getStrVal();
        LLVMIRParserEvent event;
        if (tokenKind == lltok::Whitespace) {
            event = LLVMIRParserEventWhitespace;
        } else {
            event = LLVMIRParserEventComment;
        }
        this->_receiver->receiveLLVMIRData(event, data);
        if (data == "\r" || data == "\n") {
            this->line += 1;
        }
        this->_lexer.Lex();
    }
}

bool LLVMIRParser::atEndOfSource()
{
    return this->_lexer.getKind() == lltok::Eof;
}

void LLVMIRParser::readNextToken()
{
    this->_lexer.Lex();
    this->skipWhitespaceAndComments();
}

bool LLVMIRParser::run()
{
    //read the first token
    this->readNextToken();

    bool done = false;
    while (!done) {
        done = true;

        bool success = this->parseNext();
        if (success) {
            done = false;
        }
    }
    return true;
}

bool LLVMIRParser::parseNext()
{
    switch (this->_lexer.getKind()) {
        case lltok::Eof:
        {
            return false;
        }
        case lltok::Whitespace:
        {
            this->skipWhitespaceAndComments();
            return true;
        }
        case lltok::Comment:
        {
            this->_receiver->receiveLLVMIRData(LLVMIRParserEventComment, this->_lexer.getStrVal());
            this->readNextToken();
            return true;
        }
        case lltok::LocalVarID:
        case lltok::LocalVar:
        {
            return this->parseLocalVar();
        }
        case lltok::kw_unreachable:
        case lltok::kw_ret:
        case lltok::kw_br:
        case lltok::kw_switch:
        case lltok::kw_indirectbr:
        case lltok::kw_invoke:
        case lltok::kw_resume:
        case lltok::kw_cleanupret:
        case lltok::kw_catchret:
        case lltok::kw_catchswitch:
        case lltok::kw_catchpad:
        case lltok::kw_cleanuppad:
        case lltok::kw_callbr:
        case lltok::kw_fneg:
        case lltok::kw_add:
        case lltok::kw_sub:
        case lltok::kw_mul:
        case lltok::kw_shl:
        case lltok::kw_fadd:
        case lltok::kw_fsub:
        case lltok::kw_fmul:
        case lltok::kw_fdiv:
        case lltok::kw_frem:
        case lltok::kw_sdiv:
        case lltok::kw_udiv:
        case lltok::kw_lshr:
        case lltok::kw_ashr:
        case lltok::kw_urem:
        case lltok::kw_srem:
        case lltok::kw_and:
        case lltok::kw_or:
        case lltok::kw_xor:
        case lltok::kw_icmp:
        case lltok::kw_fcmp:
        case lltok::kw_trunc:
        case lltok::kw_zext:
        case lltok::kw_sext:
        case lltok::kw_fptrunc:
        case lltok::kw_fpext:
        case lltok::kw_bitcast:
        case lltok::kw_addrspacecast:
        case lltok::kw_uitofp:
        case lltok::kw_sitofp:
        case lltok::kw_fptoui:
        case lltok::kw_fptosi:
        case lltok::kw_inttoptr:
        case lltok::kw_ptrtoint:
        case lltok::kw_select:
        case lltok::kw_va_arg:
        case lltok::kw_extractelement:
        case lltok::kw_insertelement:
        case lltok::kw_shufflevector:
        case lltok::kw_phi:
        case lltok::kw_landingpad:
        case lltok::kw_call:
        case lltok::kw_tail:
        case lltok::kw_musttail:
        case lltok::kw_notail:
        case lltok::kw_alloca:
        case lltok::kw_load:
        case lltok::kw_store:
        case lltok::kw_cmpxchg:
        case lltok::kw_atomicrmw:
        case lltok::kw_fence:
        case lltok::kw_getelementptr:
        case lltok::kw_extractvalue:
        case lltok::kw_insertvalue:
        {
            bool outAteExtraComma = false;
            return this->parseInstruction(outAteExtraComma);
        }
        case lltok::kw_define:
        {
            return this->parseDefine();
        }
   
        default:
            std::cerr << "!!!!!!!!!!ERROR: UNKNOWN TOKEN TYPE!!!!!!!!!!!!!!!!\n";
            break;
    }
    return false;
}

bool LLVMIRParser::parseLocalVar()
{
    START_NODE(LLVMIRParserEventLocalVar)
    if (this->_lexer.getKind() != lltok::LocalVarID && this->_lexer.getKind() != lltok::LocalVar) {
        return false;
    }
    SEND_DATA(LLVMIRParserEventLocalVar);
    this->readNextToken();

    if (this->_lexer.getKind() != lltok::equal) {
        this->_receiver->receiveLLVMIRError("Unexpected token", this->line, this->column);
    }
    //consume the =
    SEND_STR(LLVMIRParserEventPunctuation, "=");
    this->readNextToken();
    
    bool outAteExtraComma = false;
    this->parseInstruction(outAteExtraComma);
    
    END_NODE
}

bool LLVMIRParser::parseInstruction(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventInstruction);
    bool valid = false;
    switch (this->_lexer.getKind()) {
        case lltok::Eof:
        {
            this->_receiver->receiveLLVMIRError("found end of file when expecting more instructions", this->line, this->column);
            break;
        }
        // Terminator Instructions.
        case lltok::kw_unreachable:
        {
            valid = this->parseUnreachable();
            break;
        }
        case lltok::kw_ret:
        {
            return this->parseRet();
        }
        case lltok::kw_br:
        {
            return this->parseBr();
        }
        case lltok::kw_switch:
        {
            return this->parseSwitch();
        }
        case lltok::kw_indirectbr:
        {
            return this->parseIndirectBr();
        }
        case lltok::kw_invoke:
        {
            return this->parseInvoke();
        }
        case lltok::kw_resume:
        {
            return this->parseResume();
        }
        case lltok::kw_cleanupret:
        {
            return this->parseCleanupRet();
        }
        case lltok::kw_catchret:
        {
            return this->parseCatchRet();
        }
        case lltok::kw_catchswitch:
        {
            return this->parseCatchSwitch();
        }
        case lltok::kw_catchpad:
        {
            return this->parseCatchPad();
        }
        case lltok::kw_cleanuppad:
        {
            return this->parseCleanupPad();
        }
        case lltok::kw_callbr:
        {
            return this->parseCallBr();
        }
        case lltok::kw_fneg:
        {
            return this->parseFneg();
        }

        case lltok::kw_add:
        case lltok::kw_sub:
        case lltok::kw_mul:
        case lltok::kw_shl:
        {
            return this->parseBinaryOp(false, true, false);
        }
        case lltok::kw_fadd:
        case lltok::kw_fsub:
        case lltok::kw_fmul:
        case lltok::kw_fdiv:
        case lltok::kw_frem:
        {
            return this->parseBinaryOp(true, false, false);
        }
        case lltok::kw_sdiv:
        case lltok::kw_udiv:
        case lltok::kw_lshr:
        case lltok::kw_ashr:
        {
            return this->parseBinaryOp(false, false, false);
        }
            
        case lltok::kw_urem:
        case lltok::kw_srem:
        {
            return this->parseArithmetic();
        }
            
        case lltok::kw_and:
        case lltok::kw_or:
        case lltok::kw_xor:
        {
            valid = this->parseLogical();
            break;
        }
        case lltok::kw_icmp:
        {
            valid = this->parseCompare();
            break;
        }
        case lltok::kw_fcmp:
        {
            valid = this->parseOptionalFastMathFlags();
            if (!valid) {
                CANCEL_NODE
            }
            valid = this->parseCompare();
            return false;
        }
        //casts
        case lltok::kw_trunc:
        case lltok::kw_zext:
        case lltok::kw_sext:
        case lltok::kw_fptrunc:
        case lltok::kw_fpext:
        case lltok::kw_bitcast:
        case lltok::kw_addrspacecast:
        case lltok::kw_uitofp:
        case lltok::kw_sitofp:
        case lltok::kw_fptoui:
        case lltok::kw_fptosi:
        case lltok::kw_inttoptr:
        case lltok::kw_ptrtoint:
            return this->parseCast();

        //other
        case lltok::kw_select:
        {
            valid = this->parseSelect();
            break;
        }
            
        case lltok::kw_va_arg:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_extractelement:
        {
            valid = this->parseExtractElement();
            break;
        }
        case lltok::kw_insertelement:
        {
            valid = this->parseInsertElement();
            break;
        }
        case lltok::kw_shufflevector:
        {
            valid = this->parseShuffleVector();
            break;
        }
        case lltok::kw_phi:
        {
            valid = this->parsePhi(outAteExtraComma);
            break;
        }
        case lltok::kw_landingpad:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }

        //call
        case lltok::kw_call:
        {
            valid = this->parseCall();
            break;
        }
        case lltok::kw_tail:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_musttail:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_notail:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_alloca:
        {
            valid = this->parseAlloca(outAteExtraComma);
            break;
        }
        case lltok::kw_load:
        {
            valid = this->parseLoad(outAteExtraComma);
            break;
        }
        case lltok::kw_store:
        {
            valid = this->parseStore(outAteExtraComma);
            break;
        }
        case lltok::kw_cmpxchg:
        {
            valid = this->parseCmpXchng(outAteExtraComma);
            break;
        }
        case lltok::kw_atomicrmw:
        {
            valid = this->parseAtomicRMW(outAteExtraComma);
            break;
        }
        case lltok::kw_fence:
        {
            valid = this->parseFence(outAteExtraComma);
            break;
        }
        case lltok::kw_getelementptr:
        {
            valid = this->parseGetElementPtr(outAteExtraComma);
            break;
        }
        case lltok::kw_extractvalue:
        {
            valid = this->parseExtractValue(outAteExtraComma);
            break;
        }
        case lltok::kw_insertvalue:
        {
            valid = this->parseInsertValue(outAteExtraComma);
            break;
        }

        default:
            this->_receiver->receiveLLVMIRError("expected instruction opcode", this->line, this->column);
            break;
    }
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseUnreachable()
{
    START_NODE(LLVMIRParserEventUnreachable)
    this->_receiver->receiveLLVMIRData(LLVMIRParserEventUnreachable, "unreachable");
    this->readNextToken();
    END_NODE
}

bool LLVMIRParser::parseDeclare()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseDefine()
{
    START_NODE(LLVMIRParserEventDefine)
    SEND_DATA_
    this->readNextToken();
    bool valid = this->parseFunctionHeader();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalFunctionMetadata();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseFunctionBody();

    END_NODE
}

bool LLVMIRParser::parseFunctionHeader()
{
    START_NODE(LLVMIRParserEventFunctionHeader)
    bool valid = this->parseOptionalLinkage();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalCallingConv();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalReturnAttrs();
    if (!valid) {
        CANCEL_NODE
    }
    LLVMIRParserEvent ignoreType;
    valid = this->parseType(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    auto nameTokKind = this->_lexer.getKind();
    if (nameTokKind == lltok::GlobalVar) {
        SEND_STR(LLVMIRParserEventFunctionName, "@" + this->_lexer.getStrVal() )
        this->readNextToken();
    } else if (nameTokKind == lltok::GlobalID){
        SEND_STR(LLVMIRParserEventFunctionID, "%" + this->_lexer.getStrVal() )
        this->readNextToken();
    }

    valid = this->parseArgumentList();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalUnnamedAddr();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseFnAttributeValuePairs(false);
    if (this->_lexer.getKind() == lltok::kw_section) {
        SEND_DATA(LLVMIRParserEventSection)
        this->readNextToken();
        valid = this->parseValue(ignoreType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    if (this->_lexer.getKind() == lltok::kw_partition) {
        SEND_DATA(LLVMIRParserEventPartition)
        this->readNextToken();
        valid = this->parseValue(ignoreType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    valid = this->parseOptionalComdat();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalAlignment();
    if (this->_lexer.getKind() == lltok::kw_gc) {
        SEND_DATA(LLVMIRParserEventGc)
        this->readNextToken();
        valid = this->parseValue(ignoreType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    if (this->_lexer.getKind() == lltok::kw_prefix) {
        SEND_DATA(LLVMIRParserEventPrefix)
        this->readNextToken();
        valid = this->parseGlobalTypeAndValue(ignoreType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    if (this->_lexer.getKind() == lltok::kw_prologue) {
        SEND_DATA(LLVMIRParserEventPrologue)
        this->readNextToken();
        valid = this->parseGlobalTypeAndValue(ignoreType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    if (this->_lexer.getKind() == lltok::kw_personality) {
        SEND_DATA(LLVMIRParserEventPersonality)
        this->readNextToken();
        valid = this->parseGlobalTypeAndValue(ignoreType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    END_NODE
}

bool LLVMIRParser::parseOptionalFunctionMetadata()
{
    while (this->_lexer.getKind() == lltok::MetadataVar) {
        bool valid = this->parseGlobalObjectMetadataAttachment();
        if (!valid) {
            return false;
        }
    }
    return true;
}

bool LLVMIRParser::parseGlobalObjectMetadataAttachment()
{
    START_NODE(LLVMIRParserEventGlobalObjectMetadata)
    bool valid = this->parseMetadataAttachment();
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseMetadataAttachment()
{
    if (this->_lexer.getKind() != lltok::MetadataVar) {
        this->_receiver->receiveLLVMIRError("Expected metadata attachment", this->line, this->column);
        return false;
    }
    START_NODE(LLVMIRParserEventMetadata)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoreType;
    bool valid = this->parseValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseInstructionMetada()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseArgumentList()
{
    START_NODE(LLVMIRParserEventArgumentList)
    EXPECT(lltok::lparen, "(", "expected '(' in argument list", LLVMIRParserEventPunctuation)
    auto tk = this->_lexer.getKind();
    if (tk == lltok::rparen) {
        //empty
    } else if (tk == lltok::dotdotdot) {
        SEND_STR(LLVMIRParserEventVariadic, "...")
        this->readNextToken();
    } else {
        bool done = false;
        while (!done) {
            done = true;
            //handle ... at end of arg list
            if (this->_lexer.getKind() == lltok::dotdotdot) {
                SEND_STR(LLVMIRParserEventVariadic, "...")
                this->readNextToken();
                break;
            }
            //otherwise must be an argument type
            LLVMIRParserEvent ignoreType;
            bool valid = this->parseType(ignoreType);
            if (!valid) {
                CANCEL_NODE
            }
            valid = this->parseOptionalParamAttrs();
            if (!valid) {
                CANCEL_NODE
            }

            if (this->_lexer.getKind() == lltok::LocalVar) {
                SEND_DATA(LLVMIRParserEventAttributeName)
                this->readNextToken();
            }

            if (this->_lexer.getKind() == lltok::comma) {
                SEND_STR(LLVMIRParserEventPunctuation, ",");
                this->readNextToken();
                done = false;
            }
        }
    }
    EXPECT(lltok::rparen, ")", "expected ')' at end of argument list", LLVMIRParserEventPunctuation)
    END_NODE
}

bool LLVMIRParser::parseOptionalUnnamedAddr()
{
    auto tk = this->_lexer.getKind();
    if (
        tk == lltok::kw_unnamed_addr
        || tk == lltok::kw_local_unnamed_addr
    ) {
        SEND_DATA(LLVMIRParserEventUnnamedAddr)
    }
    return true;
}

bool LLVMIRParser::parseOptionalComdat()
{
    if (this->_lexer.getKind() != lltok::kw_comdat) {
        return true;
    }
    START_NODE(LLVMIRParserEventComdat)
    SEND_DATA_
    this->readNextToken();
    if (this->_lexer.getKind() == lltok::lparen) {
        SEND_STR(LLVMIRParserEventPunctuation, ")")
        this->readNextToken();
        if (this->_lexer.getKind() == lltok::ComdatVar) {
            SEND_DATA(LLVMIRParserEventComdatVar)
            this->readNextToken();
        } else {
            ERROR("expected comdat variable")
        }
        EXPECT(lltok::rparen, ")", "expected ')' after comdat var", LLVMIRParserEventPunctuation)
    }
    END_NODE
}

bool LLVMIRParser::parseFunctionBody()
{
    START_NODE(LLVMIRParserEventFunctionBody)
    EXPECT(lltok::lbrace, "{", "expected '{' in function body", LLVMIRParserEventPunctuation)
    if (this->_lexer.getKind() == lltok::rbrace) {
        SEND_STR(LLVMIRParserEventPunctuation, "}")
        this->readNextToken();
        ERROR("function body requires at least one basic block")
    }
    auto tk = this->_lexer.getKind();
    while (
        tk != lltok::rbrace
        && tk != lltok::kw_uselistorder
    ) {
        bool valid = this->parseBasicBlock();
        if (!valid) {
            CANCEL_NODE
        }
        tk = this->_lexer.getKind();
    }
    while (this->_lexer.getKind() != lltok::rbrace) {
        this->parseUseListOrder();
    }
    EXPECT(lltok::rbrace, "}", "expected '}' at end of function body", LLVMIRParserEventPunctuation)
    END_NODE
}

bool LLVMIRParser::parseBasicBlock()
{
    START_NODE(LLVMIRParserEventBasicBlock)
    if (this->_lexer.getKind() == lltok::LabelStr || this->_lexer.getKind() == lltok::LabelID) {
        SEND_STR(LLVMIRParserEventBasicBlock, this->_lexer.getStrVal()+":")
        this->readNextToken();
        
    }
    bool done = false;
    while (!done) {
        done = true;
        if (this->_lexer.getKind() == lltok::LocalVarID) {
            SEND_DATA(LLVMIRParserEventLocalVarID)
            this->readNextToken();
            EXPECT(lltok::equal, "=", "expected '=' after instruction id", LLVMIRParserEventPunctuation)
        } else if (this->_lexer.getKind() == lltok::LocalVar) {
            SEND_DATA(LLVMIRParserEventLocalVar)
            this->readNextToken();
            EXPECT(lltok::equal, "=", "expected '=' after instruction name", LLVMIRParserEventPunctuation)
        }
        bool outAteExtraComma = false;
        bool valid = this->parseInstruction(outAteExtraComma);
        if (valid) {
            done = false;
        } else {
            CANCEL_NODE
        }
        if (outAteExtraComma) {
            // If the instruction parser ate an extra comma at the end of it, it
            // *must* be followed by metadata.
            valid = this->parseInstructionMetada();
            if (!valid) {
                CANCEL_NODE
            }
        } else {
            // With a normal result, we check to see if the instruction is followed by
            // a comma and metadata.
            if (this->_lexer.getKind() == lltok::comma) {
                SEND_STR(LLVMIRParserEventPunctuation, ",")
                this->readNextToken();
                
                valid = this->parseInstructionMetada();
                if (!valid) {
                    CANCEL_NODE
                }
            }
        }
        auto tk = this->_lexer.getKind();
        if (
            tk == lltok::LabelStr
            || tk == lltok::rbrace
            || tk == lltok::kw_uselistorder
        ) {
            done = true;
        }
    }
    END_NODE
}

bool LLVMIRParser::parseUseListOrder()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseRet()
{
    START_NODE(LLVMIRParserEventRet)
    SEND_STR(LLVMIRParserEventRet, "ret")
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseType(outType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseBr()
{
    START_NODE(LLVMIRParserEventBr);
    SEND_STR(LLVMIRParserEventBr, "br");
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    if (outType == LLVMIRParserEventLabelType) {
        END_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after branch condition", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after true destination", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseSwitch()
{
    START_NODE(LLVMIRParserEventSwitch)
    SEND_STR(LLVMIRParserEventSwitch, "switch");
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after switch condition", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::lsquare, "[", "expected '[' with switch table", LLVMIRParserEventPunctuation);
    while (this->_lexer.getKind() != lltok::rsquare) {
        valid = this->parseTypeAndValue(outType);
        if (!valid) {
            CANCEL_NODE
        }
        EXPECT(lltok::comma, ",", "expected ',' after switch condition", LLVMIRParserEventPunctuation)
        valid = this->parseTypeAndValue(outType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    SEND_STR(LLVMIRParserEventPunctuation, "]");
    this->readNextToken();
    END_NODE
}

bool LLVMIRParser::parseIndirectBr()
{
    START_NODE(LLVMIRParserEventIndirectBr)
    SEND_STR(LLVMIRParserEventIndirectBr, "indirectbr");
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after indirectbr address", LLVMIRParserEventPunctuation)
    
    EXPECT(lltok::lsquare, "[", "expected '[' with indirectbr", LLVMIRParserEventPunctuation);
    
    bool done = false;
    while (!done) {
        done = true;
        this->parseTypeAndValue(outType);
        if (this->_lexer.getKind() == lltok::comma) {
            SEND_STR(LLVMIRParserEventPunctuation, ",");
            this->readNextToken();
            done = false;
        }
    }
    
    EXPECT(lltok::rsquare, "]", "expected ']' at end of block list", LLVMIRParserEventPunctuation);
    END_NODE
}

bool LLVMIRParser::parseInvoke()
{
    START_NODE(LLVMIRParserEventInvoke)
    SEND_STR(LLVMIRParserEventInvoke, "invoke");
    this->readNextToken();
    bool valid = this->parseOptionalCallingConv();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalReturnAttrs();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalAddrSpace();
    if (!valid) {
        CANCEL_NODE
    }
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    valid = this->parseType(outType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseParameterList();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseFnAttributeValuePairs(false);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalOperandBundles();
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::kw_to, "to", "expected 'to' in invoke", LLVMIRParserEventKeyword)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::kw_unwind, "unwind", "expected 'unwind' in invoke", LLVMIRParserEventKeyword)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseOptionalLinkage()
{
    bool outHasLinkage;
    bool valid = this->parseOptionalLinkageAux(outHasLinkage);
    if (!valid) {
        return false;
    }
    if (outHasLinkage) {
        this->readNextToken();
    }
    valid = this->parseOptionalDSOLocal();
    if (!valid) {
        return false;
    }
    valid = this->parseOptionalDLLStorageClass();
    if (!valid) {
        return false;
    }
    return true;
}

bool LLVMIRParser::parseOptionalLinkageAux(bool & outHasLinkage)
{
    auto tk = this->_lexer.getKind();
    if (
        tk == lltok::kw_private
        || tk == lltok::kw_internal
        || tk == lltok::kw_weak
        || tk == lltok::kw_weak_odr
        || tk == lltok::kw_linkonce
        || tk == lltok::kw_linkonce_odr
        || tk == lltok::kw_available_externally
        || tk == lltok::kw_appending
        || tk == lltok::kw_common
        || tk == lltok::kw_extern_weak
        || tk == lltok::kw_external
    ) {
        outHasLinkage = true;
    } else {
        outHasLinkage = false;
    }
    if (outHasLinkage){
        SEND_DATA(LLVMIRParserEventLinkage)
    }
    return true;
}

bool LLVMIRParser::parseOptionalDSOLocal()
{
    switch (this->_lexer.getKind()) {
        case lltok::kw_dso_local:
        {
            SEND_DATA(LLVMIRParserEventDSOLocal)
            this->readNextToken();
            break;
        }
        case lltok::kw_dso_preemptable:
        {
            SEND_DATA(LLVMIRParserEventDSOPreemptable)
            this->readNextToken();
            break;
        }
        default:
            break;
    }
    return true;
}

bool LLVMIRParser::parseOptionalDLLStorageClass()
{
    switch (this->_lexer.getKind()) {
        case lltok::kw_dllimport:
        {
            SEND_DATA(LLVMIRParserEventDLLImport)
            this->readNextToken();
            break;
        }
        case lltok::kw_dllexport:
        {
            SEND_DATA(LLVMIRParserEventDLLExport)
            this->readNextToken();
            break;
        }
        default:
            break;
    }
    return true;
}

bool LLVMIRParser::parseOptionalCallingConv()
{
    START_NODE(LLVMIRParserEventCallingConv)
    switch (this->_lexer.getKind()) {
        case lltok::kw_ccc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "ccc")
            break;
        }
        case lltok::kw_fastcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "fastcc")
            break;
        }
        case lltok::kw_coldcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "coldcc")
            break;
        }
        case lltok::kw_x86_stdcallcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "x86_stdcallcc")
            break;
        }
        case lltok::kw_x86_fastcallcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "x86_fastcallc")
            break;
        }
        case lltok::kw_x86_regcallcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "x86_regcallcc")
            break;
        }
        case lltok::kw_x86_thiscallcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "x86_thiscallc")
            break;
        }
        case lltok::kw_x86_vectorcallcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "x86_vectorcallcc")
            break;
        }
        case lltok::kw_arm_apcscc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "arm_apcscc")
            break;
        }
        case lltok::kw_arm_aapcscc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "arm_aapcscc")
            break;
        }
        case lltok::kw_arm_aapcs_vfpcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "arm_aapcs_vfpcc")
            break;
        }
        case lltok::kw_aarch64_vector_pcs:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "aarch64_vector_pcs")
            break;
        }
        case lltok::kw_msp430_intrcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "msp430_intrcc")
            break;
        }
        case lltok::kw_avr_intrcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "avr_intrcc")
            break;
        }
        case lltok::kw_avr_signalcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "avr_signalcc")
            break;
        }
        case lltok::kw_ptx_kernel:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "ptx_kernel")
            break;
        }
        case lltok::kw_ptx_device:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "ptx_device")
            break;
        }
        case lltok::kw_spir_kernel:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "spir_kernel")
            break;
        }
        case lltok::kw_spir_func:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "spir_func")
            break;
        }
        case lltok::kw_intel_ocl_bicc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "intel_ocl_bic")
            break;
        }
        case lltok::kw_x86_64_sysvcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "x86_64_sysvcc")
            break;
        }
        case lltok::kw_win64cc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "win64cc")
            break;
        }
        case lltok::kw_webkit_jscc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "webkit_jscc")
            break;
        }
        case lltok::kw_anyregcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "anyregcc")
            break;
        }
        case lltok::kw_preserve_mostcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "preserve_mostcc")
            break;
        }
        case lltok::kw_preserve_allcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "preserve_allc")
            break;
        }
        case lltok::kw_ghccc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "ghccc")
            break;
        }
        case lltok::kw_swiftcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "swiftcc")
            break;
        }
        case lltok::kw_x86_intrcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "x86_intrcc")
            break;
        }
        case lltok::kw_hhvmcc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "hhvmcc")
            break;
        }
        case lltok::kw_hhvm_ccc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "hhvm_ccc")
            break;
        }
        case lltok::kw_cxx_fast_tlscc:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "cxx_fast_tlsc")
            break;
        }
        case lltok::kw_amdgpu_vs:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_vs")
            break;
        }
        case lltok::kw_amdgpu_ls:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_ls")
            break;
        }
        case lltok::kw_amdgpu_hs:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_hs")
            break;
        }
        case lltok::kw_amdgpu_es:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_es")
            break;
        }
        case lltok::kw_amdgpu_gs:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_gs")
            break;
        }
        case lltok::kw_amdgpu_ps:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_ps")
            break;
        }
        case lltok::kw_amdgpu_cs:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_cs")
            break;
        }
        case lltok::kw_amdgpu_kernel:
        {
            SEND_STR(LLVMIRParserEventCallingConv, "amdgpu_kernel")
            break;
        }
        case lltok::kw_cc: {
            this->_lexer.Lex();
            LLVMIRParserEvent outType = LLVMIRParserEventNone;
            this->parseValue(outType);
            break;
        }
        default:
        {
            this->_receiver->receiveLLVMIRNodeEnd(__parserEvent);
            return true;
        }
    }
    this->readNextToken();
    END_NODE
}

bool LLVMIRParser::parseOptionalReturnAttrs()
{
    bool done = false;
    while (!done) {
        done = true;
        switch (this->_lexer.getKind()) {
            case lltok::StringConstant:
            {
                bool valid = this->parseStringAttribute();
                if (!valid) {
                    return false;
                } else {
                    done = false;
                }
                break;
            }
            case lltok::kw_dereferenceable:
            case lltok::kw_dereferenceable_or_null:
            {
                bool valid = this->parseOptionalDerefAttrBytes();
                if (!valid) {
                    return false;
                } else {
                    done = false;
                }
                break;
            }
            case lltok::kw_align:
            {
                bool valid = this->parseOptionalAlignment();
                if (!valid) {
                    return false;
                } else {
                    done = false;
                }
                break;
            }
            case lltok::kw_inreg:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventReturnAttr, "inreg");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_noalias:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventReturnAttr, "noalias");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_nonnull:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventReturnAttr, "nonnull");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_signext:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventReturnAttr, "signext");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_zeroext:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventReturnAttr, "zeroext");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_byval:
            case lltok::kw_inalloca:
            case lltok::kw_nest:
            case lltok::kw_nocapture:
            case lltok::kw_returned:
            case lltok::kw_sret:
            case lltok::kw_swifterror:
            case lltok::kw_swiftself:
            case lltok::kw_immarg:
            {
                INVALID("invalid use of parameter-only attribute")
                return false;
            }
            case lltok::kw_alignstack:
            case lltok::kw_alwaysinline:
            case lltok::kw_argmemonly:
            case lltok::kw_builtin:
            case lltok::kw_cold:
            case lltok::kw_inlinehint:
            case lltok::kw_jumptable:
            case lltok::kw_minsize:
            case lltok::kw_naked:
            case lltok::kw_nobuiltin:
            case lltok::kw_noduplicate:
            case lltok::kw_noimplicitfloat:
            case lltok::kw_noinline:
            case lltok::kw_nonlazybind:
            case lltok::kw_noredzone:
            case lltok::kw_noreturn:
            case lltok::kw_nocf_check:
            case lltok::kw_nounwind:
            case lltok::kw_optforfuzzing:
            case lltok::kw_optnone:
            case lltok::kw_optsize:
            case lltok::kw_returns_twice:
            case lltok::kw_sanitize_address:
            case lltok::kw_sanitize_hwaddress:
            case lltok::kw_sanitize_memtag:
            case lltok::kw_sanitize_memory:
            case lltok::kw_sanitize_thread:
            case lltok::kw_speculative_load_hardening:
            case lltok::kw_ssp:
            case lltok::kw_sspreq:
            case lltok::kw_sspstrong:
            case lltok::kw_safestack:
            case lltok::kw_shadowcallstack:
            case lltok::kw_strictfp:
            case lltok::kw_uwtable:
            {
                INVALID("invalid use of function-only attribute")
                return false;
            }
            case lltok::kw_readnone:
            case lltok::kw_readonly:
            {
                INVALID("invalid use of attribute on return type")
                return false;
            }
            //not an attr
            default:
                break;
        }
    }
    return true;
}

bool LLVMIRParser::parseStringAttribute()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseOptionalDerefAttrBytes()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseOptionalAlignment()
{
    if (this->_lexer.getKind() != lltok::kw_align) {
        return true;
    }
    START_NODE(LLVMIRParserEventAlign)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoredType;
    bool valid = this->parseValue(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseOptionalAddrSpace()
{
    if (this->_lexer.getKind() != lltok::kw_addrspace) {
        return true;
    }
    START_NODE(LLVMIRParserEventAddrSpace)
    SEND_DATA(LLVMIRParserEventAddrSpace);
    this->readNextToken();
    EXPECT(lltok::lparen, "(", "expected '(' in address space", LLVMIRParserEventPunctuation)
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    this->parseValue(outType);
    EXPECT(lltok::rparen, ")", "expected ')' in address space", LLVMIRParserEventPunctuation)
    END_NODE
}

bool LLVMIRParser::parseParameterList()
{
    START_NODE(LLVMIRParserEventParameterList);
    EXPECT(lltok::lparen, "(", "expected '(' in call", LLVMIRParserEventPunctuation);
    bool done = false;
    while (!done) {
        done = true;
        if (this->_lexer.getKind() == lltok::dotdotdot) {
            std::string errorMsg = "unexpected ellipsis in argument list for ";
            if (this->isMustTailCall()) {
                ERROR((errorMsg+"non-musttail call").data())
            } else {
                ERROR((errorMsg+"musttail call in non-varargs function").data())
            }
            EXPECT(lltok::rparen, ")", "expected ')' at end of argument list", LLVMIRParserEventPunctuation);
            END_NODE
        }
        LLVMIRParserEvent outType = LLVMIRParserEventNone;
        bool valid = this->parseType(outType);
        if (!valid) {
            CANCEL_NODE
        }
        if (outType == LLVMIRParserEventMetadataType) {
            valid = this->parseMetadataAsValue();
        } else {
            valid = this->parseOptionalParamAttrs();
            if (!valid) {
                CANCEL_NODE
            }
            valid = this->parseValue(outType);
        }
        if (!valid) {
            CANCEL_NODE
        }
        if (this->_lexer.getKind() == lltok::comma) {
            SEND_STR(LLVMIRParserEventPunctuation, ",")
            this->readNextToken();
            done = false;
        }
    }
    EXPECT(lltok::rparen, ")", "expected ')' in call", LLVMIRParserEventPunctuation)
    END_NODE
}

bool LLVMIRParser::parseFnAttributeValuePairs(bool inAttrGrp)
{
    START_NODE(LLVMIRParserEventFnAttribute)
    bool done = false;
    while (!done) {
        done = true;
        switch (this->_lexer.getKind()) {
            case lltok::AttrGrpID:
            {
                SEND_DATA(LLVMIRParserEventAttrGrpID)
                break;
            }
            case lltok::StringConstant:
            {
                bool valid = this->parseStringAttribute();
                if (!valid) {
                    CANCEL_NODE
                }
                done = false;
                break;
            }
            case lltok::kw_align:
            {
                SEND_DATA(LLVMIRParserEventAlign)
                if (inAttrGrp) {
                    this->readNextToken();
                    EXPECT(lltok::equal, "=", "expected '=' here", LLVMIRParserEventPunctuation);
                    LLVMIRParserEvent outType = LLVMIRParserEventNone;
                    bool valid = this->parseValue(outType);
                    if (!valid) {
                        CANCEL_NODE
                    }
                } else {
                    this->parseOptionalAlignment();
                }
                done = false;
                break;
            }
            case lltok::kw_alignstack:
            {
                SEND_DATA(LLVMIRParserEventAlignStack)
                EXPECT(lltok::equal, "=", "expected '=' here", LLVMIRParserEventPunctuation)
                LLVMIRParserEvent outType = LLVMIRParserEventNone;
                bool valid = this->parseValue(outType);
                if (!valid) {
                    CANCEL_NODE
                }
                break;
            }
            case lltok::kw_allocsize:
            {
                SEND_DATA(LLVMIRParserEventAllocSize)
                bool valid = this->parseAllocSizeArguments();
                if (!valid) {
                    CANCEL_NODE
                }
                break;
            }
                
            default:
                break;
        }
    }
    END_NODE
}

bool LLVMIRParser::parseOptionalOperandBundles()
{
    if (this->_lexer.getKind() != lltok::lsquare) {
        return true;
    }
    START_NODE(LLVMIRParserEventOperandBundle)
    SEND_DATA_
    this->readNextToken();

    bool done = false;
    while (!done) {
        done = true;

        LLVMIRParserEvent outType = LLVMIRParserEventNone;
        bool valid = this->parseValue(outType);
        if (!valid) {
            CANCEL_NODE
        }
        EXPECT(lltok::lparen, "(", "expected '(' in operand bundle", LLVMIRParserEventPunctuation)
        bool done2 = false;
        while (!done2) {
            done2 = true;
            LLVMIRParserEvent outType = LLVMIRParserEventNone;
            this->parseType(outType);
            this->parseValue(outType);
            
            if (this->_lexer.getKind() == lltok::comma) {
                done2 = false;
            }
        }
        EXPECT(lltok::rparen, ")", "expected ')' in operand bundle", LLVMIRParserEventPunctuation)
        if (this->_lexer.getKind() == lltok::comma) {
            done = false;
        }
    }
    EXPECT(lltok::rsquare, "]", "expected ']' in operand bundle", LLVMIRParserEventPunctuation)
    END_NODE
}

bool LLVMIRParser::parseMetadataAsValue()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseOptionalParamAttrs()
{
    bool done = false;
    bool valid = true;
    while (!done) {
        done = true;
        switch (this->_lexer.getKind()) {
            case lltok::StringConstant:
            {
                valid = this->parseStringAttribute();
                if (!valid) {
                    return false;
                }
                done = false;
                break;
            }
            case lltok::kw_align:
            {
                valid = this->parseOptionalAlignment();
                if (!valid) {
                    return false;
                }
                done = false;
                break;
            }
            case lltok::kw_byval:
            {
                valid = this->parseByValWithOptionalType();
                if (!valid) {
                    return false;
                }
                done = false;
                break;
            }
            case lltok::kw_dereferenceable:
            case lltok::kw_dereferenceable_or_null:
            {
                valid = this->parseOptionalDerefAttrBytes();
                if (!valid) {
                    return false;
                }
                done = false;
                break;
            }
            case lltok::kw_inalloca:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_inreg:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_nest:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_noalias:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_nocapture:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_nonnull:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_readnone:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_readonly:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_returned:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_signext:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_sret:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_swifterror:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_swiftself:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_writeonly:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_zeroext:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
            case lltok::kw_immarg:
            {
                this->_receiver->receiveLLVMIRData(LLVMIRParserEventParamAttr, this->_lexer.getStrVal());
                break;
            }
                
            case lltok::kw_alignstack:
            case lltok::kw_alwaysinline:
            case lltok::kw_argmemonly:
            case lltok::kw_builtin:
            case lltok::kw_inlinehint:
            case lltok::kw_jumptable:
            case lltok::kw_minsize:
            case lltok::kw_naked:
            case lltok::kw_nobuiltin:
            case lltok::kw_noduplicate:
            case lltok::kw_noimplicitfloat:
            case lltok::kw_noinline:
            case lltok::kw_nonlazybind:
            case lltok::kw_noredzone:
            case lltok::kw_noreturn:
            case lltok::kw_nocf_check:
            case lltok::kw_nounwind:
            case lltok::kw_optforfuzzing:
            case lltok::kw_optnone:
            case lltok::kw_optsize:
            case lltok::kw_returns_twice:
            case lltok::kw_sanitize_address:
            case lltok::kw_sanitize_hwaddress:
            case lltok::kw_sanitize_memtag:
            case lltok::kw_sanitize_memory:
            case lltok::kw_sanitize_thread:
            case lltok::kw_speculative_load_hardening:
            case lltok::kw_ssp:
            case lltok::kw_sspreq:
            case lltok::kw_sspstrong:
            case lltok::kw_safestack:
            case lltok::kw_shadowcallstack:
            case lltok::kw_strictfp:
            case lltok::kw_uwtable:
            {
                this->_receiver->receiveLLVMIRError("invalid use of function-only attribute", this->line, this->column);
                break;
            }

            default:
                break;
        }
    }
    return true;
}

bool LLVMIRParser::parseByValWithOptionalType()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseAllocSizeArguments()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseResume()
{
    START_NODE(LLVMIRParserEventResume)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseCleanupRet()
{
    START_NODE(LLVMIRParserEventCatchRet)
    SEND_DATA_
    this->readNextToken();
    EXPECT(lltok::kw_from, "from", "expected 'from' after cleanupret",LLVMIRParserEventKeyword)
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::kw_unwind, "unwind", "expected 'unwind' in cleanupret",LLVMIRParserEventKeyword)
    if (this->_lexer.getKind() == lltok::kw_to) {
        SEND_STR(LLVMIRParserEventKeyword, "to")
        this->readNextToken();
        EXPECT(lltok::kw_caller, "caller", "expected 'caller' in cleanupret", LLVMIRParserEventKeyword)
    } else {
        bool valid = this->parseTypeAndValue(outType);
        if (!valid) {
            CANCEL_NODE
        }
    }
    END_NODE
}

bool LLVMIRParser::parseCatchRet()
{
    START_NODE(LLVMIRParserEventCatchRet)
    SEND_DATA_
    this->readNextToken();
    EXPECT(lltok::kw_from, "from", "expected 'from' after catchret", LLVMIRParserEventKeyword)
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::kw_to, "to", "expected 'to' in catchret", LLVMIRParserEventKeyword)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseCatchSwitch()
{
    START_NODE(LLVMIRParserEventCatchSwitch)
    SEND_DATA_
    this->readNextToken();
    EXPECT(lltok::kw_within, "within", "expected 'within' after catchswitch", LLVMIRParserEventKeyword)
    auto tokKind = this->_lexer.getKind();
    if (tokKind != lltok::kw_none && tokKind != lltok::LocalVar &&
        tokKind != lltok::LocalVarID){
        ERROR("expected scope value for catchswitch")
    }
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::lsquare, "[", "expected '[' with catchswitch labels", LLVMIRParserEventPunctuation)
    bool done = false;
    while (!done) {
        done = true;
        valid = this->parseTypeAndValue(outType);
        if (!valid) {
            CANCEL_NODE
        }
        if (this->_lexer.getKind() == lltok::comma) {
            SEND_STR(LLVMIRParserEventPunctuation, ",")
            this->readNextToken();
            done = false;
        }
    }
    EXPECT(lltok::rsquare, "]", "expected ']' after catchswitch labels", LLVMIRParserEventPunctuation)
    EXPECT(lltok::kw_unwind, "unwind", "expected 'unwind' after catchswitch scope", LLVMIRParserEventKeyword)
    if (this->_lexer.getKind() == lltok::kw_to) {
        SEND_DATA(LLVMIRParserEventKeyword)
        this->readNextToken();
        EXPECT(lltok::kw_caller, "caller", "expected 'caller' in catchswitch", LLVMIRParserEventKeyword)
        
    } else {
        valid = this->parseTypeAndValue(outType);
    }
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseCatchPad()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseCleanupPad()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

bool LLVMIRParser::parseCallBr()
{
    START_NODE(LLVMIRParserEventCallBr)
    SEND_DATA(LLVMIRParserEventCallBr)
    this->readNextToken();
    bool valid = this->parseOptionalCallingConv();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalReturnAttrs();
    if (!valid) {
        CANCEL_NODE
    }
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    valid = this->parseType(outType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = parseParameterList();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseFnAttributeValuePairs(false);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalOperandBundles();
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::kw_to, "to", "expected 'to' in callbr", LLVMIRParserEventKeyword)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }

    EXPECT(lltok::lsquare, "[", "expected '[' in callbr", LLVMIRParserEventPunctuation)
    bool done = false;
    while (!done) {
        done = true;
        valid = this->parseTypeAndValue(outType);
        if (!valid) {
            CANCEL_NODE
        }
        if (this->_lexer.getKind() == lltok::comma) {
            SEND_STR(LLVMIRParserEventPunctuation, ",")
            done = false;
        }
    }
    EXPECT(lltok::rsquare, "]", "expected ']' at end of block list", LLVMIRParserEventPunctuation)

    END_NODE
}

bool LLVMIRParser::parseFneg()
{
    START_NODE(LLVMIRParserEventFneg)
    SEND_DATA_
    this->readNextToken();
    bool valid = this->parseOptionalFastMathFlags();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseUnaryOp(true);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseCast()
{
    START_NODE(LLVMIRParserEventCast)
    switch (this->_lexer.getKind()) {
        case lltok::kw_trunc: SEND_STR(LLVMIRParserEventCast, "trunc"); break;
        case lltok::kw_zext: SEND_STR(LLVMIRParserEventCast, "zext"); break;
        case lltok::kw_sext: SEND_STR(LLVMIRParserEventCast, "sext"); break;
        case lltok::kw_fptrunc: SEND_STR(LLVMIRParserEventCast, "fptrunc"); break;
        case lltok::kw_fpext: SEND_STR(LLVMIRParserEventCast, "fpext"); break;
        case lltok::kw_bitcast: SEND_STR(LLVMIRParserEventCast, "bitcast"); break;
        case lltok::kw_addrspacecast: SEND_STR(LLVMIRParserEventCast, "addrspacecast"); break;
        case lltok::kw_uitofp: SEND_STR(LLVMIRParserEventCast, "uitofp"); break;
        case lltok::kw_sitofp: SEND_STR(LLVMIRParserEventCast, "sitofp"); break;
        case lltok::kw_fptoui: SEND_STR(LLVMIRParserEventCast, "fptoui"); break;
        case lltok::kw_fptosi: SEND_STR(LLVMIRParserEventCast, "fptosi"); break;
        case lltok::kw_inttoptr: SEND_STR(LLVMIRParserEventCast, "inttoptr"); break;
        case lltok::kw_ptrtoint: SEND_STR(LLVMIRParserEventCast, "ptrtoint"); break;
        default: CANCEL_NODE
    }
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::kw_to, "to", "expected 'to' after cast value", LLVMIRParserEventKeyword);
    valid = this->parseType(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseLogical()
{
    START_NODE(LLVMIRParserEventLogical)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' in logical operation", LLVMIRParserEventPunctuation)
    valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseCompare()
{
    START_NODE(LLVMIRParserEventCompare)
    bool valid = this->parseCmpPredicate();
    if (!valid) {
        CANCEL_NODE
    }
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after compare value", LLVMIRParserEventPunctuation)

    valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseCmpPredicate()
{
    auto tokenKind = this->_lexer.getKind();
    if (tokenKind == lltok::kw_icmp) {
        this->_receiver->receiveLLVMIRData(LLVMIRParserEventCompare, "icmp");
    } else if (tokenKind == lltok::kw_fcmp) {
        this->_receiver->receiveLLVMIRData(LLVMIRParserEventCompare, "fcmp");
    } else {
        this->_receiver->receiveLLVMIRError("expected comparison keyword", this->line, this->column);
        return false;
    }
    auto opcode = this->_lexer.getUIntVal();

    this->readNextToken();

    //FIXME: should this 54 value come from llvm's instruction.h?
    if (opcode == 54) {
        switch (this->_lexer.getKind()) {
            case lltok::kw_oeq: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "oeq"); break;
            case lltok::kw_one: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "one"); break;
            case lltok::kw_olt: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "olt"); break;
            case lltok::kw_ogt: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ogt"); break;
            case lltok::kw_ole: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ole"); break;
            case lltok::kw_oge: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "oge"); break;
            case lltok::kw_ord: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ord"); break;
            case lltok::kw_uno: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "uno"); break;
            case lltok::kw_ueq: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ueq"); break;
            case lltok::kw_une: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "une"); break;
            case lltok::kw_ult: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ult"); break;
            case lltok::kw_ugt: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ugt"); break;
            case lltok::kw_ule: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ule"); break;
            case lltok::kw_uge: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "uge"); break;
            case lltok::kw_true: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "true"); break;
            case lltok::kw_false: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "false"); break;
            default:
                this->_receiver->receiveLLVMIRError("expected fcmp predicate (e.g. 'oeq')", this->line, this->column);
                return false;
        }
    } else {
        switch (this->_lexer.getKind()) {
            case lltok::kw_eq:  this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "eq"); break;
            case lltok::kw_ne:  this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ne"); break;
            case lltok::kw_slt: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "slt"); break;
            case lltok::kw_sgt: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "sgt"); break;
            case lltok::kw_sle: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "sle"); break;
            case lltok::kw_sge: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "sge"); break;
            case lltok::kw_ult: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ult"); break;
            case lltok::kw_ugt: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ugt"); break;
            case lltok::kw_ule: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "ule"); break;
            case lltok::kw_uge: this->_receiver->receiveLLVMIRData(LLVMIRParserEventCmpPredicate, "uge"); break;
            default:
                this->_receiver->receiveLLVMIRError("expected icmp predicate (e.g. 'eq')", this->line, this->column);
                return false;
        }
    }
    this->readNextToken();
    return true;
}

bool LLVMIRParser::parseTypeAndValue(LLVMIRParserEvent & outType)
{
    LLVMIRParserEvent outTypeType = LLVMIRParserEventNone;
    bool valid = this->parseType(outTypeType);
    if (!valid) {
        return false;
    }
    this->parseValue(outType);
    if (outType == LLVMIRParserEventLocalVar || outType == LLVMIRParserEventLocalVarID) {
        outType = outTypeType;
    }
    return valid;
}

bool LLVMIRParser::parseType(LLVMIRParserEvent & outType)
{
    START_NODE(LLVMIRParserEventType)
    switch (this->_lexer.getKind()) {
        // Type ::= 'float' | 'void' (etc)
        case lltok::Type:
        {
            if (this->_lexer.getStrVal() == "label") {
                outType = LLVMIRParserEventLabelType;
            } else {
                outType = LLVMIRParserEventBasicType;
            }
            SEND_DATA(outType)
            this->readNextToken();
            break;
        }
        // Type ::= StructType
        case lltok::lbrace:
        {
            bool valid = this->parseAnonStructType();
            if (!valid) {
                CANCEL_NODE
            }
            break;
        }
        // Type ::= '[' ... ']'
        case lltok::lsquare:
        {
            SEND_STR(LLVMIRParserEventPunctuation, "[")
            this->readNextToken();
            bool valid = this->parseArrayVectorType(false);
            if (!valid) {
                CANCEL_NODE
            }
            break;
        }
        // Either vector or packed struct.
        // Type ::= '<' ... '>'
        case lltok::less:
        {
            SEND_STR(LLVMIRParserEventPunctuation, "<")
            this->readNextToken();
            if (this->_lexer.getKind() == lltok::lbrace) {
                bool valid = this->parseAnonStructType();
                if (!valid) {
                    CANCEL_NODE
                }
                EXPECT(lltok::greater, ">", "expected '>' at end of packed struct", LLVMIRParserEventPunctuation)
            } else {
                bool valid = this->parseArrayVectorType(true);
                if (!valid) {
                    CANCEL_NODE
                }
            }
            break;
        }
        case lltok::LocalVar:
        {
            SEND_STR(LLVMIRParserEventLocalVarType, "%" + this->_lexer.getStrVal())
            this->readNextToken();
            break;
        }
        case lltok::LocalVarID:
        {
            SEND_STR(LLVMIRParserEventLocalVarIDType, "%" + this->_lexer.getStrVal())
            this->readNextToken();
            break;
        }

        default:
            break;
    }
    bool done = false;
    while (!done) {
        done = true;
        switch (this->_lexer.getKind()) {
            case lltok::star:
            {
                SEND_STR(LLVMIRParserEventPointerType, "*");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_addrspace:
            {
                SEND_STR(LLVMIRParserEventAddrSpace, "addrspace")
                this->readNextToken();
                EXPECT(lltok::lparen, "(", "expected '(' in address space", LLVMIRParserEventPunctuation)
                LLVMIRParserEvent outType = LLVMIRParserEventNone;
                this->parseValue(outType);
                EXPECT(lltok::rparen, ")", "expected ')' in address space", LLVMIRParserEventPunctuation)
                EXPECT(lltok::star, "*", "expected '*' in address space", LLVMIRParserEventPunctuation)
                done = false;
                break;
            }
            case lltok::lparen:
            {
                std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
                CANCEL_NODE
            }

            default:
                break;
        }
    }

    END_NODE
}

bool LLVMIRParser::parseAnonStructType()
{
    START_NODE(LLVMIRParserEventStructType)
    this->parseStructBody();
    END_NODE
}

bool LLVMIRParser::parseStructBody()
{
    START_NODE(LLVMIRParserEventStructBody)
    EXPECT(lltok::lbrace, "{", "expected '{' in struct body", LLVMIRParserEventPunctuation)
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseType(outType);
    if (!valid) {
        CANCEL_NODE
    }
    bool done = false;
    while (!done) {
        done = true;
        valid = this->parseType(outType);
        if (!valid) {
            CANCEL_NODE
        }
        if (this->_lexer.getKind() == lltok::comma) {
            SEND_STR(LLVMIRParserEventPunctuation, ",")
            this->readNextToken();
            done = false;
        }
    }
    EXPECT(lltok::rbrace, "}", "expected '}' at end of struct", LLVMIRParserEventPunctuation)
    END_NODE
}

bool LLVMIRParser::parseArrayVectorType(bool isVector)
{
    size_t __startCol = this->column;
    size_t __startLine = this->line;
    size_t __startIndex = this->index;
    LLVMIRParserEvent __parserEvent = LLVMIRParserEventNone;
    if (isVector) {
        __parserEvent = LLVMIRParserEventVectorType;
    } else {
        __parserEvent = LLVMIRParserEventArrayType;
    }
    if (isVector && this->_lexer.getKind() == lltok::kw_vscale) {
        SEND_STR(LLVMIRParserEventVscale, "vscale")
        EXPECT(lltok::kw_x, "x", "expected 'x' after vscale", LLVMIRParserEventKeyword)
    }
    if (
        this->_lexer.getKind() != lltok::APSInt
        || this->_lexer.getAPSIntVal().isSigned()
        || this->_lexer.getAPSIntVal().getBitWidth() > 64)
    {
        ERROR("expected number in address space")
    }

    uint64_t size = this->_lexer.getAPSIntVal().getZExtValue();
    SEND_DATA_
    this->readNextToken();

    EXPECT(lltok::kw_x, "x", "expected 'x' after element count", LLVMIRParserEventKeyword)

    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseType(outType);
    if (!valid) {
        CANCEL_NODE
    }

    if (isVector) {
        EXPECT(lltok::greater, ">", "expected end of sequential type", LLVMIRParserEventPunctuation)
        
    } else {
        EXPECT(lltok::rsquare, "]", "expected end of sequential type", LLVMIRParserEventPunctuation)
    }
    if (isVector) {
        if (size == 0) {
            ERROR("zero element vector is illegal")
        }
        if ((unsigned)size != size) {
            ERROR("size too large for vector")
        }
    }
    END_NODE
}

bool LLVMIRParser::parseValue(LLVMIRParserEvent & outType)
{
    switch (this->_lexer.getKind()) {
        case lltok::GlobalID:
        {
            START_NODE(LLVMIRParserEventGlobalID)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::GlobalVar:
        {
            START_NODE(LLVMIRParserEventGlobalVar)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::LocalVarID:
        {
            START_NODE(LLVMIRParserEventLocalVarID)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::LocalVar:
        {
            START_NODE(LLVMIRParserEventLocalVar)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::APSInt:
        {
            START_NODE(LLVMIRParserEventAPSInt)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::APFloat:
        {
            START_NODE(LLVMIRParserEventAPSFloat)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::StringConstant:
        {
            START_NODE(LLVMIRParserEventStringConstant)
            outType = __parserEvent;
            SEND_STR(__parserEvent, "\"" + this->_lexer.getStrVal() + "\"")
            this->readNextToken();
            END_NODE
        }
        case lltok::kw_true:
        case lltok::kw_false:
        {
            START_NODE(LLVMIRParserEventBoolConstant)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::kw_null:
        {
            START_NODE(LLVMIRParserEventNull)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::kw_undef:
        {
            START_NODE(LLVMIRParserEventUndef)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::kw_zeroinitializer:
        {
            START_NODE(LLVMIRParserEventZero)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::kw_none:
        {
            START_NODE(LLVMIRParserEventNoneKw)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            END_NODE
        }
        case lltok::lbrace:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::less:
        {
            SEND_STR(LLVMIRParserEventPunctuation, "<")
            this->readNextToken();
            size_t __startCol = this->column;
            size_t __startLine = this->line;
            size_t __startIndex = this->index;
            LLVMIRParserEvent __parserEvent;
            bool isPackedStruct = false;
            if (this->_lexer.getKind() == lltok::lbrace) {
                __parserEvent = LLVMIRParserEventPackedStruct;
                isPackedStruct = true;
                this->readNextToken();

            } else {
                __parserEvent = LLVMIRParserEventVector;
            }
            this->_receiver->receiveLLVMIRNodeStart(__parserEvent);

            bool outIsInRange = false;
            bool valid = this->parseGlobalValueVector(false, outIsInRange);
            if (!valid) {
                CANCEL_NODE
            }
            if (isPackedStruct) {
                EXPECT(lltok::rbrace, "}", "expected end of packed struct", LLVMIRParserEventPunctuation)
            }
            EXPECT(lltok::greater, ">", "expected end of constant", LLVMIRParserEventPunctuation)
            END_NODE
        }
        case lltok::lsquare:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_c:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_asm:
        {
            START_NODE(LLVMIRParserEventAsm)
            outType = __parserEvent;
            SEND_DATA_
            this->readNextToken();
            OPTIONAL(lltok::kw_sideeffect, LLVMIRParserEventSideEffect)
            OPTIONAL(lltok::kw_alignstack, LLVMIRParserEventAlignStack)
            OPTIONAL(lltok::kw_inteldialect, LLVMIRParserEventIntelDialect)
            LLVMIRParserEvent localOutType = LLVMIRParserEventNone;
            this->parseValue(localOutType);
            EXPECT(lltok::comma, ",", "expected comma in inline asm expression", LLVMIRParserEventPunctuation)
            this->parseValue(localOutType);
            END_NODE
        }
        case lltok::kw_blockaddress:
        {
            START_NODE(LLVMIRParserEventBlockAddress)
            outType = __parserEvent;
            SEND_DATA_;
            this->readNextToken();
            EXPECT(lltok::lparen, "(", "expected '(' in block address expression", LLVMIRParserEventPunctuation)
            LLVMIRParserEvent localOutType = LLVMIRParserEventNone;
            this->parseValue(localOutType);
            EXPECT(lltok::comma, ",", "expected comma in block address expression", LLVMIRParserEventPunctuation)
            this->parseValue(localOutType);
            EXPECT(lltok::rparen, ")", "expected ')' in block address expression", LLVMIRParserEventPunctuation)
            END_NODE
        }
        case lltok::kw_trunc:
        case lltok::kw_zext:
        case lltok::kw_sext:
        case lltok::kw_fptrunc:
        case lltok::kw_fpext:
        case lltok::kw_bitcast:
        case lltok::kw_addrspacecast:
        case lltok::kw_uitofp:
        case lltok::kw_sitofp:
        case lltok::kw_fptoui:
        case lltok::kw_fptosi:
        case lltok::kw_inttoptr:
        case lltok::kw_ptrtoint:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_extractvalue:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_insertvalue:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_icmp:
        case lltok::kw_fcmp:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_fneg:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_add:
        case lltok::kw_sub:
        case lltok::kw_mul:
        case lltok::kw_shl:
        {
            return this->parseBinaryOp(false, true, false);
        }
        case lltok::kw_fadd:
        case lltok::kw_fsub:
        case lltok::kw_fmul:
        case lltok::kw_fdiv:
        case lltok::kw_frem:
        {
            return this->parseBinaryOp(true, false, false);
        }
        case lltok::kw_sdiv:
        case lltok::kw_udiv:
        case lltok::kw_lshr:
        case lltok::kw_ashr:
        {
            return this->parseBinaryOp(true, false, true);
        }
        case lltok::kw_urem:
        case lltok::kw_srem:
        {
            return this->parseArithmetic();
        }
        case lltok::kw_and:
        case lltok::kw_or:
        case lltok::kw_xor:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        case lltok::kw_getelementptr:
        case lltok::kw_shufflevector:
        case lltok::kw_insertelement:
        case lltok::kw_extractelement:
        case lltok::kw_select:
        {
            std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
            return false;
        }
        default:
            this->_receiver->receiveLLVMIRError("unknown token type while parsing value", this->line, this->column);
            return false;
    }
    return false;
}

bool LLVMIRParser::parseSelect()
{
    START_NODE(LLVMIRParserEventSelect);
    SEND_STR(LLVMIRParserEventSelect, "select")
    this->readNextToken();
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after select condition", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after select value", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseOptionalFastMathFlags()
{
    bool done = false;
    while (!done) {
        done = true;
        switch (this->_lexer.getKind()) {
            case lltok::kw_fast:
            {
                SEND_STR(LLVMIRParserEventFast, "fast");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_nnan:
            {
                SEND_STR(LLVMIRParserEventNnan, "nnan");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_ninf:
            {
                SEND_STR(LLVMIRParserEventNinf, "ninf");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_nsz:
            {
                SEND_STR(LLVMIRParserEventNsz, "nsz");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_arcp:
            {
                SEND_STR(LLVMIRParserEventArcp, "arcp");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_contract:
            {
                SEND_STR(LLVMIRParserEventContract, "contract");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_reassoc:
            {
                SEND_STR(LLVMIRParserEventReassoc, "reassoc");
                this->readNextToken();
                done = false;
                break;
            }
            case lltok::kw_afn:
            {
                SEND_STR(LLVMIRParserEventAfn, "afn");
                this->readNextToken();
                done = false;
                break;
            }

            default:
                break;
        }
    }
    return true;
}

bool LLVMIRParser::parseUnaryOp(bool isFP)
{
    START_NODE(LLVMIRParserEventUnaryOp)
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseBinaryOp(bool isFP, bool allowWrap, bool allowExact)
{
    size_t __startCol = this->column;
    size_t __startLine = this->line;
    size_t __startIndex = this->index;
    LLVMIRParserEvent __parserEvent = LLVMIRParserEventNone;
    if (isFP) {
        switch (this->_lexer.getKind()) {
            case lltok::kw_fadd:
            {
                __parserEvent = LLVMIRParserEventFAdd;
                break;
            }
            case lltok::kw_fsub:
            {
                __parserEvent = LLVMIRParserEventFSub;
                break;
            }
            case lltok::kw_fmul:
            case lltok::kw_mul:
            {
                __parserEvent = LLVMIRParserEventFMul;
                break;
            }
            case lltok::kw_fdiv:
            {
                __parserEvent = LLVMIRParserEventFDiv;
                break;
            }
            case lltok::kw_frem:
            {
                __parserEvent = LLVMIRParserEventFRem;
                break;
            }
            default:
                break;
        }
        SEND_DATA_
        this->readNextToken();
        this->parseOptionalFastMathFlags();

    } else {
        switch (this->_lexer.getKind()) {
            case lltok::kw_add:
            {
                __parserEvent = LLVMIRParserEventAdd;
                break;
            }
            case lltok::kw_sub:
            {
                __parserEvent = LLVMIRParserEventSub;
                break;
            }
            case lltok::kw_mul:
            {
                __parserEvent = LLVMIRParserEventMul;
                break;
            }
            case lltok::kw_shl:
            {
                __parserEvent = LLVMIRParserEventShl;
                break;
            }
            case lltok::kw_sdiv:
            {
                __parserEvent = LLVMIRParserEventSDiv;
                break;
            }
            case lltok::kw_udiv:
            {
                __parserEvent = LLVMIRParserEventUDiv;
                break;
            }
            case lltok::kw_lshr:
            {
                __parserEvent = LLVMIRParserEventLShr;
                break;
            }
            case lltok::kw_ashr:
            {
                __parserEvent = LLVMIRParserEventAShr;
                break;
            }
            default:
                break;
        }
        SEND_DATA_
        this->readNextToken();
        if (allowWrap) {
            if (this->_lexer.getKind() == lltok::kw_nuw) {
                SEND_STR(LLVMIRParserEventNuw, "nuw")
                this->readNextToken();
            }
            if (this->_lexer.getKind() == lltok::kw_nsw) {
                SEND_STR(LLVMIRParserEventNsw, "nsw")
                this->readNextToken();
            }
        }
        if (allowExact) {
            if (this->_lexer.getKind() == lltok::kw_exact) {
                SEND_STR(LLVMIRParserEventExact, "exact")
                this->readNextToken();
            }
        }
    }

    bool valid = this->parseArithmetic();
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseArithmetic()
{
    LLVMIRParserEvent outType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(outType);
    if (!valid) {
        return false;
    }
    if (this->_lexer.getKind() != lltok::comma) {
        this->_receiver->receiveLLVMIRError("expected ',' in arithmetic operation", this->line, this->column);
        return false;
    } else {
        SEND_STR(LLVMIRParserEventPunctuation, ",");
        this->readNextToken();
    }
    valid = this->parseValue(outType);
    if (!valid) {
        return false;
    }
    return true;
}

bool LLVMIRParser::parseGlobalValueVector(bool wantsInRange, bool & outIsInRange)
{
    //empty list
    auto tokKind = this->_lexer.getKind();
    if (
        tokKind == lltok::rbrace
        || tokKind == lltok::rsquare
        || tokKind == lltok::greater
        || tokKind == lltok::rparen
    ) {
        return false;
    }
    bool done = false;
    bool valid = true;
    while (!done) {
        done = true;
        
        if (wantsInRange && this->_lexer.getKind() == lltok::kw_inrange) {
            outIsInRange = true;
            SEND_DATA(LLVMIRParserEventInRange)
        }
        LLVMIRParserEvent outType = LLVMIRParserEventNone;
        valid = this->parseGlobalTypeAndValue(outType);
        if (!valid) {
            return false;
        }
        if (this->_lexer.getKind() == lltok::comma) {
            done = false;
            SEND_STR(LLVMIRParserEventPunctuation, ",")
            this->readNextToken();
        }
    }
    return true;
}

bool LLVMIRParser::parseGlobalTypeAndValue(LLVMIRParserEvent & outType)
{
    LLVMIRParserEvent ignoreType = LLVMIRParserEventNone;
    bool valid = this->parseType(ignoreType);
    if (!valid) {
        return false;
    }
    valid = this->parseGlobalValue(outType);
    if (!valid) {
        return false;
    }
    return true;
}

bool LLVMIRParser::parseGlobalValue(LLVMIRParserEvent & outType)
{
    START_NODE(LLVMIRParserEventGlobalValue)
    bool valid = this->parseValue(outType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseExtractElement()
{
    START_NODE(LLVMIRParserEventExtractElement)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoreType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after extract value", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseInsertElement()
{
    START_NODE(LLVMIRParserEventInsertElement)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoreType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after insertelement value", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after insertelement value", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseShuffleVector()
{
    START_NODE(LLVMIRParserEventShuffleVector)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoreType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after shuffle mask", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after shuffle value", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}
bool LLVMIRParser::parsePhi(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventPhi)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoreType;
    bool valid = this->parseType(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::lsquare, "[", "expected '[' in phi value list", LLVMIRParserEventPunctuation)
    valid = this->parseValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after insertelement value", LLVMIRParserEventPunctuation)
    valid = this->parseValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::rsquare, "]", "expected ']' in phi value list", LLVMIRParserEventPunctuation)
    
    if (this->_lexer.getKind() == lltok::comma) {
        SEND_STR(LLVMIRParserEventPunctuation, ",")
        this->readNextToken();
        
        bool done = false;
        while (!done) {
            done = true;
            if (this->_lexer.getKind() == lltok::MetadataVar) {
                outAteExtraComma = true;
                break;
            }
            
            EXPECT(lltok::lsquare, "[", "expected '[' in phi value list", LLVMIRParserEventPunctuation)
            valid = this->parseValue(ignoreType);
            if (!valid) {
                CANCEL_NODE
            }
            EXPECT(lltok::comma, ",", "expected ',' after insertelement value", LLVMIRParserEventPunctuation)
            valid = this->parseValue(ignoreType);
            if (!valid) {
                CANCEL_NODE
            }
            EXPECT(lltok::rsquare, "]", "expected ']' in phi value list", LLVMIRParserEventPunctuation)

            if (this->_lexer.getKind() == lltok::comma) {
                SEND_STR(LLVMIRParserEventPunctuation, ",")
                this->readNextToken();
                done = false;
            }
        }
    }

    END_NODE
}

bool LLVMIRParser::parseCall()
{
    START_NODE(LLVMIRParserEventCall)
    SEND_DATA_
    this->readNextToken();
    bool valid = this->parseOptionalFastMathFlags();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalCallingConv();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalReturnAttrs();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOptionalAddrSpace();
    if (!valid) {
        CANCEL_NODE
    }
    LLVMIRParserEvent ignoreType;
    valid = this->parseType(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseParameterList();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseFnAttributeValuePairs(false);
    if (!valid) {
        CANCEL_NODE
    }
    this->parseOptionalOperandBundles();
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseExtractValue(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventExtractValue)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoreType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseIndexList(outAteExtraComma);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseInsertValue(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventInsertValue)
    SEND_DATA_
    this->readNextToken();
    LLVMIRParserEvent ignoreType = LLVMIRParserEventNone;
    bool valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after shuffle value", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseIndexList(outAteExtraComma);
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseIndexList(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventIndexList)
    EXPECT(lltok::comma, ",", "expected ',' as start of index list", LLVMIRParserEventPunctuation)
    
    bool done = false;
    bool hasIndices = false;
    bool valid = false;
    while (!done) {
        done = true;
        if (this->_lexer.getKind() == lltok::MetadataVar) {
            if (!hasIndices) {
                ERROR("expected index")
            }
            outAteExtraComma = true;
            END_NODE
        }
        
        LLVMIRParserEvent outType = LLVMIRParserEventNone;
        valid = this->parseValue(outType);
        if (!valid) {
            CANCEL_NODE
        }

        if (this->_lexer.getKind() == lltok::comma) {
            SEND_STR(LLVMIRParserEventPunctuation, ",");
            this->readNextToken();
            done = false;
        }
    }
    
    END_NODE
}

bool LLVMIRParser::parseAlloca(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventAlloca)
    SEND_DATA_
    this->readNextToken();
    if (this->_lexer.getKind() == lltok::kw_inalloca) {
        SEND_STR(LLVMIRParserEventInalloca, "inalloca")
        this->readNextToken();
    }
    if (this->_lexer.getKind() == lltok::kw_swifterror) {
        SEND_STR(LLVMIRParserEventSwifterror, "swifterror")
        this->readNextToken();
    }
    LLVMIRParserEvent ignoredType;
    bool valid = this->parseType(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    if (this->_lexer.getKind() == lltok::comma) {
        SEND_STR(LLVMIRParserEventPunctuation, ",")
        this->readNextToken();
        auto tokKind = this->_lexer.getKind();
        if (tokKind == lltok::kw_align)
        {
            valid = this->parseOptionalAlignment();
            if (!valid) {
                CANCEL_NODE
            }
            valid = this->parseOptionalCommaAddrSpace(outAteExtraComma);
            if (!valid) {
                CANCEL_NODE
            }
        }
        else if (tokKind == lltok::kw_addrspace)
        {
            valid = this->parseOptionalAddrSpace();
            if (!valid) {
                CANCEL_NODE
            }
        } else if (tokKind == lltok::MetadataVar) {
            outAteExtraComma = true;
        } else {
            this->parseTypeAndValue(ignoredType);
            if (this->_lexer.getKind() == lltok::comma) {
                SEND_STR(LLVMIRParserEventPunctuation, ",")
                this->readNextToken();

                tokKind = this->_lexer.getKind();
                if (tokKind == lltok::kw_align)
                {
                    valid = this->parseOptionalAlignment();
                    if (!valid) {
                        CANCEL_NODE
                    }
                    valid = this->parseOptionalCommaAddrSpace(outAteExtraComma);
                    if (!valid) {
                        CANCEL_NODE
                    }
                } else if (tokKind == lltok::kw_addrspace) {
                    valid = this->parseOptionalAddrSpace();
                    if (!valid) {
                        CANCEL_NODE
                    }
                } else if (tokKind == lltok::MetadataVar) {
                    outAteExtraComma = true;
                }
            }
        }
    }
    END_NODE
}

bool LLVMIRParser::parseOptionalCommaAddrSpace(bool & outAteExtraComma)
{
    outAteExtraComma = false;
    bool done = false;
    while (!done) {
        done = true;
        //metadata at the end is an early exit.
        if (this->_lexer.getKind() == lltok::MetadataVar) {
            outAteExtraComma = true;
            return false;
        }
        if (this->_lexer.getKind() != lltok::kw_addrspace) {
            this->_receiver->receiveLLVMIRError("expected metadata or 'addrspace'", this->line, this->column);
            return false;
        }
        bool valid = this->parseOptionalAddrSpace();
        if (!valid) {
            return false;
        }
        if (this->_lexer.getKind() == lltok::comma) {
            done = false;
            SEND_STR(LLVMIRParserEventPunctuation, ",")
            this->readNextToken();
        }
    }
    return true;
}

bool LLVMIRParser::parseOptionalCommaAlign(bool & outAteExtraComma)
{
    outAteExtraComma = false;
    while (this->_lexer.getKind() == lltok::comma) {
        SEND_STR(LLVMIRParserEventPunctuation, ",")
        this->readNextToken();

        //metadata at the end is an early exit.
        if (this->_lexer.getKind() == lltok::MetadataVar) {
            outAteExtraComma = true;
            return false;
        }

        if (this->_lexer.getKind() != lltok::kw_align) {
            this->_receiver->receiveLLVMIRError("expected metadata or 'align'", this->line, this->column);
            return false;
        }

        bool valid = this->parseOptionalAlignment();
        if (!valid) {
            return false;
        }
    }
    return true;
}

bool LLVMIRParser::parseLoad(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventLoad)
    SEND_DATA_
    this->readNextToken();
    bool isAtomic = false;
    
    if (this->_lexer.getKind() == lltok::kw_atomic) {
        SEND_STR(LLVMIRParserEventAtomic, "atomic")
        this->readNextToken();
        isAtomic = true;
    }
    if (this->_lexer.getKind() == lltok::kw_volatile) {
        SEND_STR(LLVMIRParserEventVolatile, "volatile")
        this->readNextToken();
    }
    LLVMIRParserEvent ignoredType;
    bool valid = this->parseType(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected comma after load's type", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    if (isAtomic) {
        valid = this->parseScopeAndOrdering();
        if (!valid) {
            CANCEL_NODE
        }
    }
    valid = this->parseOptionalCommaAlign(outAteExtraComma);
    END_NODE
}

bool LLVMIRParser::parseStore(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventStore)
    SEND_DATA_
    this->readNextToken();
    bool isAtomic = false;
    
    if (this->_lexer.getKind() == lltok::kw_atomic) {
        SEND_STR(LLVMIRParserEventAtomic, "atomic")
        this->readNextToken();
        isAtomic = true;
    }
    if (this->_lexer.getKind() == lltok::kw_volatile) {
        SEND_STR(LLVMIRParserEventVolatile, "volatile")
        this->readNextToken();
    }
    LLVMIRParserEvent ignoredType;
    bool valid = this->parseTypeAndValue(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after store operand", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    if (isAtomic) {
        valid = this->parseScopeAndOrdering();
        if (!valid) {
            CANCEL_NODE
        }
    }
    valid = this->parseOptionalCommaAlign(outAteExtraComma);
    END_NODE
}

bool LLVMIRParser::parseScopeAndOrdering()
{
    bool valid = this->parseScope();
    if (!valid) {
        return false;
    }
    valid = this->parseOrdering();
    if (!valid) {
        return false;
    }
    return true;
}

bool LLVMIRParser::parseScope()
{
    if (this->_lexer.getKind() == lltok::kw_syncscope) {
        START_NODE(LLVMIRParserEventScope)
        SEND_STR(LLVMIRParserEventSyncscope, "syncscope")
        this->readNextToken();
        EXPECT(lltok::lparen, "(", "Expected '(' in syncscope", LLVMIRParserEventPunctuation);
        LLVMIRParserEvent ignoredType;
        this->parseValue(ignoredType);
        EXPECT(lltok::rparen, ")", "Expected ')' in syncscope", LLVMIRParserEventPunctuation)
        END_NODE
    }
    return true;
}

bool LLVMIRParser::parseOrdering()
{
    switch (this->_lexer.getKind()) {
        case lltok::kw_unordered:
        {
            SEND_STR(LLVMIRParserEventUnordered, "unordered")
            this->readNextToken();
            break;
        }
        case lltok::kw_monotonic:
        {
            SEND_STR(LLVMIRParserEventMonotonic, "monotonic")
            this->readNextToken();
            break;
        }
        case lltok::kw_acquire:
        {
            SEND_STR(LLVMIRParserEventAcquire, "acquire")
            this->readNextToken();
            break;
        }
        case lltok::kw_release:
        {
            SEND_STR(LLVMIRParserEventRelease, "release")
            this->readNextToken();
            break;
        }
        case lltok::kw_acq_rel:
        {
            SEND_STR(LLVMIRParserEventAcqRel, "acq_rel")
            this->readNextToken();
            break;
        }case lltok::kw_seq_cst:
        {
            SEND_STR(LLVMIRParserEventSeqCst, "seq_cst")
            this->readNextToken();
            break;
        }
        default:
            this->_receiver->receiveLLVMIRError("Expected ordering on atomic instruction", this->line, this->column);
            break;
    }
    return true;
}

bool LLVMIRParser::parseCmpXchng(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventCmpXchng)
    SEND_DATA_
    this->readNextToken();
    if (this->_lexer.getKind() == lltok::kw_weak) {
        SEND_STR(LLVMIRParserEventWeak, "weak")
        this->readNextToken();
    }
    if (this->_lexer.getKind() == lltok::kw_volatile) {
        SEND_STR(LLVMIRParserEventVolatile, "volatile")
        this->readNextToken();
    }
    LLVMIRParserEvent ignoredType;
    bool valid = this->parseTypeAndValue(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after cmpxchg address", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after cmpxchg cmp operand", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoredType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseScopeAndOrdering();
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseOrdering();
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseAtomicRMW(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventAtomicRMW)
    SEND_DATA_
    this->readNextToken();
    
    if (this->_lexer.getKind() == lltok::kw_volatile) {
        SEND_DATA(LLVMIRParserEventVolatile)
    }
    switch (this->_lexer.getKind()) {
        case lltok::kw_xchg:
        case lltok::kw_add:
        case lltok::kw_sub:
        case lltok::kw_and:
        case lltok::kw_nand:
        case lltok::kw_or:
        case lltok::kw_xor:
        case lltok::kw_max:
        case lltok::kw_min:
        case lltok::kw_umax:
        case lltok::kw_umin:
        case lltok::kw_fadd:
        case lltok::kw_fsub:
        {
            SEND_DATA(LLVMIRParserEventAtomicRMWInst)
            this->readNextToken();
            break;
        }
        default:
            ERROR("expected binary operation in atomicrmw")
    }
    LLVMIRParserEvent ignoreType;
    bool valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected ',' after atomicrmw address", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    valid = this->parseScopeAndOrdering();
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseFence(bool & outAteExtraComma)
{
    START_NODE(LLVMIRParserEventFence)
    SEND_DATA_
    this->readNextToken();
    bool valid = this->parseScopeAndOrdering();
    if (!valid) {
        CANCEL_NODE
    }
    END_NODE
}

bool LLVMIRParser::parseGetElementPtr(bool & outAteExtraComma)
{
    outAteExtraComma = false;
    START_NODE(LLVMIRParserEventGetElementPtr)
    SEND_DATA_
    this->readNextToken();
    if (this->_lexer.getKind() == lltok::kw_inbounds) {
        SEND_DATA(LLVMIRParserEventInbounds)
        this->readNextToken();
    }
    LLVMIRParserEvent ignoreType;
    bool valid = this->parseType(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    EXPECT(lltok::comma, ",", "expected comma after getelementptr's type", LLVMIRParserEventPunctuation)
    valid = this->parseTypeAndValue(ignoreType);
    if (!valid) {
        CANCEL_NODE
    }
    
    if (this->_lexer.getKind() == lltok::comma) {
        SEND_STR(LLVMIRParserEventPunctuation, ",")
        this->readNextToken();

        bool done = false;
        while (!done) {
            done = true;
            if (this->_lexer.getKind() == lltok::MetadataVar) {
                outAteExtraComma = true;
                break;
            }
            
            valid = this->parseTypeAndValue(ignoreType);
            if (!valid) {
                CANCEL_NODE
            }
            
            if (this->_lexer.getKind() == lltok::comma) {
                SEND_STR(LLVMIRParserEventPunctuation, ",")
                this->readNextToken();
                done = false;
            }
        }
    }
    
    END_NODE
}

bool LLVMIRParser::isMustTailCall()
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return false;
}

#undef START_NODE
#undef SEND_DATA_
#undef SEND_DATA
#undef SEND_STR
#undef EXPECT
#undef OPTIONAL
#undef ERROR
#undef CANCEL_NODE
#undef END_NODE_NO_RETURN
#undef END_NODE
#undef INVALID
