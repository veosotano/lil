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

#include "LILShared.h"
#include "LILLexer.h"
#include "../shared/utf8/utf8.h"

#include "LILToken.h"
#include "LILForeignLangToken.h"
#include "LILStringToken.h"

//#define LILLEXERDEBUG

using namespace LIL;

bool isLatin1Letter(LILChar ch)
{
    return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}

namespace LIL
{
    class LILLexerPrivate
    {
    public:
        LILLexerPrivate()
        : sourceString("")
        , currentChar()
        , index(0)
        , bufferLength(0)
        , currentTokenText()
        , currentLine()
        , currentColumn()
        , peekPositionOffset()
        , peekLineOffset()
        , peekColumnOffset()
        , preferHex()
        , previousTokenIndex(0)
        , previousTokenLine(0)
        , previousTokenColumn(0)
        {
        }

        //The source string
        LILString sourceString;

        // The Unicode character that is currently being processed
        LILChar currentChar;

        size_t index;
        size_t bufferLength;
        std::string::const_iterator iterator;
        std::string::const_iterator bufferBegin;
        std::string::const_iterator bufferEnd;

        // The text of a string token that is currently being processed
        LILString currentTokenText;

        // The line and column number of the current character
        size_t currentLine;
        size_t currentColumn;

        // Offsets from the buffer position and line/column numbers used when
        // peeking ahead in the stream
        size_t peekPositionOffset;
        size_t peekLineOffset;
        size_t peekColumnOffset;

        // If you are expecting a hexadecimal number, set this to true
        // don't forget to reset it afterwards
        bool preferHex;

        //stores the start of the previous token in case we want to rewind
        size_t previousTokenIndex;
        size_t previousTokenLine;
        size_t previousTokenColumn;
        std::string::const_iterator previousTokenIterator;
    };
}

LILLexer::LILLexer()
: d(new LILLexerPrivate)
{
    this->reset();
}

LILLexer::~LILLexer()
{
    delete d;
}

/*!
 * Resets all properties of the tokenizer to their defaults,
 * and clears the file
 */
void LILLexer::reset()
{
    d->sourceString = "";

    d->currentChar = '\0';
    d->currentTokenText = LILString();

    d->index = 0;
    d->bufferLength = 0;

    d->currentLine = 1;
    d->currentColumn = 1;

    d->peekPositionOffset = 0;
    d->peekLineOffset = 0;
    d->peekColumnOffset = 0;

    // By default, numbers are read as real numbers and A-F will be an identifier
    d->preferHex = false;

    d->previousTokenIndex = 0;
}

void LILLexer::setString(const LILString & theString)
{
    d->sourceString = theString;
    d->index = 0;
    d->bufferLength = theString.length();
    std::string::const_iterator beginIt = theString.begin();
    d->bufferBegin = d->iterator = beginIt;
    d->bufferEnd = theString.end();
}

bool LILLexer::isHexPreferred() const
{
    return d->preferHex;
}

void LILLexer::setHexPreferred(bool prefer)
{
    d->preferHex = prefer;
}

size_t LILLexer::currentLine() const
{
    return d->currentLine;
}

size_t LILLexer::currentColumn() const
{
    return d->currentColumn;
}

size_t LILLexer::currentIndex() const
{
    return d->index;
}

bool LILLexer::atEndOfSource() const
{
    return (d->iterator == d->bufferEnd) && (d->currentChar == '\0');
}

/*!
 * Reads the next character from the buffer and stores it.
 */
void LILLexer::readNextChar()
{
    if (d->iterator == d->bufferEnd)
    {
        d->currentChar = '\0';
    }
    else
    {
        d->currentChar = LILChar(utf8::next(d->iterator, d->bufferEnd));
#ifdef LILLEXERDEBUG
        const char parsedChar = static_cast<const char>(d->currentChar.data());
        std::cerr << "read char: ";
        std::cerr << parsedChar;
        std::cerr << "\n";
#endif
    }

    //LILLog(LoggerChannelLexer, LILString::format("Read character %c (line %d, col %d)", d->currentChar.data(), d->currentLine, d->currentColumn));

    d->index += 1;

    d->currentColumn += 1;
}

