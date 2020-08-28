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
 *      This file encapsulates a token that contains a string
 *
 ********************************************************************/

#ifndef LILSTRINGTOKEN_H
#define LILSTRINGTOKEN_H

#include "LILToken.h"

namespace LIL
{
    class LILNode;

    class LILStringToken : public LILToken
    {
    public:
        LILStringToken(TokenType type, size_t line, size_t column, size_t index);
        virtual ~LILStringToken();
        void setValue(LILString newValue);

        bool equals(TokenType otherType, LILString otherValue);
        LILString toString();

        void setHasArguments(bool newValue);
        bool hasArguments() const;

        void addArgument(std::shared_ptr<LILNode> parserNode);
        const std::vector<std::shared_ptr<LILNode> > & getArguments() const;
        void addIndex(size_t index);
        const std::vector<size_t> & getIndexes() const;

    private:
        bool _hasArguments;
        std::vector<std::shared_ptr<LILNode> > _arguments;
        std::vector<size_t> _indexes;
    };
}

#endif
