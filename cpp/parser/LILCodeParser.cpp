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
 *      This file reads LIL source code and tells a delegate
 *      about it
 *
 ********************************************************************/

#include "LILShared.h"
#include "LILCodeParser.h"
#include "LILLexer.h"
#include "LILStringToken.h"
#include "LILToken.h"
#include "LILAbstractParserReceiver.h"

#define LIL_START_NODE(node_type)\
    size_t __startCol = d->column;\
    size_t __startLine = d->line;\
    size_t __startIndex = d->index;\
    NodeType __nodeType = node_type;\
    d->receiver->receiveNodeStart(__nodeType);\

#define LIL_CHECK_FOR_END\
    if(this->atEndOfSource()) {\
        d->receiver->receiveNodeEnd(__nodeType);\
        return false;\
    }\

#define LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE\
    LIL_CHECK_FOR_END(__nodeType);\
    this->skip(TokenTypeWhitespace);\
    LIL_CHECK_FOR_END(__nodeType);\

#define LIL_EXPECT(tokenType, expecting_str)\
    if (!d->currentToken->isA(tokenType)){\
        d->receiver->receiveError("Found " + d->currentToken->getString() + " while expecting " + expecting_str, d->file, d->line, d->column);\
        d->receiver->receiveNodeEnd(__nodeType);\
        return false;\
    }\

#define LIL_CANCEL_NODE {\
    d->receiver->receiveNodeEnd(__nodeType);\
    return false;\
}

#define LIL_END_NODE_NO_RETURN\
    LILRange __newRange = {\
        __startIndex,\
        d->index - __startIndex,\
    };\
    d->receiver->receiveSourceLocation(d->file, __startLine, __startCol, __newRange);\
    d->receiver->receiveNodeEnd(__nodeType);\

#define LIL_END_NODE_SKIP(skipWhitespace)\
    LIL_END_NODE_NO_RETURN\
    if (skipWhitespace && !atEndOfSource())\
        this->skip(TokenTypeWhitespace);\
    return true;

#define LIL_END_NODE\
    LIL_END_NODE_SKIP(true)

using namespace LIL;

namespace LIL
{
    class LILCodeParserPrivate
    {
        friend class LILCodeParser;

        LILCodeParserPrivate()
        : hasLoadedFile()
        , file()
        , lexer()
        , line(1)
        , column(1)
        , receiver()
        , notifiesReceiver(true)
        , allowsWhitespaceFunctionCall(true)
        {
        }
        LILAbstractParserReceiver * receiver;

        bool hasLoadedFile;

        std::shared_ptr<LILLexer> lexer;
        LILString file;
        std::vector<LILString > loadedFiles;

        std::shared_ptr<LILToken> currentToken;
        size_t line;
        size_t column;
        size_t index;
        LILString lastObjectType;
        bool notifiesReceiver;
        bool allowsWhitespaceFunctionCall;
    };
}

LILCodeParser::LILCodeParser(LILAbstractParserReceiver * receiver)
: d(new LILCodeParserPrivate)
{
    std::shared_ptr<LILLexer> lexer = std::shared_ptr<LILLexer>(new LILLexer());
    d->lexer = lexer;
    d->receiver = receiver;
}

LILCodeParser::~LILCodeParser()
{
    delete d;
}

void LILCodeParser::reset()
{
    d->lexer->reset();
    d->hasLoadedFile = false;
    d->file = "";
    d->receiver->reset();
}

void LILCodeParser::parseString(const LILString & theString)
{
    //needs reset on next load
    d->lexer->setString(theString);
    d->lexer->readNextChar();

    this->readNextToken();

    std::shared_ptr<LILNode> statement;

    bool done = this->atEndOfSource();

    if(!done) this->skip(TokenTypeWhitespace);

    while (!done)
    {
        if (statement)
        {
            statement.reset();
        }
        if (this->atEndOfSource())
        {
            done = true;
        }
        else
        {
            this->parseNext();
        }
    }

    d->receiver->receiveNodeCommit();
}

void LILCodeParser::readNextToken()
{
    //read next one
    std::shared_ptr<LILToken> theToken = d->lexer->readNextToken();
    this->updateCurrentToken(theToken);
}

void LILCodeParser::updateCurrentToken(std::shared_ptr<LILToken> theToken)
{
    if (theToken && (theToken->isA(TokenTypeBlockComment) || theToken->isA(TokenTypeLineComment)))
    {
                 d->receiver->receiveNodeData(ParserEventComment, theToken->getString());
        this->readNextToken();
    }
    else if (theToken)
    {
        d->currentToken = theToken;
//        std::cerr << "acquired token: ";
//        std::cerr << theToken->getString().chardata();
//        std::cerr << "\n";
    } else {
        d->currentToken.reset();
    }

    if (!this->atEndOfSource())
    {
        d->line = d->currentToken->line;
        d->column = d->currentToken->column;
        d->index = d->currentToken->index;
    }
    else
    {
        d->line = d->lexer->currentLine();
        d->column = d->lexer->currentColumn() - 1;
        d->index = d->lexer->currentIndex() - 1;
    }
}
bool LILCodeParser::atEndOfSource() const
{
    if (!d->currentToken && d->lexer->atEndOfSource())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void LILCodeParser::skip(TokenType type)
{
    if (d->currentToken && d->currentToken->isA(type))
    {
        if (type == TokenTypeWhitespace)
        {
            d->receiver->receiveNodeData(ParserEventWhitespace, d->currentToken->getString());
            this->readNextToken();
            if (!this->atEndOfSource() && d->currentToken->isA(type))
            {
                this->skip(type);
            }
        }
        else
        {
            d->receiver->receiveNodeData(ParserEventOther, d->currentToken->getString());
            this->readNextToken();
        }
    }
}

void LILCodeParser::skipInvalidToken()
{
    if (d->currentToken->isA(TokenTypeSingleQuoteString) || d->currentToken->isA(TokenTypeDoubleQuoteString))
    {
        std::shared_ptr<LILStringToken> strToken = std::static_pointer_cast<LILStringToken>(d->currentToken);
        LILString theString;
        if (strToken->hasArguments())
        {
            d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());

            this->readNextToken();
            if (this->atEndOfSource())
                return;

            this->skipStringArgument();
            if (d->currentToken->isA(TokenTypeBlockClose))
            {
                d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
                this->readNextToken();
                if (this->atEndOfSource())
                    return;
            }

            bool stringDone = false;
            std::shared_ptr<LILStringToken> strChunk;
            while(!stringDone)
            {
                //readString() will set stringDone to false, and re-set it to true if needed
                strChunk = d->lexer->readString(strToken, stringDone);

                this->readNextToken();
                if (this->atEndOfSource())
                    return;

                if (!stringDone)
                {
                    //notify the mid chunk of the string
                    d->receiver->receiveNodeData(ParserEventInvalid, strChunk->getString());

                    this->skipStringArgument();

                    if (d->currentToken->isA(TokenTypeBlockClose))
                    {
                        d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
                        this->readNextToken();
                    }
                    if (this->atEndOfSource())
                        return;
                }
            }
            //notify the end chunk of the string
            d->receiver->receiveNodeData(ParserEventInvalid, strChunk->getString());
        }
        else
        {
            d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
        }
        this->readNextToken();
    }
    else
    {
        d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
    }
    this->readNextToken();
}

void LILCodeParser::skipStringArgument()
{
    //skip the %
    if (d->currentToken->isA(TokenTypePercentSign))
    {
        d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
        this->readNextToken();
        if (this->atEndOfSource())
            return;
    }

    bool hasBlock = false;
    if (d->currentToken->isA(TokenTypeBlockOpen))
    {
        hasBlock = true;
        d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
        this->readNextToken();
        if (this->atEndOfSource())
            return;

        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
            return;
    }

    bool valueValid = true;
    NodeType nodeType = NodeTypeInvalid;
    valueValid = this->readBasicValue(nodeType);
    return;
}

void LILCodeParser::skipUntilSemicolon()
{
    while (!this->atEndOfSource() &&!d->currentToken->isA(TokenTypeSemicolon) && !d->currentToken->isA(TokenTypeBlockClose))
    {
        this->readNextToken();
        if (this->atEndOfSource()) return;
        if (d->currentToken->isA(TokenTypeBlockOpen))
        {
            this->skipUntilSemicolon();
            if (this->atEndOfSource()) return;
            this->readNextToken();
            if (this->atEndOfSource()) return;
        }
    }
    this->readNextToken();
    if (this->atEndOfSource()) return;
    this->skip(TokenTypeWhitespace);
}

void LILCodeParser::skipUntilEndOfValuePath()
{
    while (!this->atEndOfSource() &&!d->currentToken->isA(TokenTypeSemicolon) && !d->currentToken->isA(TokenTypeBlockClose))
    {
        this->readNextToken();
        if (this->atEndOfSource()) return;
        if (d->currentToken->isA(TokenTypeBlockOpen))
        {
            this->skipUntilEndOfValuePath();
            if (this->atEndOfSource()) return;
            this->readNextToken();
            if (this->atEndOfSource()) return;
        }
    }
    this->readNextToken();
    if (this->atEndOfSource()) return;
    this->skip(TokenTypeWhitespace);
}

void LILCodeParser::parseNext()
{
     if (this->atEndOfSource())
         return;

    bool isValid = false;

     switch (d->currentToken->getType())
     {
         case TokenTypeInstructionSign:
         {
             isValid = this->readInstruction();
             break;
         }

         case TokenTypeObjectSign:
         {
             if (this->isObjectSelector())
             {
                 isValid = this->readRule();
             }
             else
             {
                 isValid = this->readObjectDefinition();
             }
             break;
         }

         case TokenTypeIdentifier:
         {
             isValid = this->readIdentifierStatement();
             break;
         }
         case TokenTypeAsterisk:
         case TokenTypeEqualSign:
         case TokenTypeMinusSign:
         case TokenTypePlusSign:
         case TokenTypeBiggerComparator:
         case TokenTypeDot:
         case TokenTypeDoubleDot:
         {
             isValid = this->readRule();
             break;
         }

         case TokenTypeBlockComment:
         case TokenTypeLineComment:
         {
             d->receiver->receiveNodeData(ParserEventComment, d->currentToken->getString());
             this->readNextToken();
             break;
         }

         case TokenTypeSemicolon:
         case TokenTypeWhitespace:
         {
             isValid = false;
             break;
         }

         case TokenTypeNumberFP:
         case TokenTypeNumberInt:
         case TokenTypePercentageNumber:
         case TokenTypeParenthesisOpen:
         {
             bool outIsSV = false;
             NodeType svExpTy = NodeTypeInvalid;
             isValid = this->readExpression(outIsSV, svExpTy);
             if (!atEndOfSource())
                 this->skip(TokenTypeWhitespace);
             break;
         }

         default:
         {
             isValid = false;
             break;
         }
     }

    if (!atEndOfSource() && !isValid)
    {
        this->skipUntilSemicolon();
    }

     if (!this->atEndOfSource())
         this->skip(TokenTypeWhitespace);

    if (!atEndOfSource() && d->currentToken->isA(TokenTypeSemicolon)) {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }

    if (!this->atEndOfSource())
        this->skip(TokenTypeWhitespace);

    if (isValid)
    {
        d->receiver->receiveNodeCommit();
    }
}

bool LILCodeParser::isBool() const
{
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        if (d->currentToken->getString() == "true" || d->currentToken->getString() == "false")
        {
            return true;
        }
    }
    return false;
}

bool LILCodeParser::isValuePath() const
{
    std::shared_ptr<LILToken> peekToken;
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        peekToken = d->lexer->peekNextToken();
        if (peekToken && peekToken->isA(TokenTypeDot))
        {
            peekToken = d->lexer->peekNextToken();
            if (peekToken->isA(TokenTypeIdentifier))
            {
                d->lexer->resetPeek();
                return true;
            }
        }
        else if (peekToken && peekToken->isA(TokenTypeParenthesisOpen))
        {
            d->lexer->resetPeek();
            return true;
        }
        d->lexer->resetPeek();
        return false;
    }
    return false;
}

//this assumes we've alread read past the identifier / property path and whitespace
bool LILCodeParser::isExpression() const
{
    std::shared_ptr<LILToken> peekToken;
    peekToken = d->currentToken;
    switch (peekToken->getType())
    {
        case TokenTypeNegator:
        {
            peekToken = d->lexer->peekNextToken();
            if (peekToken->isA(TokenTypeEqualSign))
            {
                d->lexer->resetPeek();
                return true;
            }
            break;
        }
        case TokenTypeEqualComparator:
        case TokenTypeBiggerComparator:
        case TokenTypeSmallerComparator:
        case TokenTypeBiggerOrEqualComparator:
        case TokenTypeSmallerOrEqualComparator:
        case TokenTypePlusSign:
        case TokenTypeMinusSign:
        case TokenTypeAsterisk:
        case TokenTypeSlash:
        case TokenTypePercentSign:
        {
            d->lexer->resetPeek();
            return true;
        }
        default:
            break;
    }
    d->lexer->resetPeek();
    return false;
}

bool LILCodeParser::isUnaryExpression() const
{
    std::shared_ptr<LILToken> peekToken = d->lexer->peekNextToken();
    if (peekToken && peekToken->isA(TokenTypeWhitespace))
    {
        peekToken = d->lexer->peekNextToken();
    }
    if (peekToken && peekToken->isA(TokenTypeParenthesisOpen))
    {
        d->lexer->resetPeek();
        return true;
    }
    d->lexer->resetPeek();
    return false;
}

bool LILCodeParser::isObjectSelector() const
{
    std::shared_ptr<LILToken> peekToken;
    if (d->currentToken->isA(TokenTypeObjectSign))
    {
        peekToken = d->lexer->peekNextToken();
        if (peekToken && peekToken->isA(TokenTypeIdentifier))
        {
            LILString objtype = peekToken->getString();
            if (
                objtype == "this"
                || objtype == "self"
                || objtype == "parent"
                || objtype == "root"
                || objtype == "event"
                || objtype == "key"
                || objtype == "value"
                )
            {
                d->lexer->resetPeek();
                return true;
            }
        }
        d->lexer->resetPeek();
        return false;
    }
    return false;
}

