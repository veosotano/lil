//===- LLVMIRLexer.h - Lexer for LLVM Assembly Files ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class represents the Lexer for .ll files.
//
//===----------------------------------------------------------------------===//

#ifndef LLVMIRLEXER_H
#define LLVMIRLEXER_H

#include "LLVMIRToken.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/Support/SourceMgr.h"
#include <string>

namespace llvm {
  class MemoryBuffer;
  class Type;
  class SMDiagnostic;
  class LLVMContext;

  class LLVMIRLexer {
	const char *CurPtr;
	StringRef CurBuf;
	SMDiagnostic &ErrorInfo;
	SourceMgr &SM;
	LLVMContext &Context;

	// Information about the current token.
	const char *TokStart;
	lltoken::Kind CurKind;
	std::string StrVal;
	unsigned UIntVal;
	Type *TyVal;
	APFloat APFloatVal;
	APSInt  APSIntVal;

	// When false (default), an identifier ending in ':' is a label token.
	// When true, the ':' is treated as a separate token.
	bool IgnoreColonInIdentifiers;

  public:
	explicit LLVMIRLexer(StringRef StartBuf, SourceMgr &SM, SMDiagnostic &,
					 LLVMContext &C);

	lltoken::Kind Lex() {
	  return CurKind = LexToken();
	}

	typedef SMLoc LocTy;
	LocTy getLoc() const { return SMLoc::getFromPointer(TokStart); }
	lltoken::Kind getKind() const { return CurKind; }
	const std::string &getStrVal() const { return StrVal; }
	Type *getTyVal() const { return TyVal; }
	unsigned getUIntVal() const { return UIntVal; }
	const APSInt &getAPSIntVal() const { return APSIntVal; }
	const APFloat &getAPFloatVal() const { return APFloatVal; }

	void setIgnoreColonInIdentifiers(bool val) {
	  IgnoreColonInIdentifiers = val;
	}

	bool Error(LocTy ErrorLoc, const Twine &Msg) const;
	bool Error(const Twine &Msg) const { return Error(getLoc(), Msg); }

	void Warning(LocTy WarningLoc, const Twine &Msg) const;
	void Warning(const Twine &Msg) const { return Warning(getLoc(), Msg); }

  private:
	lltoken::Kind LexToken();

	int getNextChar();
	lltoken::Kind ReadString(lltoken::Kind kind);
	bool ReadVarName();

	lltoken::Kind LexWhitespace();
	lltoken::Kind LexComment();
	lltoken::Kind LexIdentifier();
	lltoken::Kind LexDigitOrNegative();
	lltoken::Kind LexPositive();
	lltoken::Kind LexAt();
	lltoken::Kind LexDollar();
	lltoken::Kind LexExclaim();
	lltoken::Kind LexPercent();
	lltoken::Kind LexUIntID(lltoken::Kind Token);
	lltoken::Kind LexVar(lltoken::Kind Var, lltoken::Kind VarID);
	lltoken::Kind LexQuote();
	lltoken::Kind Lex0x();
	lltoken::Kind LexHash();
	lltoken::Kind LexCaret();

	uint64_t atoull(const char *Buffer, const char *End);
	uint64_t HexIntToVal(const char *Buffer, const char *End);
	void HexToIntPair(const char *Buffer, const char *End, uint64_t Pair[2]);
	void FP80HexToIntPair(const char *Buffer, const char *End, uint64_t Pair[2]);
  };
} // end namespace llvm

#endif