/*!
 * Reads and returns a pointer to the next token in the buffer, or \c NULL if the buffer was empty.
 */
std::shared_ptr<LILToken> LILLexer::readNextToken()
{
    std::shared_ptr<LILToken> ret;

    if (this->atEndOfSource() && d->currentChar == '\0')
    {
        return ret;
    }

    //store start index
    d->previousTokenIndex = d->index;
    d->previousTokenLine = d->currentLine;
    d->previousTokenColumn = d->currentColumn;
    d->previousTokenIterator = d->iterator;

    LILChar cc = d->currentChar;

    // Identifiers can start with a letter or an underscore
    if (isLatin1Letter(cc) || cc == '_')
    {
        if (d->preferHex)
            return this->readHexOrIdentifier();
        else
            return this->readIdentifier();
    }

    if (cc.isSpace())
        return this->readWhitespace();

    // If it starts with a number it is either a number or a percentage
    if (cc.isDigit())
    {
        if (d->preferHex)
            return this->readHexOrIdentifier();
        else
            return this->readNumberOrPercentage();
    }

    switch (cc.data())
    {
        // If it starts with quotes, either single or double, it is a string
        case '"':
        case '\'':
            return this->readString();
        case '#':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeInstructionSign, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '@':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeObjectSign, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '&':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeAmpersand, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '{':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeBlockOpen, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '}':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeBlockClose, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case ',':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeComma, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case ':':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeColon, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case ';':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeSemicolon, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '(':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeParenthesisOpen, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case ')':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeParenthesisClose, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '|':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeVerticalBar, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '/':
            return this->readCommentOrSymbol();
        case '!':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeNegator, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '.':
            return this->readDotChars();

        case '=':
            return this->readEqualSignOrFatArrow();

        case '>':
            return this->readComparator();

        case '<':
            return this->readComparatorOrForeignLang();

        case '[':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeSquareBracketOpen, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case ']':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeSquareBracketClose, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '+':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypePlusSign, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '-':
            return this->readMinusSignOrThinArrow();
        case '*':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeAsterisk, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;
        case '`':
            return this->readCString();
        case '%':
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypePercentSign, cc, d->currentLine, d->currentColumn - 1, d->index));
            this->readNextChar();
            return ret;

        default:
            return this->readInvalidChar();
    }
}

/*!
 * Reads and returns a pointer to the next token in the buffer, or \c NULL
 * if there is no input buffer.
 *
 * The actual position in the file is saved and can be restored
 * by calling \a resetPeek().
 */
std::shared_ptr<LILToken> LILLexer::peekNextToken()
{
    std::shared_ptr<LILToken> ret;

    // Store the current position in the buffer
    size_t savedPosition = d->index;
    size_t savedLine = d->currentLine;
    size_t savedColumn = d->currentColumn;

    // Read the next token
    ret = this->readNextToken();

    // Store the new offset
    d->peekPositionOffset += (d->index - savedPosition);
    d->peekLineOffset += (d->currentLine - savedLine);
    d->peekColumnOffset += (d->currentColumn - savedColumn);

//    if (ret)
        //LILLog(LoggerChannelLexer, LILString::format("Peeked (offset: %d, current position: %d, saved position: %d), read token %s", d->peekPositionOffset, d->index, savedPosition, ret->toString().chardata()));
//    else
        //LILLog(LoggerChannelLexer, LILString::format("Peeked (offset: %d, current position: %d, saved position: %d), no token read", d->peekPositionOffset, d->index, savedPosition));

    return ret;
}

/*!
 * Restores state after a call to \a peekNextToken().
 *
 * This method only needs to be called once to restore state
 * regardless of how many times \a peekNextToken() is called.
 */
void LILLexer::resetPeek()
{
    // Restore the saved position in the buffer
    // We start one character before we were before, since we are re-reading the character
    size_t delta = (d->peekPositionOffset + 1);

    d->index -= delta;
    d->currentLine -= d->peekLineOffset;
    d->currentColumn -= (d->peekColumnOffset + 1);

    if (d->currentChar == '\0')
        delta -= 1;

    for (int i = 0; i<delta; ++i)
    {
        LILChar currentChar = utf8::prior(d->iterator, d->bufferBegin);
    }
    this->readNextChar();

    // Reset the peek offsets
    d->peekPositionOffset = 0;
    d->peekLineOffset = 0;
    d->peekColumnOffset = 0;

    //LILLog(LoggerChannelLexer, LILString("Peek reset"));
}

