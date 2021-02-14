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
 *      This file implements converting characters into tokens,
 *      the smallest parts in the parsing system
 *
 ********************************************************************/

#ifndef LILLEXER_H
#define LILLEXER_H

namespace LIL
{
    class LILToken;
    class LILStringToken;
    class LILLexerPrivate;

    class LILLexer
    {
    public:
        LILLexer();
        ~LILLexer();

        void reset();

        void setString(const LILString & theString);

        bool isHexPreferred() const;
        void setHexPreferred(bool prefer);

        size_t currentLine() const;
        size_t currentColumn() const;
        size_t currentIndex() const;

        bool atEndOfSource() const;

        void readNextChar();

        std::shared_ptr<LILToken> readNextToken();
        std::shared_ptr<LILToken> peekNextToken();
        void resetPeek();
        void rewindToPreviousToken();
        std::shared_ptr<LILStringToken> readString(std::shared_ptr<LILStringToken> strToken, bool & done);

    private:
        void storeCurrentCharAndReadNext();

        LILString extractCurrentTokenText();

        std::shared_ptr<LILToken> readWhitespace();
        std::shared_ptr<LILToken> readIdentifier();
        std::shared_ptr<LILToken> readHexOrIdentifier();
        std::shared_ptr<LILToken> readNumberOrPercentage();
        std::shared_ptr<LILToken> readString();
        std::shared_ptr<LILToken> readCString();
        std::shared_ptr<LILToken> readInstructionSignOrDoc();
        std::shared_ptr<LILToken> readCommentOrSymbol();
        std::shared_ptr<LILToken> readDotChars();
        std::shared_ptr<LILToken> readComparator();
        std::shared_ptr<LILToken> readComparatorOrForeignLang();
        std::shared_ptr<LILToken> readEqualSignOrFatArrow();
        std::shared_ptr<LILToken> readMinusSignOrThinArrow();
        std::shared_ptr<LILToken> readInvalidChar();

        LILLexerPrivate *d;
    };
}

#endif