bool LILCodeParser::isAssignment() const
{

    bool ret = false;

    std::shared_ptr<LILToken> peekToken;
    if (d->currentToken->isA(TokenTypeInstructionSign))
    {
        bool currentPref = d->lexer->isHexPreferred();
        d->lexer->setHexPreferred(true);

        peekToken = d->lexer->peekNextToken();
        ret = peekToken->isA(TokenTypeHexNumber);
        d->lexer->setHexPreferred(currentPref);

    }
    else
    {
        peekToken = d->currentToken;
        if (peekToken->isA(TokenTypeIdentifier)) {
            peekToken = d->lexer->peekNextToken();
        }
        bool done = false;
        while (peekToken && !done) {
            //we assume we are done
            done = true;
            //skip all whitespace and comments
            while (peekToken && (peekToken->isA(TokenTypeWhitespace) || peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)))
            {
                peekToken = d->lexer->peekNextToken();
            }
            //if the next token is a colon, it is either an assignment or a filter
            if (peekToken->isA(TokenTypeColon))
            {
                peekToken = d->lexer->peekNextToken();
                if (!peekToken)
                {
                    d->lexer->resetPeek();
                    return false;
                }

                //if it was a filter, it would be an identifier
                if (!peekToken->isA(TokenTypeIdentifier)) {
                    d->lexer->resetPeek();
                    return true;
                }

                //it might be a filter in a selector chain
                while (peekToken && !peekToken->isA(TokenTypeSemicolon) && !peekToken->isA(TokenTypeBlockClose) && !peekToken->isA(TokenTypeBlockOpen))
                {
                    peekToken = d->lexer->peekNextToken();
                }
                //if we find an opening block, we're dealing with a selector
                if (peekToken && peekToken->isA(TokenTypeBlockOpen))
                {
                    ret = false;
                }
            }
            //either property grouping or a rule
            else if (peekToken->isA(TokenTypeComma))
            {
                peekToken = d->lexer->peekNextToken();
                //skip all whitespace and comments
                while (peekToken && (peekToken->isA(TokenTypeWhitespace) || peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)))
                {
                    peekToken = d->lexer->peekNextToken();
                }
                if (peekToken->isA(TokenTypeIdentifier))
                {
                    peekToken = d->lexer->peekNextToken();
                    done = false;
                    continue;
                }
            }
            //this might be a value path
            else if(peekToken->isA(TokenTypeDot))
            {
                peekToken = d->lexer->peekNextToken();
                if(peekToken && peekToken->isA(TokenTypeIdentifier)){
                    //we are still in the value path, continue
                    peekToken = d->lexer->peekNextToken();
                    done = false;
                    continue;
                }
                else
                {
                    //this can't be a property definition -- either a syntax error or a selector chain
                    ret = false;
                }
            }
            else
            {
                if (d->currentToken->isA(TokenTypeObjectSign))
                {
                    ret = false;
                }
                else
                {
                    //no colon, it may be a rule -- we peek until we find the end of the statement
                    //or we can conclude it actually is a rule
                    bool done = false;
                    while (!done)
                    {
                        switch (peekToken->getType())
                        {
                            case TokenTypeObjectSign:
                            {
                                ret = false;
                                done = true;
                                break;
                            }

                            case TokenTypeBlockOpen:
                                ret = false;
                                done = true;
                                break;

                            default:
                                break;
                        }

                        if (!done)
                        {
                            peekToken = d->lexer->peekNextToken();
                            if (!peekToken)
                            {
                                //we probably reached end of source, abort
                                done = true;
                            }
                        }
                    }
                }
            }
        } //end while !done
    }

    d->lexer->resetPeek();
    return ret;
}

bool LILCodeParser::isRule() const
{

    bool ret = false;

    std::shared_ptr<LILToken> peekToken;
    if (d->currentToken->isA(TokenTypeInstructionSign))
    {
        return this->isInstructionRule();
    }
    else
    {
        peekToken = d->lexer->peekNextToken();
        bool done = false;
        while (peekToken && !done) {
            //we assume we are done
            done = true;
            //skip all whitespace and comments
            while (peekToken && (peekToken->isA(TokenTypeWhitespace) || peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)))
            {
                peekToken = d->lexer->peekNextToken();
            }
            //if the next token is a colon, it is either a property definition or a filter
            if (peekToken->isA(TokenTypeColon))
            {
                //we'll peek until we find a end of statement, a closing block or an opening one
                peekToken = d->lexer->peekNextToken();
                //if we find a whitespace or an object sign here, we can be sure it's a property definition
                if (!peekToken)
                {
                    d->lexer->resetPeek();
                    return false;
                }
                if (peekToken->isA(TokenTypeWhitespace) || peekToken->isA(TokenTypeObjectSign))
                {
                    ret = false;
                }
                else
                {
                    while (peekToken && !peekToken->isA(TokenTypeSemicolon) && !peekToken->isA(TokenTypeBlockClose) && !peekToken->isA(TokenTypeBlockOpen))
                    {
                        peekToken = d->lexer->peekNextToken();
                    }
                    //if we find an opening block, we're dealing with a selector
                    if (peekToken && peekToken->isA(TokenTypeBlockOpen))
                    {
                        ret = true;
                    }
                }
            }
            //either property grouping or a rule
            else if (peekToken->isA(TokenTypeComma))
            {
                peekToken = d->lexer->peekNextToken();
                //skip all whitespace and comments
                while (peekToken && (peekToken->isA(TokenTypeWhitespace) || peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)))
                {
                    peekToken = d->lexer->peekNextToken();
                }
                if (peekToken->isA(TokenTypeIdentifier))
                {
                    peekToken = d->lexer->peekNextToken();
                    done = false;
                    continue;
                }
            }
            //this might be dot notation
            else if(peekToken->isA(TokenTypeDot))
            {
                peekToken = d->lexer->peekNextToken();
                done = false;
                continue;
            }
            else
            {
                if (d->currentToken->isA(TokenTypeObjectSign))
                {
                    return this->isObjectSelector();
                }
                else
                {
                    //we peek until we find the end of the statement
                    //or we can conclude it actually is a rule
                    bool done = false;
                    while (!done)
                    {
                        switch (peekToken->getType())
                        {
                            case TokenTypeObjectSign:
                            {
                                ret = this->isObjectSelector();;
                                done = true;
                                break;
                            }

                            case TokenTypeBlockOpen:
                                ret = true;
                                done = true;
                                break;

                            default:
                                break;
                        }

                        if (!done)
                        {
                            peekToken = d->lexer->peekNextToken();
                            if (!peekToken)
                            {
                                //we probably reached end of source, abort
                                done = true;
                            }
                        }
                    }
                }
            }
        } //end while !done
    }

    d->lexer->resetPeek();
    return ret;
}

bool LILCodeParser::isInstructionRule() const
{
    std::shared_ptr<LILToken> peekToken = d->lexer->peekNextToken();
    if (peekToken->isA(TokenTypeInstructionSign)) {
        peekToken = d->lexer->peekNextToken();
    }
    if (
        peekToken->getString() == "new"
        || peekToken->getString() == "move"
        || peekToken->getString() == "delete"
        ) {
        return true;
    }
    return false;
}

bool LILCodeParser::isColorInstruction() const
{
    //FIXME: implement this
    return false;
}

bool LILCodeParser::isComparisonSign() const
{
    switch (d->currentToken->getType())
    {
        case TokenTypeBiggerComparator:
        case TokenTypeSmallerComparator:
        case TokenTypeAmpersand:
        case TokenTypeVerticalBar:
            return true;

        default:
            break;
    }
    return false;
}

bool LILCodeParser::isCombinator() const
{
    return this->isCombinator(d->currentToken);
}

bool LILCodeParser::isCombinator(std::shared_ptr<LILToken> token) const
{
    switch (token->getType())
    {
        case TokenTypeEqualSign:
        case TokenTypeMinusSign:
        case TokenTypePlusSign:
        case TokenTypeBiggerComparator:
        case TokenTypeDot:
        case TokenTypeDoubleDot:
            return true;
        case TokenTypeWhitespace:
            return this->isChildrenCombinator();
        default:
            break;
    }
    return false;
}

bool LILCodeParser::isChildrenCombinator() const
{
    //if the next token is anything other than a combinator, an open block or an object sign the whitespace means children combinator
    std::shared_ptr<LILToken> peekToken = d->lexer->peekNextToken();
    if(!peekToken){
        return false;
    }
    bool ret = !this->isCombinator(peekToken) && !peekToken->isA(TokenTypeBlockOpen) && !peekToken->isA(TokenTypeObjectSign);
    d->lexer->resetPeek();
    return ret;
}

bool LILCodeParser::isNegator() const
{
    if (d->currentToken->isA(TokenTypeNegator))
    {
        return true;
    }

    return false;
}

bool LILCodeParser::isIndexAccessor() const
{
    return d->currentToken->isA(TokenTypeSquareBracketOpen);
}

bool LILCodeParser::isFunctionCall(bool isPastIdentifier) const
{
    if (isPastIdentifier)
    {
        if (d->currentToken->isA(TokenTypeParenthesisOpen)){
            return true;
        } else if (!d->currentToken->isA(TokenTypeWhitespace)){
            return false;
        }
        //we got whitespace, check if next thing is a valid value
        if (!d->allowsWhitespaceFunctionCall) {
            return false;
        }
        std::shared_ptr<LILToken> peekToken = d->lexer->peekNextToken();

        while (peekToken && peekToken->isA(TokenTypeWhitespace))
        {
            peekToken = d->lexer->peekNextToken();

            if (peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)) {
                peekToken = d->lexer->peekNextToken();
            }
        }
        if (peekToken){
            d->lexer->resetPeek();
            return !this->isPunctuation(peekToken);
        }
    }
    else if (d->currentToken->isA(TokenTypeIdentifier))
    {
        if (this->isBuiltinFunctionCall())
            return true;

        std::shared_ptr<LILToken> peekToken = d->lexer->peekNextToken();
        if(!peekToken) return false;

        if (
            !peekToken->isA(TokenTypeWhitespace)
            && !peekToken->isA(TokenTypeParenthesisOpen)
            && !peekToken->isA(TokenTypeBlockComment)
            && !peekToken->isA(TokenTypeLineComment)
        ) {
            d->lexer->resetPeek();
            return false;
        }

        //we got whitespace, check if next thing is a valid value
        while (peekToken && peekToken->isA(TokenTypeWhitespace))
        {
            peekToken = d->lexer->peekNextToken();

            if (peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)) {
                peekToken = d->lexer->peekNextToken();
            }
        }
        if (peekToken){
            d->lexer->resetPeek();
            return !this->isPunctuation(peekToken);
        }

        d->lexer->resetPeek();
        return peekToken->isA(TokenTypeParenthesisOpen);
    }

    d->lexer->resetPeek();
    return false;
}

bool LILCodeParser::isBuiltinFunctionCall() const
{
    std::string value = d->currentToken->getString().data();
    if (
        value == "ref"
        || value == "sel"
        || value == "flag"
        || value == "unflag"
        || value == "addFlag"
        || value == "takeFlag"
        || value == "replaceFlag"
        || value == "pointerTo"
        || value == "valueOf"
        )
    {
        return true;
    }

    return false;
}

bool LILCodeParser::isFlowControlCall() const
{
    std::string value = d->currentToken->getString().data();
    if (
        value == "return"
        || value == "repeat"
        || value == "continue"
        || value == "break"
        )
    {
        return true;
    }
    
    return false;
}

bool LILCodeParser::isFunctionDecl() const
{
    std::string value = d->currentToken->getString().data();
    if (
        value == "fn"
        || value == "override"
        || value == "insert"
    ) {
        return true;
    }

    return false;
}

bool LILCodeParser::isFlowControl() const
{
    std::string value = d->currentToken->getString().data();
    if (
        value == "if"
        || value == "else"
        || value == "switch"
        || value == "for"
        || value == "loop"
        || value == "finally"
    ) {
        return true;
    }

    return false;
}

bool LILCodeParser::isIfIs() const
{
    if (!d->currentToken || d->currentToken->getString() != "if"){
        return false;
    }
    std::shared_ptr<LILToken> peekToken = d->lexer->peekNextToken();

    while (peekToken && peekToken->isA(TokenTypeWhitespace))
    {
        peekToken = d->lexer->peekNextToken();
        if (!peekToken) {
            d->lexer->resetPeek();
            return false;
        }

        if (peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)) {
            peekToken = d->lexer->peekNextToken();
            if (!peekToken) {
                d->lexer->resetPeek();
                return false;
            }
        }
    }
    if (peekToken && peekToken->isA(TokenTypeIdentifier)){
        peekToken = d->lexer->peekNextToken();
    } else {
        d->lexer->resetPeek();
        return false;
    }
    while (peekToken && peekToken->isA(TokenTypeWhitespace))
    {
        peekToken = d->lexer->peekNextToken();
        if (!peekToken) {
            d->lexer->resetPeek();
            return false;
        }

        if (peekToken->isA(TokenTypeBlockComment) || peekToken->isA(TokenTypeLineComment)) {
            peekToken = d->lexer->peekNextToken();
            if (!peekToken) {
                d->lexer->resetPeek();
                return false;
            }
        }
    }

    bool ret = true;
    if (peekToken && peekToken->isA(TokenTypeIdentifier)){
        ret = peekToken->getString() == "is";
    } else {
        ret = false;
    }
    d->lexer->resetPeek();
    return ret;
}

bool LILCodeParser::isFlag() const
{
    std::string name = d->currentToken->getString().data();
    return name == "flag"
    || name == "unflag"
    || name == "addFlag"
    || name == "takeFlag"
    || name == "replaceFlag";
}