void LILLexer::rewindToPreviousToken()
{
    d->index = d->previousTokenIndex;
    d->currentLine = d->previousTokenLine;
    d->currentColumn = d->previousTokenColumn;
    d->iterator = d->previousTokenIterator;
    utf8::prior(d->iterator, d->bufferBegin);
    d->currentChar = LILChar(utf8::next(d->iterator, d->bufferEnd));
}

/*!
 * Skips over any whitespace characters.
 */
void LILLexer::skipWhitespace()
{
    while (d->currentChar.isSpace())
    {
        // We only want to consider something after \n to be a new line, as this
        // effectively matches \n and \r\n. No modern system considers \r alone
        // to be a new line, and checking for it here would cause most Windows
        // files to show incorrect line numbers, as a new line would be registered
        // for both the \r AND the \n.
        if (d->currentChar == '\n')
        {
            d->currentLine++;
            d->currentColumn = 1;
        }

        this->readNextChar();
    }
}

/*!
 * Stores the current character in the multi-character token buffer,
 * then reads the next character.
 */
void LILLexer::storeCurrentCharAndReadNext()
{
    d->currentTokenText += d->currentChar;
    this->readNextChar();
}

/*!
 * Clears the current text token and returns its value.
 */
LILString LILLexer::extractCurrentTokenText()
{
    LILString text = d->currentTokenText;
    d->currentTokenText.clear();
    return text;
}

/*!
 * Reads and returns a whitespace token.
 */
std::shared_ptr<LILToken> LILLexer::readWhitespace()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    while (d->currentChar.isSpace())
    {
        // We only want to consider something after \n to be a new line, as this
        // effectively matches \n and \r\n. No modern system considers \r alone
        // to be a new line, and checking for it here would cause most Windows
        // files to show incorrect line numbers, as a new line would be registered
        // for both the \r AND the \n.
        if (d->currentChar == '\n')
        {
            d->currentLine++;
            d->currentColumn = 1;
        }

        this->storeCurrentCharAndReadNext();
    }

    return std::shared_ptr<LILToken>(new LILToken(TokenTypeWhitespace, this->extractCurrentTokenText(), line, column, index));
}

/*!
 * Reads and returns an identifier token.
 */
std::shared_ptr<LILToken> LILLexer::readIdentifier()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    while (isLatin1Letter(d->currentChar) || d->currentChar.isDigit() || d->currentChar == '_')
    {
        this->storeCurrentCharAndReadNext();
    }

    return std::shared_ptr<LILToken>(new LILToken(TokenTypeIdentifier, this->extractCurrentTokenText(), line, column, index));
}

/*!
 * Reads and returns a hexadecimal number or identifier token.
 */
std::shared_ptr<LILToken> LILLexer::readHexOrIdentifier()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    bool done = false;
    d->currentTokenText.clear();
    while (!done)
    {
        switch (d->currentChar.data())
        {
        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
            this->storeCurrentCharAndReadNext();
            continue;

        default:
            if (d->currentChar.isDigit())
            {
                this->storeCurrentCharAndReadNext();
                continue;
            }
            else
            {
                if (isLatin1Letter(d->currentChar))
                {
                    done = true;
                    break;
                }
                else if (d->currentTokenText.length() > 0)
                {
                    return std::shared_ptr<LILToken>(new LILToken(TokenTypeHexNumber, this->extractCurrentTokenText(), line, column, index));
                }
                else
                {
                    done = true;
                    break;
                }
            }
        }
    }

    // If we reached this far, it is an identifier - finish reading it
    return this->readIdentifier();
}

/*!
 * Reads and returns either a number or a percentage token.
 *
 * NOTE: This function assumes that the current character is a digit
 * (between U+0030 and U+0039, inclusive).
 */