bool LILCodeParser::isPunctuation(std::shared_ptr<LILToken> token) const
{
    switch (token->getType()) {
        case TokenTypeDot:
        case TokenTypeParenthesisOpen:
        case TokenTypeParenthesisClose:
        case TokenTypeBlockOpen:
        case TokenTypeBlockClose:
        case TokenTypeColon:
        case TokenTypeSlash:
        case TokenTypePlusSign:
        case TokenTypeMinusSign:
        case TokenTypeAsterisk:
        case TokenTypeSemicolon:
        case TokenTypeAmpersand:
        case TokenTypeComma:
        case TokenTypeDoubleDot:
        case TokenTypeEllipsis:
        case TokenTypeEqualSign:
        case TokenTypePercentSign:
        case TokenTypeVerticalBar:
        case TokenTypeEqualComparator:
        case TokenTypeBiggerComparator:
        case TokenTypeSmallerComparator:
        case TokenTypeSquareBracketOpen:
        case TokenTypeSquareBracketClose:
        case TokenTypeBiggerOrEqualComparator:
        case TokenTypeSmallerOrEqualComparator:
        case TokenTypeThinArrow:
        case TokenTypeFatArrow:
            return true;

        case TokenTypeNone:
        case TokenTypeIdentifier:
        case TokenTypeObjectSign:
        case TokenTypeHexNumber:
        case TokenTypeNumberFP:
        case TokenTypeNumberInt:
        case TokenTypeDoubleQuoteString:
        case TokenTypeSingleQuoteString:
        case TokenTypeCString:
        case TokenTypeNegator:
        case TokenTypeWhitespace:
        case TokenTypeLineComment:
        case TokenTypeBlockComment:
        case TokenTypeInstructionSign:
        case TokenTypePercentageNumber:
            return false;
    }
    return false;
}

bool LILCodeParser::readIdentifierStatement()
{
    bool statementDone = false;
    bool isValid = false;
    while (!statementDone)
    {
        statementDone = true;
        if (d->currentToken->getString() == "class")
        {
            return this->readClassDecl();
        }
        if (d->currentToken->getString() == "var")
        {
            return this->readVarDecl();
        }
        if (this->isAssignment()){
            isValid = this->readAssignment(true, true);
        }
        else if (this->isValuePath())
        {
            isValid = this->readValuePath(true, true);
            if (this->isExpression())
            {
                bool outIsSV = false;
                NodeType outType;
                isValid = this->readExpression(outIsSV, outType);
                if (atEndOfSource())
                    return false;
                if (isValid)
                {
                    statementDone = false;
                }
            }
        }
        else
        {
            if (this->isRule())
            {
                isValid = this->readRule();
            }
            else
            {
                bool outIsSingleValue;
                NodeType outType;
                isValid = this->readExpression(outIsSingleValue, outType);
                if (!isValid)
                    return false;
            }
        }
    }
    return isValid;
}

bool LILCodeParser::readClassDecl()
{
    LIL_START_NODE(NodeTypeClassDecl)
    d->receiver->receiveNodeData(ParserEventClassDecl, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeObjectSign, "object sign");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END

    LIL_EXPECT(TokenTypeIdentifier, "identifier");
    bool otValid = this->readObjectType();
    if (otValid) {
        d->receiver->receiveNodeCommit();
    }
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (d->currentToken->isA(TokenTypeIdentifier) && d->currentToken->getString() == "inherits") {
        d->receiver->receiveNodeData(ParserEventInherits, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        LIL_EXPECT(TokenTypeObjectSign, "object sign");
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END

        LIL_EXPECT(TokenTypeIdentifier, "identifier");
        bool otValid = this->readObjectType();
        if (otValid) {
            d->receiver->receiveNodeCommit();
        }
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    LIL_EXPECT(TokenTypeBlockOpen, "block open");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool done = false;
    while (!done) {
        done = true;
        if (d->currentToken->isA(TokenTypeBlockClose)) {
            break;
        }
        LIL_EXPECT(TokenTypeIdentifier, "identifier");
        if (
            d->currentToken->getString() == "var"
            || d->currentToken->getString() == "ivar"
            || d->currentToken->getString() == "prop"
            ){
            bool vdValid = this->readVarDecl();
            if (vdValid) {
                d->receiver->receiveNodeCommit();
            } else {
                LIL_CANCEL_NODE
            }
            LIL_CHECK_FOR_END
            if (d->currentToken->isA(TokenTypeSemicolon)) {
                done = false;
                d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                this->readNextToken();
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }
        }
    }

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeBlockClose, "block close");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE
}

bool LILCodeParser::readType()
{
    bool done = false;
    bool valid = true;

    bool isMultiple = false;
    auto peekToken = d->lexer->peekNextToken();
    if (peekToken) {
        if (peekToken->isA(TokenTypeParenthesisOpen)) {
            peekToken = d->lexer->peekNextToken();
        }
        if (peekToken && peekToken->isA(TokenTypeIdentifier)) {
            peekToken = d->lexer->peekNextToken();
        }
        if (peekToken && peekToken->isA(TokenTypeVerticalBar)) {
            isMultiple = true;
        }
    }
    d->lexer->resetPeek();

    if (!isMultiple) {
        return this->readTypeSimple();
    }

    LIL_START_NODE(NodeTypeMultipleType);
    bool needsClosingParenthesis = d->currentToken->isA(TokenTypeParenthesisOpen);
    if (needsClosingParenthesis) {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END
    }
    while (!done) {
        done = true;

        bool tyValid = this->readTypeSimple();
        if (tyValid) {
            d->receiver->receiveNodeCommit();
        } else {
            valid = false;
        }
        LIL_CHECK_FOR_END
        if (d->currentToken->isA(TokenTypeVerticalBar)) {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END
            done = false;
            continue;
        }
    }
    if (needsClosingParenthesis) {
        if (!d->currentToken->isA(TokenTypeParenthesisClose)){
            d->receiver->receiveError("Found " + d->currentToken->getString() + " while expecting parenthesis close", d->file, d->line, d->column);
        } else {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END
        }
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readTypeSimple()
{
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        LILString tokenStr = d->currentToken->getString();
        if (tokenStr == "fn") {
            return this->readFunctionType();
        } else if (tokenStr == "ptr"){
            return this->readPointerType();
        }
        LIL_START_NODE(NodeTypeType)

        d->receiver->receiveNodeData(ParserEventType, tokenStr);
        this->readNextToken();
        LIL_CHECK_FOR_END
        if (
            tokenStr == "array"
            || tokenStr == "map"
        ) {
            LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis");
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            bool done = false;
            while(!done){
                done = true;
                if (d->currentToken->isA(TokenTypeComma)) {
                    done = false;
                    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                    this->readNextToken();
                    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
                }
                bool tyValid = this->readType();
                if (tyValid) {
                    d->receiver->receiveNodeCommit();
                }
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }
            LIL_EXPECT(TokenTypeParenthesisClose, "close parenthesis");
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
        }
        LIL_END_NODE_SKIP(false)
    }
    else if (d->currentToken->isA(TokenTypeObjectSign))
    {
        LIL_START_NODE(NodeTypeType)
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END

        LIL_EXPECT(TokenTypeIdentifier, "identifier")
        d->receiver->receiveNodeData(ParserEventType, d->currentToken->getString());
        this->readNextToken();
        LIL_END_NODE_SKIP(false)
    }
    else
    {
        d->receiver->receiveError("Found " + d->currentToken->getString() + " while expecting identifier or object sign", d->file, d->line, d->column);
        return false;
    }
    return false;
}

bool LILCodeParser::readFunctionType()
{
    LIL_START_NODE(NodeTypeFunctionType)
    d->receiver->receiveNodeData(ParserEventType, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END

    LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool done = false;
    while(!done){
        done = true;
        if (d->currentToken->isA(TokenTypeComma)) {
            done = false;
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
        }
        bool tyValid = this->readType();
        if (tyValid) {
            d->receiver->receiveNodeCommit();
        }
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    if (d->currentToken->isA(TokenTypeEllipsis)) {
        d->receiver->receiveNodeData(ParserEventFunctionVariadic, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END
    }
    LIL_EXPECT(TokenTypeParenthesisClose, "close parenthesis");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeFatArrow, "fat arrow");
    if (d->currentToken->isA(TokenTypeFatArrow)) {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    d->receiver->receiveNodeData(ParserEventReturnType, "");
    bool tyValid = this->readType();
    if (tyValid) {
        d->receiver->receiveNodeCommit();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readPointerType()
{
    LIL_START_NODE(NodeTypePointerType)
    d->receiver->receiveNodeData(ParserEventType, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END
    
    LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    
    bool tyValid = this->readType();
    if (tyValid) {
        d->receiver->receiveNodeCommit();
    }
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    
    LIL_EXPECT(TokenTypeParenthesisClose, "close parenthesis");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readVarDecl()
{
    //skip the "var"
    if (d->currentToken->getString() != "var")
        return false;

    LIL_START_NODE(NodeTypeVarDecl)
    d->receiver->receiveNodeData(ParserEventVariableDeclaration, d->currentToken->getString());

    this->readNextToken();
    LIL_CHECK_FOR_END

    if (d->currentToken->isA(TokenTypeDot)) {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END

        bool tyValid = this->readType();
        if (tyValid) {
            d->receiver->receiveNodeCommit();
        }
    }
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the variable name
    LIL_EXPECT(TokenTypeIdentifier, "identifier")

    LILString name = d->currentToken->getString();

    d->receiver->receiveNodeData(ParserEventVarName, d->currentToken->getString());

    std::shared_ptr<LILToken> theIdentifier = d->currentToken;

    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (d->currentToken->isA(TokenTypeIdentifier) && d->currentToken->getString() == "extern") {
        d->receiver->receiveNodeData(ParserEventExtern, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }
    if (
        d->currentToken->isA(TokenTypeSemicolon)
        || d->currentToken->isA(TokenTypeParenthesisClose)
        ||  d->currentToken->isA(TokenTypeBlockClose)
    ) {
        LIL_END_NODE_SKIP(false)
    }
    else
    {
        LIL_EXPECT(TokenTypeColon, "colon")
    }
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool outIsSingleValue;
    NodeType svExpTy = NodeTypeInvalid;
    bool svIsValid = this->readExpression(outIsSingleValue, svExpTy);
    if (svIsValid){
        d->receiver->receiveNodeCommit();
    } else {
        LIL_CANCEL_NODE
    }

    LIL_END_NODE
}

bool LILCodeParser::readAssignment(bool allowValuePath, bool firstIsVar, bool firstPartAlreadyRead)
{
    LIL_START_NODE(NodeTypeAssignment)
    bool valid = true;
    
    if (firstPartAlreadyRead) {
        d->receiver->receiveNodeCommit();
    }

    switch (d->currentToken->getType())
    {
        case TokenTypeIdentifier:
        {
            bool done = false;
            while (!done)
            {
                done = true;
                if (allowValuePath) {
                    bool ppValid = this->readValuePath(firstIsVar, false);
                    d->receiver->receiveNodeCommit();
                    if (!ppValid){
                        LIL_CANCEL_NODE
                    }
                } else {
                    if (firstIsVar) {
                        bool vnValid = this->readVarName();
                        if (vnValid) {
                            d->receiver->receiveNodeCommit();
                        } else {
                            LIL_CANCEL_NODE
                        }
                    } else {
                        bool pnValid = this->readPropertyName();
                        if (pnValid) {
                            d->receiver->receiveNodeCommit();
                        } else {
                            LIL_CANCEL_NODE
                        }
                    }
                    LIL_CHECK_FOR_END
                }

                if (d->currentToken->isA(TokenTypeComma))
                {
                    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());

                    this->readNextToken();
                    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

                    if (d->currentToken->isA(TokenTypeIdentifier))
                    {
                        done = false;
                        continue;
                    }
                }
            }

            //allow whitespace before colon
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

            //now must come a colon
            LIL_EXPECT(TokenTypeColon, "colon")

            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

            break;
        }

        case TokenTypeColon:
        {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            break;
        }

        default:
        {
            this->skipInvalidToken();
            LIL_CANCEL_NODE
            break;
        }
    }

    //do not allow empty property definitions
    if (d->currentToken->isA(TokenTypeSemicolon) || d->currentToken->isA(TokenTypeBlockClose)){
        LIL_CANCEL_NODE
    }

    //readVals does auto commit
    bool valuesValid = this->readVals();

    if (!valuesValid) valid = false;

    if (!valid || atEndOfSource())
    {
        LIL_CANCEL_NODE
    }

    LIL_END_NODE
}

bool LILCodeParser::readVals()
{
    return this->readVals(true, false);
}

bool LILCodeParser::readVals(bool useComma, bool strict)
{
    bool done = false;
    bool valid = true;

    //if strict, requires value to be valid
    bool outIsSV = false;
    NodeType svExpTy = NodeTypeInvalid;
    bool singleValValid = this->readExpression(outIsSV, svExpTy);

    if (strict && !singleValValid)
        valid = false;

    while (!done && singleValValid && valid)
    {
        done = true;

        if ((valid))
        {
            d->receiver->receiveNodeCommit();
        }

        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
            return false;

        if (useComma && d->currentToken->isA(TokenTypeComma))
        {
            done = false;
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            if (this->atEndOfSource())
                return false;

            this->skip(TokenTypeWhitespace);
            if (this->atEndOfSource())
                return false;

            bool outIsSV = false;
            NodeType svExpTy = NodeTypeInvalid;
            bool expValid = this->readExpression(outIsSV, svExpTy);
            if(!expValid) valid = false;
            continue;
        }
        else if (!useComma && d->currentToken->isA(TokenTypeSemicolon))
        {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            if (this->atEndOfSource())
                return false;

            this->skip(TokenTypeWhitespace);
            if (this->atEndOfSource())
                return false;

            bool outIsSV = false;
            NodeType svExpTy = NodeTypeInvalid;
            bool expValid = this->readExpression(outIsSV, svExpTy);
            if(!expValid) valid = false;
            done = false;
            continue;
        }
    }
    return valid;
}

bool LILCodeParser::readExpression(bool &outIsSingleValue, NodeType & outType)
{
    bool done = false;
    bool valid = false;

    bool isNegation = false;
    if (d->currentToken->isA(TokenTypeNegator))
    {
        isNegation = true;
        this->skip(TokenTypeNegator);
        if (this->atEndOfSource())
            return false;
        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
            return false;
    }
    NodeType svNodeType = NodeTypeInvalid;
    if (d->currentToken->isA(TokenTypeParenthesisOpen))
    {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        if (this->atEndOfSource())
            return false;
        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
            return false;
        //in case reading the expression gives a single value, take the event from here
        bool outIsSV = false;
        NodeType svExpTy = NodeTypeInvalid;
        valid = this->readExpression(outIsSV, svNodeType);
        if (valid && outIsSV)
        {
            svNodeType = svExpTy;
        }
        if (this->atEndOfSource())
            return false;
        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
            return false;
        if (!d->currentToken->isA(TokenTypeParenthesisClose))
            return false;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        if (this->atEndOfSource())
            return false;
        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
            return false;
    }
    else
    {
        valid = this->readSingleValue(svNodeType);
        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
            return valid;
    }

    if (this->atEndOfSource())
        return valid;

    bool isSingleValue = true;

    while (!done && valid)
    {
        done = true;

        if (this->isExpression())
        {
            bool expValid = this->readExpressionInner();
            if (expValid)
            {
                isSingleValue = false;
            }
            else
            {
                valid = false;
            }
        }
        if (atEndOfSource())
            return false;
    }
    if (isSingleValue)
    {
        outIsSingleValue = true;
        outType = svNodeType;
    }
    return valid;
}

bool LILCodeParser::readExpressionInner()
{
    LIL_START_NODE(NodeTypeExpression)

    bool isValid = false;

    d->receiver->receiveNodeData(ParserEventExpressionSign, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool outIsSV = false;
    NodeType svExpTy = NodeTypeInvalid;
    isValid = this->readExpression(outIsSV, svExpTy);
    if (isValid && outIsSV)
    {
        d->receiver->receiveNodeCommit();
    }

    LIL_END_NODE
}

bool LILCodeParser::readUnaryExpression()
{
    LIL_START_NODE(NodeTypeUnaryExpression)

    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeParenthesisOpen, "parenthesis open")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //readVals auto commits
    bool valueValid = this->readVals();
    if (!valueValid)
        LIL_CANCEL_NODE

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeParenthesisClose, "parenthesis close")
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_END_NODE
}

bool LILCodeParser::readSingleValue(NodeType &nodeType)
{
    if (d->currentToken->isA(TokenTypeIdentifier)) {

        if (d->currentToken->getString() == "class")
        {
            return this->readClassDecl();
        }
        if (d->currentToken->getString() == "var")
        {
            return this->readVarDecl();
        }
        if (this->isFunctionDecl())
        {
            nodeType = NodeTypeFunctionDecl;
            return this->readFunctionDecl();
        }
        else if (this->isFlowControl())
        {
            nodeType = NodeTypeFlowControl;
            return this->readFlowControl();
        }
        else if (this->isFlowControlCall())
        {
            nodeType = NodeTypeFlowControlCall;
            return this->readFlowControlCall();
        }
        else if (this->isFunctionCall(false))
        {
            nodeType = NodeTypeFunctionCall;
            return this->readFunctionCall();
        }
    }
    return this->readBasicValue(nodeType);
 }

bool LILCodeParser::readBasicValue(NodeType &nodeType)
{
    switch (d->currentToken->getType())
    {
        case TokenTypeNumberInt:
        {
            LIL_START_NODE(NodeTypeNumberLiteral)
            d->receiver->receiveNodeData(ParserEventNumberLiteral, d->currentToken->getString());
            this->readNextToken();
            d->receiver->receiveNodeData(ParserEventNumberInt, "");
            nodeType = __nodeType;
            LIL_END_NODE_SKIP(false)
        }
        case TokenTypeNumberFP:
        {
            LIL_START_NODE(NodeTypeNumberLiteral)
            d->receiver->receiveNodeData(ParserEventNumberLiteral, d->currentToken->getString());
            this->readNextToken();
            d->receiver->receiveNodeData(ParserEventNumberFP, "");
            nodeType = __nodeType;
            LIL_END_NODE_SKIP(false)
        }

        case TokenTypePercentageNumber:
        {
            LIL_START_NODE(NodeTypePercentage)
            d->receiver->receiveNodeData(ParserEventPercentageLiteral, d->currentToken->getString());
            this->readNextToken();
            nodeType = __nodeType;
            LIL_END_NODE_SKIP(false)
        }

        case TokenTypeIdentifier:
        {
            //this is either a function, a boolean or an object name
            LILString valuestr = d->currentToken->getString();
            if(valuestr == "null")
            {
                LIL_START_NODE(NodeTypeNull)
                d->receiver->receiveNodeData(ParserEventNull, d->currentToken->getString());
                this->readNextToken();
                nodeType = __nodeType;
                LIL_END_NODE_SKIP(false)
            }
            else if (this->isBool())
            {
                LIL_START_NODE(NodeTypeBool)
                d->receiver->receiveNodeData(ParserEventBoolLiteral, d->currentToken->getString());
                this->readNextToken();
                nodeType = __nodeType;
                LIL_END_NODE_SKIP(false)
            }
            else
            {
                nodeType = NodeTypeValuePath;
                return this->readValuePath(true, true);
            }
            break;
        }
        case TokenTypeSingleQuoteString:
        case TokenTypeDoubleQuoteString:
        {
            std::shared_ptr<LILStringToken> strToken = std::static_pointer_cast<LILStringToken>(d->currentToken);
            LILString theString;

            if (strToken->hasArguments())
            {
                nodeType = NodeTypeStringFunction;
                return this->readStringFunction();
            }
            else
            {
                nodeType = NodeTypeStringLiteral;
                return this->readStringLiteral();
            }
            break;
        }
        case TokenTypePlusSign:
        case TokenTypeMinusSign:
        {
            if (this->isUnaryExpression())
            {
                return this->readUnaryExpression();
            }
            else
            {
                d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                this->readNextToken();
                if (atEndOfSource())
                {
                    return false;
                }

                if (d->currentToken->isA(TokenTypeNumberInt))
                {
                    LIL_START_NODE(NodeTypeNumberLiteral)
                    d->receiver->receiveNodeData(ParserEventNumberLiteral, d->currentToken->getString());
                    this->readNextToken();
                    d->receiver->receiveNodeData(ParserEventNumberInt, "");
                    nodeType = __nodeType;
                    LIL_END_NODE_SKIP(false)
                }
                else if (d->currentToken->isA(TokenTypeNumberFP))
                {
                    LIL_START_NODE(NodeTypeNumberLiteral)
                    d->receiver->receiveNodeData(ParserEventNumberLiteral, d->currentToken->getString());
                    this->readNextToken();
                    d->receiver->receiveNodeData(ParserEventNumberFP, "");
                    nodeType = __nodeType;
                    LIL_END_NODE_SKIP(false)
                }
                else if (d->currentToken->isA(TokenTypePercentageNumber))
                {
                    LIL_START_NODE(NodeTypePercentage)
                    d->receiver->receiveNodeData(ParserEventPercentageLiteral, d->currentToken->getString());
                    this->readNextToken();
                    nodeType = __nodeType;
                    LIL_END_NODE_SKIP(false)
                }
                else
                {
                    d->receiver->receiveError("Unexpected token", d->file, d->line, d->column);
                    return false;
                }
            }
            break;
        }

        case TokenTypeInstructionSign:
        {
            nodeType = NodeTypeInstruction;
            return this->readInstruction();
        }
        case TokenTypeObjectSign:
        {
            if (this->isObjectSelector())
            {
                nodeType = NodeTypeValuePath;
                return this->readObjectPath();
            }
            else
            {
                nodeType = NodeTypeObjectDefinition;
                return this->readObjectDefinition();
            }
        }
        case TokenTypeCString:
        {
            LIL_START_NODE(NodeTypeCStringLiteral)
            d->receiver->receiveNodeData(ParserEventCStringLiteral, d->currentToken->getString());
            this->readNextToken();
            LIL_END_NODE_SKIP(false)

            break;
        }
        default:
            break;
    }
    return false;
}

bool LILCodeParser::readStringLiteral()
{
    LIL_START_NODE(NodeTypeStringLiteral)
    d->receiver->receiveNodeData(ParserEventStringLiteral, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readStringFunction()
{
    LIL_START_NODE(NodeTypeStringFunction)
    std::shared_ptr<LILStringToken> strToken = std::static_pointer_cast<LILStringToken>(d->currentToken);

    d->receiver->receiveNodeData(ParserEventStringFunctionStart, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END

    bool argValid = this->readStringArgument();
    if (!argValid) {
        LIL_CANCEL_NODE
    }

    bool stringDone = false;
    std::shared_ptr<LILStringToken> strChunk;
    while(!stringDone){
        //readString() will set stringDone to false, and re-set it to true if needed
        strChunk = d->lexer->readString(strToken, stringDone);

        this->readNextToken();
        if (this->atEndOfSource())
        {
            d->receiver->receiveNodeData(ParserEventStringFunctionEnd, strChunk->getString());
            LIL_CANCEL_NODE
        }

        if (!stringDone)
        {
            //notify the mid chunk of the string
            d->receiver->receiveNodeData(ParserEventStringFunctionMid, strChunk->getString());

            bool argumentValid = this->readStringArgument();
            if (!argumentValid) {
                LIL_CANCEL_NODE
            }

            if (d->currentToken->isA(TokenTypeBlockClose))
            {
                d->receiver->receiveNodeData(ParserEventStringFunctionArgEnd, d->currentToken->getString());
            }
            if (this->atEndOfSource())
            {
                LIL_CANCEL_NODE
            }
        }
        else
        {
            d->receiver->receiveNodeData(ParserEventStringFunctionEnd, strChunk->getString());
        }
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readStringArgument()
{
    //skip the %
    if (d->currentToken->getString() == "%") {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        if (this->atEndOfSource())
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    bool hasBlock = false;
    if (d->currentToken->isA(TokenTypeBlockOpen))
    {
        hasBlock = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        if (this->atEndOfSource())
        {
            return false;
        }

        this->skip(TokenTypeWhitespace);
        if (this->atEndOfSource())
        {
            return false;
        }
    }

    //the block was empty, exit gracefully
    if (d->currentToken->isA(TokenTypeBlockClose))
    {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        return true;
    }

    //configure to return to a string on whitespace
    d->allowsWhitespaceFunctionCall = false;

    bool valid = true;
    NodeType nodeType = NodeTypeInvalid;
    valid = this->readBasicValue(nodeType);
    if (valid) {
        d->receiver->receiveNodeCommit();
    }

    if (this->atEndOfSource())
    {
        return false;
    }

    this->skip(TokenTypeWhitespace);
    if (this->atEndOfSource())
    {
        return false;
    }

    d->lexer->rewindToPreviousToken();
    d->allowsWhitespaceFunctionCall = true;

    if (!valid || this->atEndOfSource())
        return false;

    if (d->currentToken->isA(TokenTypeBlockClose))
    {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        return true;
    }
    else if (hasBlock)
    {
        this->skipInvalidToken();
        if (d->currentToken->isA(TokenTypeBlockClose))
        {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            if (!atEndOfSource()) {
                this->skip(TokenTypeWhitespace);
            }
        }
    }
    return true;
}

bool LILCodeParser::readObjectType()
{
    LIL_START_NODE(NodeTypeObjectType)
    LIL_EXPECT(TokenTypeIdentifier, "identifier");
    d->receiver->receiveNodeData(ParserEventType, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readObjectPath()
{
    bool osValid = this->readObjectSelector();
    if (!osValid || atEndOfSource())
        return false;

    return this->readValuePath(false, true);
}

//this assumes currentToken is an object sign
bool LILCodeParser::readObjectDefinition()
{
    LILString objtype;

    LIL_START_NODE(NodeTypeObjectDefinition)
    LIL_EXPECT(TokenTypeObjectSign, "object sign")

    d->receiver->receiveNodeData(ParserEventObjectDefinition, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END

    if (d->currentToken->isA(TokenTypeWhitespace) || d->currentToken->isA(TokenTypeBlockOpen)) {
        //no type means object
        objtype = "object";

    } else {
        LIL_EXPECT(TokenTypeIdentifier, "identifier");

        objtype = d->currentToken->getString();
        d->receiver->receiveNodeData(ParserEventType, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    }
    d->lastObjectType = objtype;

    if (d->currentToken->isA(TokenTypeWhitespace)) {
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    //get the name of the object
    switch (d->currentToken->getType()) {
        case TokenTypeIdentifier:
        {
            d->receiver->receiveNodeData(ParserEventVarName, d->currentToken->getString());
            LIL_CHECK_FOR_END
            break;
        }
        case TokenTypeBlockOpen:
            //it is the opening curly brace, therefore an anonymous object:
            //do nothing
            break;
        case TokenTypeSemicolon:
        case TokenTypeBlockClose:
        case TokenTypeParenthesisClose:
        {
            //the property definition ends here
            LIL_END_NODE
        }

        default:
        {
            d->receiver->receiveError("Found " + d->currentToken->getString() + " while expecting an identifier or a block open", d->file, d->line, d->column);
            LIL_CANCEL_NODE
        }
    }

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //skip the {
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the inner part of the block
    while (d->currentToken && !d->currentToken->isA(TokenTypeBlockClose))
    {
        if (this->isCombinator(d->currentToken))
        {
            bool ruleValid = this->readRule();
            if (ruleValid) {
                d->receiver->receiveNodeCommit();
            }
        }
        else
        {
            switch (d->currentToken->getType())
            {
                case TokenTypeAmpersand:
                {
                    d->lastObjectType = objtype;
                    this->readObjectDefinition();
                    d->receiver->receiveNodeCommit();
                    break;
                }

                case TokenTypeIdentifier:
                case TokenTypeColon:
                case TokenTypeObjectSign:
                case TokenTypeNegator:
                {
                    if (d->currentToken->getString() == "var")
                    {
                        bool vdValid = this->readVarDecl();
                        if (vdValid) {
                            d->receiver->receiveNodeCommit();
                        }
                        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
                        if (d->currentToken->isA(TokenTypeSemicolon)) {
                            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                            this->readNextToken();
                            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
                        }
                        continue;
                    }
                    if (this->isAssignment())
                    {
                        bool pdValid = this->readAssignment(true, false);
                        if (pdValid) {
                            d->receiver->receiveNodeCommit();
                        }
                        else
                        {
                            d->receiver->receiveError("Ignoring malformed property definition", d->file, d->line, d->column);
                            //allow the object definition to be alive with malformed properties
                            LIL_CHECK_FOR_END

                            d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
                            this->readNextToken();
                            LIL_CHECK_FOR_END
                            continue;
                        }
                    }
                    else
                    {
                        bool ruleValid = this->readRule();
                        if (ruleValid) {
                            d->receiver->receiveNodeCommit();
                        }
                    }
                    break;
                }

                case TokenTypeInstructionSign:
                {
                    if (this->isAssignment())
                    {
                        bool asgmtValid = this->readAssignment(true, false);
                        if (asgmtValid) {
                            d->receiver->receiveNodeCommit();
                        }
                    }
                    else
                    {
                        bool ruleValid = this->readInstructionRule();
                        if (ruleValid) {
                            d->receiver->receiveNodeCommit();
                        }
                    }
                    break;
                }

                default:
                {
                    //Unexpected token
                    d->receiver->receiveError("Ignoring unexpected token while reading object definition", d->file, d->line, d->column);
                    d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
                    this->readNextToken();
                    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
                    break;
                }
            }
            if (!this->atEndOfSource() && d->currentToken->isA(TokenTypeSemicolon))
            {
                d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                this->readNextToken();
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }
            if (this->atEndOfSource())
            {
                d->receiver->receiveError("Auto closing block of object definition because of unexpected end of file", d->file, d->line, d->column);
                return false;
            }
        }
    }
    LIL_CHECK_FOR_END

    //we're out of the block, we expect a closing brace
    LIL_EXPECT(TokenTypeBlockClose, "closing brace");

    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();

    d->lastObjectType = objtype;
    LIL_END_NODE
}

bool LILCodeParser::readRule()
{
    bool valid = true;

    //if we had a weird token here, abort
    if (d->currentToken->isA(TokenTypeDot) || d->currentToken->isA(TokenTypeSemicolon))
    {
        return false;
    }

    LIL_START_NODE(NodeTypeRule)

    //read selector chains
    //readSelectorChains auto commits
    bool scValid = this->readSelectorChains(TokenTypeBlockOpen);
    if (!scValid)
    {
        if (!atEndOfSource())
        {
            //notify the receiver
            d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END
        }
        LIL_CANCEL_NODE
    }

    LIL_CHECK_FOR_END

    if (!scValid)
    {
        LIL_CANCEL_NODE
    }

    //if we have a end of statement here, we're done
    if (d->currentToken->isA(TokenTypeSemicolon))
    {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();

        LIL_END_NODE
    }

    //we expect a block to open
    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the inner part of the block
    while (!this->atEndOfSource() && valid && !d->currentToken->isA(TokenTypeBlockClose))
    {
        switch (d->currentToken->getType())
        {
            case TokenTypeIdentifier:
            case TokenTypeObjectSign:
            {
                if (this->isAssignment())
                {
                    bool asgmtValid = this->readAssignment(true, false);
                    if (asgmtValid) {
                        d->receiver->receiveNodeCommit();
                    }
                }
                else
                {
                    this->readRule();
                    d->receiver->receiveNodeCommit();
                }
                break;
            }

            case TokenTypeDoubleDot:
            case TokenTypeNegator:
            case TokenTypeColon:
            case TokenTypeAsterisk:
            {
                bool ruleValid = this->readRule();
                if (ruleValid){
                    d->receiver->receiveNodeCommit();
                }
                break;
            }

            case TokenTypeInstructionSign:
            {
                bool ruleValid = this->readInstructionRule();
                if (ruleValid){
                    d->receiver->receiveNodeCommit();
                }
                break;
            }

            default:
            {
                d->receiver->receiveError("Unexpected token of type " + LILToken::tokenStringRepresentation(d->currentToken->getType()) + " while reading rule", d->file, d->line, d->column);
                d->receiver->receiveNodeData(ParserEventInvalid, d->currentToken->getString());
                this->readNextToken();
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
                break;
            }
        }

        //        if (this->atEndOfSource())
        //        {
        //            std::shared_ptr<LILNameSelector> sbjct = selectorChains.back()->subject()->getName();
        //            LILString lmntnm;
        //            if (sbjct)
        //            {
        //                lmntnm = sbjct->getElementName();
        //            }
        //            else
        //            {
        //                lmntnm = "unknown element";
        //            }
        //
        //            d->receiver->receiveError("Auto closing block of rule targeting " + lmntnm + " because of unexpected end of file", d->file, d->line, d->column);
        //            //leave the block context
        //            d->currentContext.pop_back();
        //            return newRule;
        //        }
    }

    if (!this->atEndOfSource())
    {
        //we expect a block to close
        LIL_EXPECT(TokenTypeBlockClose, "block close")

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }

    LIL_END_NODE
}

bool LILCodeParser::readSimpleSelector()
{
    LIL_START_NODE(NodeTypeSimpleSelector)

    bool isNegating = false;
    isNegating = this->isNegator();
    if (isNegating)
    {
        this->readNextToken();
        LIL_CHECK_FOR_END
    }

    bool isValid = false;

    switch (d->currentToken->getType())
    {
        case TokenTypeIdentifier:
        {
            isValid = this->readNameSelector(isNegating);
            if (isValid)
                d->receiver->receiveNodeCommit();

            //we're done negating for now
            isNegating = false;
            break;
        }

        case TokenTypeAsterisk:
        {
            if (isNegating){
                LIL_CANCEL_NODE
            }

            d->receiver->receiveNodeStart(NodeTypeSelector);
            d->receiver->receiveNodeData(ParserEventUniversalSelector, d->currentToken->getString());
            d->receiver->receiveNodeEnd(NodeTypeSelector);
            d->receiver->receiveNodeCommit();

            this->readNextToken();
            break;
        }

        case TokenTypeObjectSign:
        {
            isValid = this->readObjectSelector();
            if (isValid)
                d->receiver->receiveNodeCommit();
            break;
        }

        default:
        {
            d->receiver->receiveError("Unexpected token", d->file, d->line, d->column);
            this->skipInvalidToken();
            break;
        }
    }

    if (this->atEndOfSource())
        LIL_CANCEL_NODE

    bool done = false;
    while (!done)
    {
        if (!isNegating)
        {
            isNegating = this->isNegator();
            if (isNegating)
            {
                this->readNextToken();
                LIL_CHECK_FOR_END
            }
        }

        bool filterValid = this->readFilterOrFlag();
        if (filterValid)
        {
            //we're done negating for now
            isNegating = false;
            d->receiver->receiveNodeCommit();
        }
        else
        {
            done = true;
        }
    }

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readObjectSelector()
{
    LIL_START_NODE(NodeTypeSelector)

    switch (d->currentToken->getType())
    {
        case TokenTypeObjectSign:
        {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END

            if (d->currentToken->isA(TokenTypeIdentifier))
            {
                LILString objtype = d->currentToken->getString();
                if (
                    objtype == "this"
                    || objtype == "self"
                    || objtype == "super"
                    || objtype == "parent"
                    || objtype == "root"
                    || objtype == "event"
                    || objtype == "key"
                    || objtype == "value"
                )
                {
                    d->receiver->receiveNodeData(ParserEventType, objtype);
                    this->readNextToken();
                    break;
                }
            }
            else
            {
                d->receiver->receiveError("Unknown token type while reading object selector", d->file, d->line, d->column);
                LIL_CANCEL_NODE
            }
        }

        default:
        {
            d->receiver->receiveError("Unknown token type while reading object selector", d->file, d->line, d->column);
            LIL_CANCEL_NODE
        }
    }

    LIL_END_NODE
}

bool LILCodeParser::readFilterOrFlag()
{
    if (d->currentToken->isA(TokenTypeColon))
    {
        this->readNextToken();
        if (this->atEndOfSource())
            return false;

        if (d->currentToken->isA(TokenTypeColon))
        {
            LIL_START_NODE(NodeTypeFlag)
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END

            LIL_EXPECT(TokenTypeIdentifier, "identifier")
            d->receiver->receiveNodeData(ParserEventFlag, d->currentToken->getString());
            this->readNextToken();
            LIL_END_NODE
        }
        else
        {
            LIL_START_NODE(NodeTypeFilter)
            d->receiver->receiveNodeData(ParserEventPunctuation, ":");

            LIL_EXPECT(TokenTypeIdentifier, "identifier")
            d->receiver->receiveNodeData(ParserEventFilter, d->currentToken->getString());
            this->readNextToken();
            LIL_END_NODE
        }
    }
    return false;
}

bool LILCodeParser::readCombinator()
{
    LIL_START_NODE(NodeTypeCombinator)

    switch (d->currentToken->getType())
    {
        case TokenTypeEqualSign:
        {
             d->receiver->receiveNodeData(ParserEventCombinatorSiblings, d->currentToken->getString());
            break;
        }
        case TokenTypePlusSign:
        {
            d->receiver->receiveNodeData(ParserEventCombinatorNextSiblings, d->currentToken->getString());
            break;
        }
        case TokenTypeMinusSign:
        {
            d->receiver->receiveNodeData(ParserEventCombinatorPreviousSiblings, d->currentToken->getString());
            break;
        }
        case TokenTypeBiggerComparator:
        {
            d->receiver->receiveNodeData(ParserEventCombinatorTextSelection, d->currentToken->getString());
            break;
        }

        case TokenTypeDot:
        {
            d->receiver->receiveNodeData(ParserEventCombinatorNearestDescendants, d->currentToken->getString());
            break;
        }
        case TokenTypeDoubleDot:
        {
            d->receiver->receiveNodeData(ParserEventCombinatorDescendants, d->currentToken->getString());
            break;
        }
        default:
        {
            LIL_CANCEL_NODE
        }
    }

    this->readNextToken();

    LIL_END_NODE
}

bool LILCodeParser::readChildrenCombinatorOrSkip()
{
    //are we dealing with whitespace?
    if (d->currentToken->isA(TokenTypeWhitespace))
    {
        if (this->isChildrenCombinator())
        {
            LIL_START_NODE(NodeTypeCombinator)
            d->receiver->receiveNodeData(ParserEventCombinatorChildren, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END

            LIL_END_NODE
        }
        else
        {
            //alright, ignore it
            this->skip(TokenTypeWhitespace);
            return false;
        }
    }
    else
    {
        //done, nothing to see here, move along...
        return false;
    }
}

bool LILCodeParser::readSymbolCombinator()
{
    const char currentTokenChar = *(d->currentToken->getString()).data().c_str();
    switch (currentTokenChar)
    {
        case '=':
            d->receiver->receiveNodeData(ParserEventCombinatorSiblings, d->currentToken->getString());
            break;
        case '-':
            d->receiver->receiveNodeData(ParserEventCombinatorPreviousSiblings, d->currentToken->getString());
            break;
        case '+':
            d->receiver->receiveNodeData(ParserEventCombinatorNextSiblings, d->currentToken->getString());
            break;
        default:
            return false;
    }

    this->readNextToken();
    if (this->atEndOfSource())
        return false;

    if (!this->atEndOfSource())
        this->skip(TokenTypeWhitespace);

    return true;
}

bool LILCodeParser::readNameSelector(bool isNegating)
{
    LIL_START_NODE(NodeTypeSelector)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    d->receiver->receiveNodeData(ParserEventNameSelector, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readInstruction()
{
    LILString currentval;

    //set preference
    d->lexer->setHexPreferred(true);
    if (d->currentToken->isA(TokenTypeInstructionSign))
    {
        //skip the instruction sign
        d->receiver->receiveNodeData(ParserEventInstruction, d->currentToken->getString());
        this->readNextToken();
        if (atEndOfSource())
            return false;
    }

    //restore
    d->lexer->setHexPreferred(false);

    //we are looking at
    //if it starts with a number, we are looking at a color instruction
    if (d->currentToken->isA(TokenTypeHexNumber))
    {
        return this->readColor();
    }
    else if (d->currentToken->isA(TokenTypeIdentifier))
    {
        currentval = d->currentToken->getString();

        if (currentval == "needs")
        {
            return this->readNeedsInstr();
        }
        else if (currentval == "new")
        {
            return this->readNewInstr();
        }
        else if (currentval == "move")
        {
//            ret = std::shared_ptr<LILInstruction>(new LILInstruction(LILMoveInstruction, this));
//            if (d->notifiesReceiver)
//            {
//                //notify the receiver
//                d->receiver->receiveNodeData(ParserEventInstruction, ret);
//            }
//            this->readNextToken();
        }
        else if (currentval == "delete")
        {

//            this->readNextToken();
//            this->skip(TokenTypeWhitespace);
//            ret = std::shared_ptr<LILInstruction>(new LILInstruction(LILDeleteInstruction, this));
//            if (d->notifiesReceiver)
//            {
//                //notify the receiver
//                d->receiver->receiveNodeData(ParserEventInstruction, ret);
//            }
        }
        else if (currentval == "configure")
        {
//            ret = std::shared_ptr<LILInstruction>(new LILInstruction(LILConfigureInstruction, currentval, this));
//
//            if (d->notifiesReceiver)
//            {
//                //notify the receiver
//                d->receiver->receiveNodeData(ParserEventInstruction, ret);
//            }
//
//            this->readNextToken();
//            if (atEndOfSource())
//                return false;
//            this->skip(TokenTypeWhitespace);
//            if (atEndOfSource())
//                return false;
//
//            bool argValid = true;
//            std::shared_ptr<LILNode> arg = this->readExpression(argValid);
//            if (!argValid)
//                return false;
//
//            ret->setArgument(arg);
//
//            if (d->notifiesReceiver)
//            {
//                //notify the receiver
//                d->receiver->receiveNodeData(ParserEventConfiguration, ret);
//            }
//
//            if (atEndOfSource())
//                return ret;
//            this->skip(TokenTypeWhitespace);
//            if (atEndOfSource())
//                return ret;
//
//            if (d->currentToken->isA(TokenTypeSemicolon))
//            {
//                this->skip(TokenTypeSemicolon);
//                if (atEndOfSource())
//                    return ret;
//                this->skip(TokenTypeWhitespace);
//                if (atEndOfSource())
//                    return ret;
//            }
        }
        else
        {
            d->receiver->receiveError("Error: unknown instruction type "+currentval, d->file, d->line, d->column);
        }
    }
    return false;
}

bool LILCodeParser::readColor()
{
    LIL_START_NODE(NodeTypeInstruction)
    LILString currentval = d->currentToken->getString();
    switch (currentval.length())
    {
        case 1:
            d->receiver->receiveNodeData(ParserEventColorG1, currentval);
            break;
        case 2:
            d->receiver->receiveNodeData(ParserEventColorG2, currentval);
            break;
        case 3:
            d->receiver->receiveNodeData(ParserEventColorRGB, currentval);
            break;
        case 4:
            d->receiver->receiveNodeData(ParserEventColorRGBA, currentval);
            break;
        case 5:
            d->receiver->receiveNodeData(ParserEventColorRGBAA, currentval);
            break;
        case 6:
            d->receiver->receiveNodeData(ParserEventColorRRGGBB, currentval);
            break;
        case 7:
            d->receiver->receiveNodeData(ParserEventColorRRGGBBA, currentval);
            break;
        case 8:
            d->receiver->receiveNodeData(ParserEventColorRRGGBBAA, currentval);
            break;

        default:
            d->receiver->receiveNodeData(ParserEventInvalid, currentval);
            d->receiver->receiveError("Wrong length for hexadecimal number (should be between 1 and 8 digits long, inclusive)", d->file, d->line, d->column);
            return false;
    }

    this->readNextToken();

    LIL_END_NODE
}

bool LILCodeParser::readNeedsInstr()
{
    LIL_START_NODE(NodeTypeInstruction)
    d->receiver->receiveNodeData(ParserEventInstruction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (!d->currentToken->isA(TokenTypeDoubleQuoteString) && !d->currentToken->isA(TokenTypeSingleQuoteString)) {
        LIL_CANCEL_NODE
    }
    NodeType nodeType = NodeTypeInvalid;
    bool valid = this->readSingleValue(nodeType);
    if (valid) {
        d->receiver->receiveNodeCommit();
    }
    LIL_CHECK_FOR_END
    LIL_EXPECT(TokenTypeSemicolon, "semicolon");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE
}

bool LILCodeParser::readNewInstr()
{
    LIL_START_NODE(NodeTypeInstruction)
    d->receiver->receiveNodeData(ParserEventInstruction, d->currentToken->getString());

    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (d->currentToken->isA(TokenTypeObjectSign))
    {
        d->receiver->receiveNodeData(ParserEventObjectDefinition, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END

        LIL_EXPECT(TokenTypeIdentifier, "identifier")

        d->receiver->receiveNodeData(ParserEventVarName, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    if (d->currentToken->isA(TokenTypeParenthesisOpen))
    {
        //parenthesis open
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        LIL_EXPECT(TokenTypeNumberInt, "number");

        d->receiver->receiveNodeData(ParserEventNumberLiteral, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        //parenthesis close
        LIL_EXPECT(TokenTypeParenthesisClose, "parenthesis close")

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }
    LIL_END_NODE
}


bool LILCodeParser::readInstructionRule()
{
    d->lexer->setHexPreferred(true);

    d->receiver->receiveNodeData(ParserEventInstruction, d->currentToken->getString());
    this->readNextToken();
    if (this->atEndOfSource())
        return false;

    d->lexer->setHexPreferred(false);

    if (this->isColorInstruction())
    {
        return false;
    }

    LILString instructionName = d->currentToken->getString();
    if (instructionName == "new" || instructionName == "move")
    {
        this->readInstruction();
        this->readRule();
    }
    else if (instructionName == "delete")
    {
        this->readSelectorChains(TokenTypeSemicolon);
        if (atEndOfSource())
            return false;

        //we expect the end of the statement here
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        if (atEndOfSource())
            return false;

        //skip any whitespace
        this->skip(TokenTypeWhitespace);
        if (atEndOfSource())
            return false;
    }

    return true;
}

bool LILCodeParser::readValuePath(bool firstIsVar, bool allowFunctionCall)
{
    bool isFirst = true;

    bool done = false;

    LIL_START_NODE(NodeTypeValuePath);

    while (!done)
    {
        done = true;
        if (d->currentToken->isA(TokenTypeIdentifier))
        {
            if (isFirst && firstIsVar)
            {
                bool isValid = this->readVarName();
                if (isValid){
                    d->receiver->receiveNodeCommit();
                } else {
                    d->receiver->receiveError("Failed to read var name in value path", d->file, d->line, d->column);
                    this->skipInvalidToken();
                }
            }
            else
            {
                bool isValid = this->readPropertyName();
                if (isValid){
                    d->receiver->receiveNodeCommit();
                } else {
                    d->receiver->receiveError("Failed to read property name in value path", d->file, d->line, d->column);
                    this->skipInvalidToken();
                }
            }
        }
        if (this->isIndexAccessor())
        {
            bool iaValid = this->readIndexAccessor();
            if (iaValid)
            {
                d->receiver->receiveNodeCommit();
            } else {
                d->receiver->receiveError("Failed to read index accessor in value path", d->file, d->line, d->column);
                this->skipInvalidToken();
            }
        }
        if (this->isFunctionCall(true))
        {
            bool fcValid = this->readFunctionCallSimple();
            if (fcValid) {
                d->receiver->receiveNodeCommit();
            } else {
                d->receiver->receiveError("Failed to read function call in value path", d->file, d->line, d->column);
                this->skipInvalidToken();
            }
        }
        if (!atEndOfSource() && d->currentToken->isA(TokenTypeDot))
        {
            done = false;
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END
        }

        isFirst = false;
    }

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readVarName()
{
    LIL_START_NODE(NodeTypeVarName)
    d->receiver->receiveNodeData(ParserEventVarName, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readPropertyName()
{
    LIL_START_NODE(NodeTypePropertyName)
    d->receiver->receiveNodeData(ParserEventPropertyName, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readIndexAccessor()
{
    LIL_START_NODE(NodeTypeIndexAccessor)
    LIL_EXPECT(TokenTypeSquareBracketOpen, "opening square bracket")

    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());

    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool outIsSV = false;
    NodeType svExpTy = NodeTypeInvalid;
    bool valueValid = this->readExpression(outIsSV, svExpTy);
    if (valueValid)
    {
        d->receiver->receiveNodeCommit();
    } else {
        LIL_CANCEL_NODE
    }

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeSquareBracketClose, "closing square bracket")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readSelectorChains(TokenType stopOn)
{
    bool done = false;

    bool allValid = true;
    while (!done)
    {
        done = true;
        bool valid = this->readSelectorChain(stopOn);
        if (valid) {
            d->receiver->receiveNodeCommit();
        } else {
            allValid = false;
        }
        if (atEndOfSource()) {
            return false;
        }
        if (d->currentToken->isA(TokenTypeComma))
        {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            if (!atEndOfSource()) {
                this->skip(TokenTypeWhitespace);
            }
            if (atEndOfSource()) {
                return allValid;
            }
            done = false;
        }
    }
    return allValid;
}

bool LILCodeParser::readSelectorChain(TokenType stopOn)
{
    LIL_START_NODE(NodeTypeSelectorChain)
    bool done = false;
    while (!done)
    {
        bool ssValid = this->readSimpleSelector();
        if (ssValid) {
            d->receiver->receiveNodeCommit();
        }
        if (!ssValid || this->atEndOfSource()){
            LIL_CANCEL_NODE
        }

        bool childComValid = this->readChildrenCombinatorOrSkip();
        if (childComValid){
            d->receiver->receiveNodeCommit();
        }
        LIL_CHECK_FOR_END

        bool otherComValid = this->readCombinator();
        if (otherComValid) {
            d->receiver->receiveNodeCommit();
        }

        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        if (
            d->currentToken->isA(TokenTypeComma)
            || d->currentToken->isA(stopOn)
            || d->currentToken->isA(TokenTypeSemicolon)
        ) {
            done = true;
        }
    }
    LIL_END_NODE
}

//this function assumes the comparison sign has already been checked
bool LILCodeParser::readComparison()
{
    LIL_START_NODE(NodeTypeExpression)

    TokenType tokenType = d->currentToken->getType();

    if (tokenType == TokenTypeNegator)
    {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END
        LIL_EXPECT(TokenTypeEqualSign, "equal sign")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }
    else
    {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    bool outIsSV = false;
    NodeType svExpTy = NodeTypeInvalid;
    bool rightNodeValid = this->readExpression(outIsSV, svExpTy);
    if(!rightNodeValid)
        return false;

    LIL_END_NODE
}

bool LILCodeParser::readFunctionDecl()
{
    bool valid = true;
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        LILString name = d->currentToken->getString();

        if (name == "fn")
        {
            this->readFnFunction();
        }
        // <fnName> (selector) { propery: value }
        else if (name == "override")
        {
            this->readOverrideFunction();
        }
        else if (name == "insert")
        {
            this->readInsertFunction();
        }
        else if (name == "macro")
        {
//            this->readMacroFunction();
        }

        else
        {
            valid = false;
        }
    }
    else
    {
        valid = false;
    }

    return valid;
}

bool LILCodeParser::readFnFunction()
{
    LIL_START_NODE(NodeTypeFunctionDecl)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "fn"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //open parenthesis
    bool needsParenthesisClose  = false;
    if (d->currentToken->isA(TokenTypeParenthesisOpen)){
        needsParenthesisClose = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        if (!d->currentToken->isA(TokenTypeParenthesisClose))
        {
            bool valueValid = true;
            bool argumentsDone = false;
            while (!argumentsDone && !d->currentToken->isA(TokenTypeParenthesisClose))
            {
                argumentsDone = true;
                NodeType nodeType = NodeTypeInvalid;
                bool svValid = this->readSingleValue(nodeType);
                if (svValid) {
                    d->receiver->receiveNodeCommit();
                }
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

                if (d->currentToken->isA(TokenTypeSemicolon))
                {
                    argumentsDone = false;
                    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                    this->readNextToken();
                    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
                }
                if(!svValid) valueValid = false;
            }
            if (!valueValid)
                LIL_CANCEL_NODE
                }

        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    //close parenthesis
    if (needsParenthesisClose){
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    if (d->currentToken->isA(TokenTypeFatArrow)) {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        bool tyValid = this->readType();
        if (tyValid) {
            d->receiver->receiveNodeCommit();
        }
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the inner part of the block
    //readEvaluables auto commits
    this->readEvaluables();

    if (!this->atEndOfSource())
    {
        //block close
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}


bool LILCodeParser::readOverrideFunction()
{
    LIL_START_NODE(NodeTypeFunctionCall)

    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "override"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //if shorthand notation
    if (d->currentToken->isA(TokenTypeParenthesisClose))
    {
        d->receiver->receiveNodeData(ParserEventFunctionShorthand, "");

        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeBlockOpen, "block open");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool valid = true;
    while (!this->atEndOfSource() && valid && !d->currentToken->isA(TokenTypeBlockClose)) {
        valid = this->readAssignment(true, false);
        if (valid) {
            d->receiver->receiveNodeCommit();
        }
    }
    if (!this->atEndOfSource())
    {
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readInsertFunction()
{
    LIL_START_NODE(NodeTypeFunctionCall)

    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "insert"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //readVals auto commits
    bool valueValid = this->readVals();
    if (!valueValid){
        LIL_CANCEL_NODE
    }

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //if shorthand notation
    if (d->currentToken->isA(TokenTypeParenthesisClose))
    {
        d->receiver->receiveNodeData(ParserEventFunctionShorthand, "");
    }
    else
    {
        if (!d->currentToken->isA(TokenTypeIdentifier) || d->currentToken->getString() != "of")
        {
            d->receiver->receiveError("Unexpected token while reading flagging function", d->file, d->line, d->column);
            LIL_CANCEL_NODE
        }

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        //now read the selector chain
        bool scValid = this->readSelectorChains(TokenTypeParenthesisClose);
        if (scValid) {
            d->receiver->receiveNodeCommit();
        }
        LIL_CHECK_FOR_END
    }

    LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readEvaluables()
{
    bool ret = true;
    bool evalsDone = false;
    while (!evalsDone && ret == true)
    {
        evalsDone = true;
        if (d->currentToken->isA(TokenTypeIdentifier))
        {
            //this is either a function or an object name
            LILString valuestr = d->currentToken->getString();

            if (valuestr == "var")
            {
                bool vdValid = this->readVarDecl();
                if (vdValid) {
                    d->receiver->receiveNodeCommit();
                } else {
                    d->receiver->receiveError(LILString::format("Error while reading var declaration on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                }
                if (this->atEndOfSource())
                    return ret;
                this->skip(TokenTypeWhitespace);
                if (this->atEndOfSource())
                    return ret;
                if (d->currentToken->isA(TokenTypeSemicolon))
                {
                    evalsDone = false;
                    //skip the semicolon
                    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                    this->readNextToken();
                    if (this->atEndOfSource())
                        return false;

                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                }
                else if (!d->currentToken->isA(TokenTypeBlockClose))
                {
                    d->receiver->receiveError(LILString::format("Unexpected token while reading evaluables on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                    this->skipInvalidToken();
                    evalsDone = false;
                }
            }
            else
            {
                //check if it is a function declaration
                if (this->isFunctionDecl())
                {
                    bool fdValid = this->readFunctionDecl();
                    if (fdValid) {
                        d->receiver->receiveNodeCommit();
                    } else {
                        d->receiver->receiveError(LILString::format("Error while reading function on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                    }
                    if (this->atEndOfSource())
                        return ret;
                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                    if (d->currentToken->isA(TokenTypeSemicolon))
                    {
                        evalsDone = false;
                        //skip the semicolon
                        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                        this->readNextToken();
                        if (this->atEndOfSource())
                            return false;

                        this->skip(TokenTypeWhitespace);
                        if (this->atEndOfSource())
                            return ret;
                    }
                    else if (!d->currentToken->isA(TokenTypeBlockClose))
                    {
                        d->receiver->receiveError(LILString::format("Unexpected token while reading evaluables on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                        this->skipInvalidToken();
                        evalsDone = false;
                    }
                }
                if (this->isFlowControl())
                {
                    bool fcValid = this->readFlowControl();
                    if (fcValid) {
                        d->receiver->receiveNodeCommit();
                    } else {
                        d->receiver->receiveError(LILString::format("Error while reading flow control on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                    }
                    if (this->atEndOfSource())
                        return ret;
                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                    evalsDone = false;
                }
                if (this->isFlowControlCall())
                {
                    bool fccValid = this->readFlowControlCall();
                    if (fccValid) {
                        d->receiver->receiveNodeCommit();
                    } else {
                        d->receiver->receiveError(LILString::format("Error while reading flow control call on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                    }
                    if (this->atEndOfSource())
                        return ret;
                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                    evalsDone = false;
                }
                else if (this->isFunctionCall(false))
                {
                    bool fcValid = this->readFunctionCall();
                    if (fcValid) {
                        d->receiver->receiveNodeCommit();
                    } else {
                        d->receiver->receiveError(LILString::format("Error while reading function on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                    }
                    if (this->atEndOfSource())
                        return ret;
                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                    if (d->currentToken->isA(TokenTypeSemicolon))
                    {
                        evalsDone = false;
                        //skip the semicolon
                        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                        this->readNextToken();
                        if (this->atEndOfSource())
                            return false;

                        this->skip(TokenTypeWhitespace);
                        if (this->atEndOfSource())
                            return ret;
                    }
                    else if (!d->currentToken->isA(TokenTypeBlockClose))
                    {
                        d->receiver->receiveError(LILString::format("Unexpected token while reading evaluables on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                        this->skipInvalidToken();
                        evalsDone = false;
                    }
                }

                else if (this->isValuePath())
                {
                    bool ppValid = this->readValuePath(true, true);
                    if (ppValid) {
                        d->receiver->receiveNodeCommit();
                    }
                    if (this->isAssignment())
                    {
                        bool asgmValid = this->readAssignment(true, true, true);
                        if (asgmValid) {
                            d->receiver->receiveNodeCommit();
                        } else {
                            d->receiver->receiveError(LILString::format("Error while reading function on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                        }
                    }
                    else if (this->isExpression())
                    {
                        bool outIsSV = false;
                        NodeType outType;
                        bool expValid = this->readExpression(outIsSV, outType);
                        if (expValid) {
                            d->receiver->receiveNodeCommit();
                        }
                        else
                        {
                            d->receiver->receiveError(LILString::format("Error while reading expression on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                        }
                    }
                    if (this->atEndOfSource())
                        return ret;
                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                    if (d->currentToken->isA(TokenTypeSemicolon))
                    {
                        evalsDone = false;
                        //skip the semicolon
                        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                        this->readNextToken();
                        if (this->atEndOfSource())
                            return false;

                        this->skip(TokenTypeWhitespace);
                        if (this->atEndOfSource())
                            return ret;
                    }
                    else if (!d->currentToken->isA(TokenTypeBlockClose))
                    {
                        d->receiver->receiveError(LILString::format("Unexpected token while reading evaluables on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                        this->skipInvalidToken();
                        evalsDone = false;
                    }
                }
                else if (this->isAssignment())
                {
                    bool asgmtValid = this->readAssignment(true, true);
                    if (asgmtValid) {
                        d->receiver->receiveNodeCommit();
                    } else {
                        d->receiver->receiveError(LILString::format("Error while reading assignment on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                    }
                    if (this->atEndOfSource())
                        return ret;
                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                    if (d->currentToken->isA(TokenTypeSemicolon))
                    {
                        evalsDone = false;
                        //skip the semicolon
                        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                        this->readNextToken();
                        if (this->atEndOfSource())
                            return false;

                        this->skip(TokenTypeWhitespace);
                        if (this->atEndOfSource())
                            return ret;
                    }
                    else if (!d->currentToken->isA(TokenTypeBlockClose))
                    {
                        d->receiver->receiveError(LILString::format("Unexpected token while reading evaluables on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                        this->skipInvalidToken();
                        evalsDone = false;
                    }
                }
                else if (d->currentToken->isA(TokenTypeIdentifier))
                {
                    bool ppValid = this->readValuePath(true, true);
                    if (ppValid) {
                        d->receiver->receiveNodeCommit();
                    }
                    if (this->atEndOfSource())
                        return ret;
                    this->skip(TokenTypeWhitespace);
                    if (this->atEndOfSource())
                        return ret;
                    if (d->currentToken->isA(TokenTypeSemicolon))
                    {
                        evalsDone = false;
                        //skip the semicolon
                        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                        this->readNextToken();
                        if (this->atEndOfSource())
                            return false;

                        this->skip(TokenTypeWhitespace);
                        if (this->atEndOfSource())
                            return ret;
                    }
                    else if (!d->currentToken->isA(TokenTypeBlockClose))
                    {
                        d->receiver->receiveError(LILString::format("Unexpected token while reading evaluables on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                        this->skipInvalidToken();
                        evalsDone = false;
                    }
                }
            }
        }
        else if (d->currentToken->isA(TokenTypeObjectSign))
        {
            bool opValid = this->readObjectPath();
            if (!opValid) {
                this->skipInvalidToken();
                continue;
            }
            if (this->isAssignment())
            {
                bool asgmtValid = this->readAssignment(true, true, true);
                if (asgmtValid)
                {
                    d->receiver->receiveNodeCommit();
                }
                else
                {
                    d->receiver->receiveError(LILString::format("Error while reading assignment on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                }
            }
            else if (this->isExpression())
            {
                bool outIsSV = false;
                NodeType outType;
                bool expValid = this->readExpression(outIsSV, outType);
                if (expValid) {
                    d->receiver->receiveNodeCommit();
                }
                else
                {
                    d->receiver->receiveError(LILString::format("Error while reading expression on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                }
            } else {
                d->receiver->receiveNodeCommit();
            }
            if (this->atEndOfSource())
                return ret;
            this->skip(TokenTypeWhitespace);
            if (this->atEndOfSource())
                return ret;
            if (d->currentToken->isA(TokenTypeSemicolon))
            {
                evalsDone = false;
                //skip the semicolon
                d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                this->readNextToken();
                if (this->atEndOfSource())
                    return false;

                this->skip(TokenTypeWhitespace);
                if (this->atEndOfSource())
                    return ret;
            }
            else if (!d->currentToken->isA(TokenTypeBlockClose))
            {
                d->receiver->receiveError(LILString::format("Unexpected token while reading evaluables on line %d and column %d", d->line, d->column), d->file, d->line, d->column);
                this->skipInvalidToken();
                evalsDone = false;
            }
        }
    }
    return ret;
}

bool LILCodeParser::readFunctionCall()
{
    bool valid = true;
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        LILString name = d->currentToken->getString();

        // <fnName>(name of selector)
        if (
            name == "ref"
            || name == "attr"
            || this->isFlag()
            ){
            this->readNameAndSelectorFunctionCall();
        }
        // <fnName> (selector)
        else if (name == "sel")
        {
            this->readSelFunction();
        }
        else if (name == "pointerTo")
        {
            this->readSingleArgumentFunctionCall("pointerTo");
        }
        else if (name == "valueOf")
        {
            this->readSingleArgumentFunctionCall("valueOf");
        }
        // <fnName>(arg1, arg2, argN, ...)
        else
        {
            LIL_START_NODE(NodeTypeValuePath)
            bool vnValid = this->readVarName();
            if (vnValid) {
                d->receiver->receiveNodeCommit();
            } else {
                LIL_CANCEL_NODE
            }
            LIL_CHECK_FOR_END
            bool fcValid = this->readStandardFunctionCall();
            if (fcValid) {
                d->receiver->receiveNodeCommit();
            } else {
                LIL_CANCEL_NODE
            }
            LIL_END_NODE_SKIP(false)
        }
    }
    else
    {
        valid = false;
    }

    return valid;
}

bool LILCodeParser::readFunctionCallSimple()
{
    LIL_START_NODE(NodeTypeFunctionCall)
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END
    }

    //open parenthesis
    bool needsParenthesisClose  = false;
    if (d->currentToken->isA(TokenTypeParenthesisOpen)){
        needsParenthesisClose = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }
    else if (d->currentToken->isA(TokenTypeWhitespace)){
        this->skip(TokenTypeWhitespace);
        if (atEndOfSource())
            LIL_CANCEL_NODE
            } else {
                LIL_CANCEL_NODE
            }

    bool valueValid = true;
    if (needsParenthesisClose)
    {
        LIL_CHECK_FOR_END

        //multiple arguments mode
        bool argumentsDone = false;

        while (
               ! argumentsDone
               && valueValid
               && !d->currentToken->isA(TokenTypeParenthesisClose)
        ){
            argumentsDone = true;
            
            if (this->isAssignment()) {
                bool asgmValid = this->readAssignment(false, true);
                if (asgmValid) {
                    d->receiver->receiveNodeCommit();
                } else {
                    this->skipInvalidToken();
                    continue;
                }
            }
            else
            {
                bool outIsSV = false;
                NodeType svExpTy = NodeTypeInvalid;
                bool isValid = this->readExpression(outIsSV, svExpTy);
                if (isValid) {
                    d->receiver->receiveNodeCommit();
                } else {
                    valueValid = false;
                }
            }
            if (d->currentToken->isA(TokenTypeComma) || d->currentToken->isA(TokenTypeSemicolon))
            {
                argumentsDone = false;
                d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                this->readNextToken();
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }
        }
        if (!valueValid)
            LIL_CANCEL_NODE


            } else {
                //single argument mode
                bool outIsSV = false;
                NodeType svExpTy = NodeTypeInvalid;
                bool isValid = this->readExpression(outIsSV, svExpTy);
                if (isValid){
                    d->receiver->receiveNodeCommit();
                } else {
                    valueValid = false;
                }

                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }

    //close parenthesis
    if (needsParenthesisClose)
    {
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END
    }

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readStandardFunctionCall()
{
    LIL_START_NODE(NodeTypeFunctionCall)

    //open parenthesis
    bool needsParenthesisClose  = false;
    if (d->currentToken->isA(TokenTypeParenthesisOpen)){
        needsParenthesisClose = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }
    else if (d->currentToken->isA(TokenTypeWhitespace))
    {
        this->skip(TokenTypeWhitespace);
        LIL_CHECK_FOR_END
    }

    //read the arguments
    bool done = false;
    while (!done)
    {
        done = true;
        bool outIsSV = false;
        NodeType svExpTy = NodeTypeInvalid;
        bool valueValid = this->readExpression(outIsSV, svExpTy);
        if (valueValid){
            d->receiver->receiveNodeCommit();
        } else {
            LIL_CANCEL_NODE
        }

        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        if (d->currentToken->isA(TokenTypeComma))
        {
            done = false;
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
        }
    }

    //close parenthesis
    if (needsParenthesisClose) {
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END
    }

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readNameAndSelectorFunctionCall()
{
    LIL_START_NODE(NodeTypeFunctionCall)

    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //open parenthesis
    LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the arguments
    LIL_EXPECT(TokenTypeIdentifier, "identifier");
    bool vpValid = this->readValuePath(true, false);
    if (vpValid) {
        d->receiver->receiveNodeCommit();
    }

    this->skip(TokenTypeWhitespace);
    if (atEndOfSource())
        return false;

    //if shorthand notation -- assumes 'of @this'
    if (d->currentToken->isA(TokenTypeParenthesisClose))
    {
        d->receiver->receiveNodeData(ParserEventFunctionShorthand, "");

        //close parenthesis
        if (!d->currentToken->isA(TokenTypeParenthesisClose))
            return false;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        if (atEndOfSource())
            return false;

        this->skip(TokenTypeWhitespace);
        if (atEndOfSource())
            return false;
    }
    else
    {
        if (!d->currentToken->isA(TokenTypeIdentifier) || d->currentToken->getString() != "of")
        {
            return false;
        }

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());

        this->readNextToken();
        if (atEndOfSource())
            return false;

        this->skip(TokenTypeWhitespace);
        if (atEndOfSource())
            return false;

        this->readSelectorChains(TokenTypeParenthesisClose);
        if (atEndOfSource())
            return false;

        //close parenthesis
        if (!d->currentToken->isA(TokenTypeParenthesisClose))
            return false;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        if (!atEndOfSource())
            this->skip(TokenTypeWhitespace);

        this->skip(TokenTypeWhitespace);
        if (atEndOfSource())
            return false;
    }

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readSelFunction()
{
    LIL_START_NODE(NodeTypeFunctionCall)

    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool scValid = this->readSelectorChains(TokenTypeParenthesisClose);
    if (scValid) {
        d->receiver->receiveNodeCommit();
    }

    LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_END_NODE
}

bool LILCodeParser::readFlagFunction()
{
    LIL_START_NODE(NodeTypeFunctionCall)

    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "flag"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //open parenthesis
    LIL_EXPECT(TokenTypeParenthesisOpen, "open parenthesis")
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the arguments
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        d->receiver->receiveNodeData(ParserEventFlag, d->currentToken->getString());
    }
    else if (d->currentToken->isA(TokenTypeAsterisk))
    {
        d->receiver->receiveNodeData(ParserEventFlag, d->currentToken->getString());
    }
    else
    {
        d->receiver->receiveError("Unexpected token while reading flagging function " + d->currentToken->getString(), d->file, d->line, d->column);
        LIL_CANCEL_NODE
    }

    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LILString name = d->currentToken->getString();
    if (name == "replaceFlag" && d->currentToken->isA(TokenTypeIdentifier))
    {
        if (d->currentToken->getString() != "with")
        {
            d->receiver->receiveError("Unexpected token while reading flagging function", d->file, d->line, d->column);
            LIL_CANCEL_NODE
        }

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());

        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        if (d->currentToken->isA(TokenTypeIdentifier))
        {
            d->receiver->receiveNodeData(ParserEventFlag, d->currentToken->getString());
        }
        else
        {
            d->receiver->receiveError("Unexpected token while reading flagging function", d->file, d->line, d->column);
            LIL_CANCEL_NODE
        }

        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    //if shorthand notation
    if (d->currentToken->isA(TokenTypeParenthesisClose))
    {
        //d->receiver->receiveNodeData(ParserEventShorthand, d->currentToken->getString());

        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END
    }
    else
    {
        if (!d->currentToken->isA(TokenTypeIdentifier) || d->currentToken->getString() != "of")
        {
            d->receiver->receiveError("Unexpected token while reading flagging function", d->file, d->line, d->column);
            LIL_CANCEL_NODE
        }

        //of
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());

        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        if (d->currentToken->isA(TokenTypeSingleQuoteString) || d->currentToken->isA(TokenTypeDoubleQuoteString)) {
            //read string selechtor chains
        }
        else
        {
            //now read the selector chain
            bool scValid = this->readSelectorChains(TokenTypeParenthesisClose);
            if (scValid) {
                d->receiver->receiveNodeCommit();
            }
            LIL_CHECK_FOR_END
        }

        //close parenthesis
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readSingleArgumentFunctionCall(const LILString & name)
{
    LIL_START_NODE(NodeTypeFunctionCall)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != name){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END

    //parentheses are optional
    bool needsClosingParenthesis = false;
    if (d->currentToken->isA(TokenTypeParenthesisOpen))
    {
        needsClosingParenthesis = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }
    else if (d->currentToken->isA(TokenTypeSemicolon))
    {
        LIL_END_NODE
    }
    else if (d->currentToken->isA(TokenTypeWhitespace))
    {
        this->skip(TokenTypeWhitespace);
        LIL_CHECK_FOR_END
    }

    NodeType outNodeType;
    bool svValid = this->readSingleValue(outNodeType);
    if (svValid) {
        d->receiver->receiveNodeCommit();
    } else {
        LIL_CANCEL_NODE
    }

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (needsClosingParenthesis)
    {
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readFlowControl()
{
    bool valid = true;
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        LILString name = d->currentToken->getString();

        if (name == "if")
        {
            this->readIfFlowControl();
        }
        else if (name == "switch")
        {
            this->readSwitchFlowControl();
        }
        else if (name == "loop")
        {
            this->readLoopFlowControl();
        }
        else if (name == "for")
        {
            this->readForFlowControl();
        }
        else if (name == "finally")
        {
            this->readFinallyFlowControl();
        }

        else
        {
            valid = false;
        }
    }
    else
    {
        valid = false;
    }

    return valid;
}


bool LILCodeParser::readIfFlowControl()
{
    LIL_START_NODE(NodeTypeFlowControl)

    if (this->isIfIs()) {
        LIL_EXPECT(TokenTypeIdentifier, "identifier")
        if (d->currentToken->getString() != "if"){
            LIL_CANCEL_NODE
        }
        d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        LIL_EXPECT(TokenTypeIdentifier, "identifier")
        bool varNameValid = this->readVarName();
        if (varNameValid) {
            d->receiver->receiveNodeCommit();
        } else {
            LIL_CANCEL_NODE
        }
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        LIL_EXPECT(TokenTypeIdentifier, "identifier")
        if (d->currentToken->getString() != "is"){
            LIL_CANCEL_NODE
        }
        d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        bool tyValid = this->readType();
        if (tyValid) {
            d->receiver->receiveNodeCommit();
        }
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    } else {
        LIL_EXPECT(TokenTypeIdentifier, "identifier")
        if (d->currentToken->getString() != "if"){
            LIL_CANCEL_NODE
        }
        d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        //open parenthesis
        bool needsParenthesisClose  = false;
        if (d->currentToken->isA(TokenTypeParenthesisOpen)){
            needsParenthesisClose = true;
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
        }

        bool outIsSV = false;
        NodeType svExpTy = NodeTypeInvalid;
        bool conditionValid = this->readExpression(outIsSV, svExpTy);
        if (conditionValid)
        {
            d->receiver->receiveNodeCommit();
        }
        else
        {
            return false;
        }

        //close parenthesis
        if (needsParenthesisClose){
            LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
        }
    }

    //block open
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    if (d->currentToken->isA(TokenTypeBlockOpen)){
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        //read the inner part of the block
        d->receiver->receiveNodeData(ParserEventFunctionBody, "");
        this->readEvaluables();

        if (!this->atEndOfSource())
        {
            //block close
            LIL_EXPECT(TokenTypeBlockClose, "block close")
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
        }
    }
    else
    {
        bool outIsSingleValue = false;
        NodeType outType;
        bool expValid = this->readExpression(outIsSingleValue, outType);
        if (expValid)
            d->receiver->receiveNodeCommit();
    }

    if (d->currentToken->isA(TokenTypeIdentifier)) {
        if (d->currentToken->getString() == "else") {
            d->receiver->receiveNodeData(ParserEventFlowControlElse, "");
            d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

            bool needsBlockClose  = false;
            if (d->currentToken->isA(TokenTypeBlockOpen)){
                needsBlockClose = true;
                d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                this->readNextToken();
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }

            this->readEvaluables();

            if (needsBlockClose){
                LIL_EXPECT(TokenTypeBlockClose, "block close")
                d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
                this->readNextToken();
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }
        }
    }

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readSwitchFlowControl()
{
    LIL_START_NODE(NodeTypeFlowControl)
    if (d->currentToken->getString() != "switch"){
        LIL_CANCEL_NODE
    }
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //parentheses are optional
    bool needsClosingParenthesis = false;
    if (d->currentToken->isA(TokenTypeParenthesisOpen))
    {
        needsClosingParenthesis = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    bool outIsSV = false;
    NodeType svExpTy = NodeTypeInvalid;
    bool conditionValid = this->readExpression(outIsSV, svExpTy);
    if (conditionValid) {
        d->receiver->receiveNodeCommit();
    } else {
        LIL_CANCEL_NODE
    }

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (needsClosingParenthesis)
    {
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (!d->currentToken->isA(TokenTypeIdentifier) ||
        (d->currentToken->getString() != "case" && d->currentToken->getString() != "default")
        )
    {
        d->receiver->receiveError("Expected case or default.", d->file, d->line, d->column);
        LIL_CANCEL_NODE
    }
    bool casesDone = false;
    bool caseValid = false;
    while (!casesDone)
    {
        casesDone = true;
        if (d->currentToken->getString() == "default") {
            caseValid = this->readSwitchDefault();
        } else if (d->currentToken->getString() == "case") {
            caseValid = this->readSwitchCase();
        } else {
            d->receiver->receiveError("Expected case or default.", d->file, d->line, d->column);
            LIL_CANCEL_NODE
        }

        if (caseValid) {
            d->receiver->receiveNodeCommit();
        }

        LIL_CHECK_FOR_END

        if (d->currentToken->isA(TokenTypeSemicolon))
        {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
        }

        //moar cases?
        if (d->currentToken->isA(TokenTypeIdentifier) &&
            (d->currentToken->getString() == "case" || d->currentToken->getString() == "default")
            )
        {
            casesDone = false;
        }
    }

    if (!this->atEndOfSource())
    {
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readSwitchDefault()
{
    LIL_START_NODE(NodeTypeFlowControl)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "default"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the inner part of the block
    //readEvaluables auto commits
    bool evaluablesValid = this->readEvaluables();
    if (!evaluablesValid){
        LIL_CANCEL_NODE
    }
    if (!this->atEndOfSource())
    {
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readSwitchCase()
{
    LIL_START_NODE(NodeTypeFlowControl)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "case"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    bool casesDone = false;
    while (!casesDone)
    {
        casesDone = true;

        bool caseValValid = false;
        NodeType nodeType = NodeTypeInvalid;
        caseValValid = this->readSingleValue(nodeType);
        if (caseValValid) {
            d->receiver->receiveNodeCommit();
        } else {
            LIL_CANCEL_NODE
        }

        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        if (d->currentToken->isA(TokenTypeComma))
        {
            casesDone = false;
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

            if (d->currentToken->getString() == "case")
            {
                d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
                this->readNextToken();
                LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
            }
        }
    }

    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //read the inner part of the block
    //readEvaluables auto commits
    bool evaluablesValid = this->readEvaluables();
    if (!evaluablesValid){
        LIL_CANCEL_NODE
    }

    if (!this->atEndOfSource())
    {
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readLoopFlowControl()
{
    LIL_START_NODE(NodeTypeFlowControl)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "loop"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    this->readEvaluables();

    if (!this->atEndOfSource())
    {
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readForFlowControl()
{
    LIL_START_NODE(NodeTypeFlowControl)
    if (d->currentToken->getString() != "for"){
        LIL_CANCEL_NODE
    }
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    //parentheses are optional
    bool needsClosingParenthesis = false;
    if (d->currentToken->isA(TokenTypeParenthesisOpen))
    {
        needsClosingParenthesis = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }

    bool outIsSV = false;
    NodeType svExpTy = NodeTypeInvalid;
    bool svValid = this->readExpression(outIsSV, svExpTy);
    if (svValid) {
        d->receiver->receiveNodeCommit();
    } else {
        LIL_CANCEL_NODE
    }

    if (needsClosingParenthesis && d->currentToken->isA(TokenTypeSemicolon))
    {
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        //middle expr was empty
        if (d->currentToken->isA(TokenTypeSemicolon))
        {
            d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
            this->readNextToken();
            LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
        }
        else
        {
            bool outIsSV2 = false;
            NodeType sv2ExpTy = NodeTypeInvalid;
            bool sv2Valid = this->readExpression(outIsSV2, sv2ExpTy);
            if (sv2Valid) {
                d->receiver->receiveNodeCommit();
            } else {
                LIL_CANCEL_NODE
            }
            if (!d->currentToken->isA(TokenTypeSemicolon))
            {
                d->receiver->receiveError("Expected semicolon", d->file, d->line, d->column);
                this->skipInvalidToken();
            }
        }

        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

        if (!d->currentToken->isA(TokenTypeParenthesisClose))
        {
            bool outIsSV2 = false;
            NodeType sv2ExpTy = NodeTypeInvalid;
            bool sv2Valid = this->readExpression(outIsSV2, sv2ExpTy);
            if (sv2Valid) {
                d->receiver->receiveNodeCommit();
            } else {
                LIL_CANCEL_NODE
            }
        }
    }

    if (needsClosingParenthesis)
    {
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    this->readEvaluables();

    if (!this->atEndOfSource())
    {
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readFinallyFlowControl()
{
    LIL_START_NODE(NodeTypeFlowControl)
    if (d->currentToken->getString() != "finally"){
        LIL_CANCEL_NODE
    }
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_EXPECT(TokenTypeBlockOpen, "block open")
    d->receiver->receiveNodeData(ParserEventFunctionBody, "");
    d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    this->readEvaluables();

    if (!this->atEndOfSource())
    {
        LIL_EXPECT(TokenTypeBlockClose, "block close")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readFlowControlCall()
{
    bool valid = true;
    if (d->currentToken->isA(TokenTypeIdentifier))
    {
        LILString name = d->currentToken->getString();

        if (name == "return")
        {
            this->readReturnFlowControlCall();
        }
        else if (name == "repeat")
        {
            this->readRepeatFlowControlCall();
        }
        else if (name == "break")
        {
            this->readBreakFlowControlCall();
        }
        else if (name == "continue")
        {
            this->readContinueFlowControlCall();
        }
        else
        {
            valid = false;
        }
    }
    else
    {
        valid = false;
    }

    return valid;
}

bool LILCodeParser::readReturnFlowControlCall()
{
    LIL_START_NODE(NodeTypeFlowControlCall)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "return"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END

    //parentheses are optional
    bool needsClosingParenthesis = false;
    if (d->currentToken->isA(TokenTypeParenthesisOpen))
    {
        needsClosingParenthesis = true;
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
        LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE
    }
    else if (d->currentToken->isA(TokenTypeSemicolon))
    {
        LIL_END_NODE
    }
    else if (d->currentToken->isA(TokenTypeWhitespace))
    {
        this->skip(TokenTypeWhitespace);
        LIL_CHECK_FOR_END
    }

    //read the return value
    //readVals auto commits
    this->readVals();

    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    if (needsClosingParenthesis)
    {
        LIL_EXPECT(TokenTypeParenthesisClose, "closing parenthesis")
        d->receiver->receiveNodeData(ParserEventPunctuation, d->currentToken->getString());
        this->readNextToken();
    }
    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readRepeatFlowControlCall()
{
    LIL_START_NODE(NodeTypeFlowControlCall)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "repeat"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readBreakFlowControlCall()
{
    LIL_START_NODE(NodeTypeFlowControlCall)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "break"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_END_NODE_SKIP(false)
}

bool LILCodeParser::readContinueFlowControlCall()
{
    LIL_START_NODE(NodeTypeFlowControlCall)
    LIL_EXPECT(TokenTypeIdentifier, "identifier")
    if (d->currentToken->getString() != "continue"){
        LIL_CANCEL_NODE
    }
    d->receiver->receiveNodeData(ParserEventFunction, d->currentToken->getString());
    this->readNextToken();
    LIL_CHECK_FOR_END_AND_SKIP_WHITESPACE

    LIL_END_NODE_SKIP(false)
}