std::shared_ptr<LILToken> LILLexer::readNumberOrPercentage()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    bool dotFound = false;
    while (d->currentChar.isDigit() || d->currentChar == '.')
    {
        if (d->currentChar == '.')
        {
            if (dotFound)
                break;
            else
                dotFound = true;
        }

        this->storeCurrentCharAndReadNext();
    }

    std::shared_ptr<LILToken> ret;
    if (d->currentChar == '%')
    {
        if (dotFound) {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypePercentageNumberFP, this->extractCurrentTokenText(), line, column, index));
        } else {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypePercentageNumberInt, this->extractCurrentTokenText(), line, column, index));
        }
        
        this->readNextChar();
    }
    else
    {
        if (dotFound) {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeNumberFP, this->extractCurrentTokenText(), line, column, index));
        } else {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeNumberInt, this->extractCurrentTokenText(), line, column, index));
        }
    }

    return ret;
}

/*!
 * Reads and returns either a single quoted or double quoted string token.
 *
 * NOTE: This function assumes that the current character is " (U+0022) or ' (U+0027).
 */
std::shared_ptr<LILToken> LILLexer::readString()
{
    std::shared_ptr<LILToken> errorState;
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    if (d->currentChar != '"' && d->currentChar != '\'')
        return errorState;

    bool isDoubleQuote = d->currentChar == '"';
    const char compareChar = isDoubleQuote ? '"' : '\'';

    std::shared_ptr<LILStringToken> strToken(new LILStringToken((isDoubleQuote ?TokenTypeDoubleQuoteString : TokenTypeSingleQuoteString), line, column, index));

    size_t strIndex = 1;

    this->storeCurrentCharAndReadNext();
    bool stringDone = false;
    while (!stringDone)
    {
        stringDone = true;
        if (this->atEndOfSource())
        {
            break;
        }
        if (d->currentChar == '%')
        {
            LILChar peekChar = utf8::peek_next(d->iterator, d->bufferEnd);
            if (isLatin1Letter(peekChar) || peekChar == '_' || peekChar == '{'||peekChar == '@') {
                strToken->setHasArguments(true);
                strToken->addIndex(strIndex);
            }
            else
            {
                //it is a regular char of the string
                //continue
                stringDone = false;
                this->storeCurrentCharAndReadNext();
                ++strIndex;
            }
            //stop here
        }
        else if (d->currentChar != compareChar)
        {
            //continue
            stringDone = false;
            this->storeCurrentCharAndReadNext();
            ++strIndex;
        }
        else
        {
            //read end quotes
            this->storeCurrentCharAndReadNext();
        }
    }

    strToken->setString(this->extractCurrentTokenText());
    return strToken;
}

std::shared_ptr<LILToken> LILLexer::readCString()
{
    std::shared_ptr<LILToken> errorState;
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    if (d->currentChar != '`')
        return errorState;

    std::shared_ptr<LILToken> ret =  std::make_shared<LILToken>(TokenTypeCString, line, column, index);

    size_t strIndex = 1;

    this->storeCurrentCharAndReadNext();
    bool stringDone = false;
    while (!stringDone)
    {
        stringDone = true;
        if (this->atEndOfSource())
        {
            break;
        }
        else if (d->currentChar != '`')
        {
            //continue
            stringDone = false;
            this->storeCurrentCharAndReadNext();
            ++strIndex;
        }
        else
        {
            //read end backtick
            this->storeCurrentCharAndReadNext();
        }
    }

    ret->setString(this->extractCurrentTokenText());
    return ret;
}

std::shared_ptr<LILStringToken> LILLexer::readString(std::shared_ptr<LILStringToken> strToken, bool & done)
{
    done = true;

    LILString currentStr = strToken->getString();
    bool isDoubleQuote = strToken->getType() == TokenTypeDoubleQuoteString;
    const char compareChar = isDoubleQuote ? '"' : '\'';

    size_t strIndex = currentStr.length();

    bool stringDone = false;
    while (!stringDone)
    {
        stringDone = true;
        if (this->atEndOfSource())
        {
            break;
        }
        if (d->currentChar == '%')
        {
            //communicate to outside
            done = false;
            strToken->addIndex(strIndex);
            //stop here
        }
        else if (d->currentChar != compareChar)
        {
            //continue
            stringDone = false;
            this->storeCurrentCharAndReadNext();
            ++strIndex;
        }
        else
        {
            //read end quotes
            this->storeCurrentCharAndReadNext();
        }
    }

    LILString endStr = this->extractCurrentTokenText();
    std::shared_ptr<LILStringToken> endToken = std::shared_ptr<LILStringToken>(new LILStringToken(strToken->getType(), strToken->line, strToken->column, strToken->index));
    endToken->setString(endStr);

    strToken->setString(currentStr + endStr);

    return endToken;
}

/*!
 * Reads and returns a comment or a symbol token.
 *
 * NOTE: This function assumes that the current character is / (U+002F).
 */
std::shared_ptr<LILToken> LILLexer::readCommentOrSymbol()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    std::shared_ptr<LILToken> ret;
    this->storeCurrentCharAndReadNext();
    if (d->currentChar == '/')
    {
        this->storeCurrentCharAndReadNext(); // skip '/'

        // Read all chars until end of line
        while (d->currentChar != '\n' && d->currentChar != '\r' && d->currentChar != '\f' && !this->atEndOfSource())
        {
            this->storeCurrentCharAndReadNext();
        }

        ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeLineComment, this->extractCurrentTokenText(), line, column, index));
    }
    else if (d->currentChar == '*')
    {
        this->storeCurrentCharAndReadNext(); //skip '*'
        while (true)
        {
            if (d->currentChar == '*')
            {
                this->storeCurrentCharAndReadNext(); // We won't know if it is the end of the comment until we seek further
                if (d->currentChar == '/')
                {
                    // It is the end, break the loop
                    d->currentTokenText += d->currentChar;
                    break;
                }
            }
            else if (this->atEndOfSource())
            {
                break;
            }
            else
            {
                this->storeCurrentCharAndReadNext();
            }
        }

        ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeBlockComment, this->extractCurrentTokenText(), line, column, index));
        readNextChar();
    }
    else
    {
        ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeSlash, this->extractCurrentTokenText(), line, column, index));
    }

    return ret;
}

/*!
 * Reads and returns a dot characters
 *
 * NOTE: This function expects the current character to be a dot "."
 */
std::shared_ptr<LILToken> LILLexer::readDotChars()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;
    std::shared_ptr<LILToken> ret;
    if (d->currentChar == '.')
    {
        this->readNextChar();
        if (d->currentChar == '.')
        {
            this->readNextChar();

            if (d->currentChar == '.')
            {
                this->readNextChar();
                ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeEllipsis, "...", line, column, index));
                return ret;
            } else {
                ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeDoubleDot, "..", line, column, index));
                return ret;
            }
        }
        else
        {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeDot, ".", line, column, index));
            return ret;
        }
    }
    return ret;
}

std::shared_ptr<LILToken> LILLexer::readComparatorOrForeignLang()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;
    std::shared_ptr<LILToken> ret;
    LILChar peekChar;
    auto iterator = d->iterator;
    LILString completeString;
    if (d->currentChar == '<') {
        bool done = false;
        bool isTag = false;
        while (!done) {
            done = true;
            if (this->atEndOfSource())
            {
                break;
            }
            peekChar = utf8::peek_next(iterator, d->bufferEnd);
            if (isLatin1Letter(peekChar)) {
                done = false;
            } else if (peekChar == '>') {
                isTag = true;
            }
            iterator += 1;
        }
        if (isTag) {
            //add the initial <
            completeString += d->currentChar;
            this->readNextChar();
            //read the foreign language name
            done = false;
            while (!done) {
                done = true;
                if (this->atEndOfSource())
                {
                    break;
                }
                if (d->currentChar != '>') {
                    this->storeCurrentCharAndReadNext();
                    done = false;
                }
            }
            auto token = std::make_shared<LILForeignLangToken>(TokenTypeForeignLang, line, column, index);
            auto languageStr = this->extractCurrentTokenText();
            completeString += languageStr;
            token->setLanguage(languageStr);
            //add the > to the complete string
            completeString += d->currentChar;

            this->readNextChar();

            //read the content of the tag
            done = false;
            while (!done) {
                done = true;
                if (this->atEndOfSource())
                {
                    break;
                }
                if (d->currentChar == '<') {
                    peekChar = utf8::peek_next(d->iterator, d->bufferEnd);
                    if (peekChar != '/') {
                        this->storeCurrentCharAndReadNext();
                        done = false;
                    }
                } else {
                    this->storeCurrentCharAndReadNext();
                    done = false;
                }
            }
            auto contentStr = this->extractCurrentTokenText();
            completeString += contentStr;
            token->setContent(contentStr);

            //read the closing tag
            done = false;
            while (!done) {
                done = true;
                if (this->atEndOfSource())
                {
                    break;
                }
                if (d->currentChar != '>') {
                    this->storeCurrentCharAndReadNext();
                    done = false;
                }
            }
            this->storeCurrentCharAndReadNext();
            completeString += this->extractCurrentTokenText();

            token->setString(completeString);

            return token;
        } else {
            this->readNextChar();
            auto token = std::make_shared<LILToken>(TokenTypeSmallerComparator, d->currentChar, line, column, index);
            return token;
        }
    }
    return nullptr;
}

/*!
 * Reads and returns a comparator
 *
 * NOTE: This function expects the current character to be a symbol
 */
std::shared_ptr<LILToken> LILLexer::readComparator()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;
    std::shared_ptr<LILToken> ret;
    if (d->currentChar == '>')
    {
        this->readNextChar();
        if (d->currentChar == '=')
        {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeBiggerOrEqualComparator, ">=", line, column, index));
            this->readNextChar();
            return ret;
        }
        else
        {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeBiggerComparator, ">", line, column, index));
            return ret;
        }
    }
    else if (d->currentChar == '<')
    {
        this->readNextChar();
        if (d->currentChar == '=')
        {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeSmallerOrEqualComparator, "<=", line, column, index));
            this->readNextChar();
            return ret;
        }
        else
        {
            ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeSmallerComparator, "<", line, column, index));
            return ret;
        }
    }
    return ret;
}

std::shared_ptr<LILToken> LILLexer::readEqualSignOrFatArrow()
{
    LILChar cc = d->currentChar;
    this->readNextChar();
    LILChar cc2 = d->currentChar;
    if (cc2 == '>') {
        LILString tokenStr = cc;
        tokenStr += cc2;
        std::shared_ptr<LILToken> ret = std::make_shared<LILToken>(TokenTypeFatArrow, tokenStr, d->currentLine, d->currentColumn - 1, d->index);
        this->readNextChar();
        return ret;
    } else {
        std::shared_ptr<LILToken> ret = std::make_shared<LILToken>(TokenTypeEqualSign, d->currentChar, d->currentLine, d->currentColumn - 1, d->index);
        //do not read next char here, since it was already read
        return ret;
    }
}

std::shared_ptr<LILToken> LILLexer::readMinusSignOrThinArrow()
{
    LILChar cc = d->currentChar;
    this->readNextChar();
    LILChar cc2 = d->currentChar;
    if (cc2 == '>') {
        LILString tokenStr = cc;
        tokenStr += cc2;
        std::shared_ptr<LILToken> ret = std::make_shared<LILToken>(TokenTypeThinArrow, tokenStr, d->currentLine, d->currentColumn - 1, d->index);
        this->readNextChar();
        return ret;
    } else {
        std::shared_ptr<LILToken> ret = std::make_shared<LILToken>(TokenTypeMinusSign, cc, d->currentLine, d->currentColumn - 1, d->index);
        //do not read next char here, since it was already read
        return ret;
    }
}

std::shared_ptr<LILToken> LILLexer::readInvalidChar()
{
    const size_t line = d->currentLine;
    const size_t column = d->currentColumn - 1;
    const size_t index = d->index;

    std::shared_ptr<LILToken> ret;

    ret = std::shared_ptr<LILToken>(new LILToken(TokenTypeNone, d->currentChar, line, column, index));
    this->readNextChar();
    return ret;
}
