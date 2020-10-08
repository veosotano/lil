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
 *      This file encapsulates the type for a function
 *
 ********************************************************************/

#ifndef LILFUNCTIONTYPE_H
#define LILFUNCTIONTYPE_H

#include "LILType.h"

namespace LIL
{
    class LILVarDecl;
    class LILFunctionType : public LILType
    {
    public:
        static std::shared_ptr<LILFunctionType> make(LILString returnTypeName);
        LILFunctionType();
        LILFunctionType(const LILFunctionType &other);
        std::shared_ptr<LILFunctionType> clone() const;
        virtual ~LILFunctionType();
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        virtual void receiveNodeData(const LILString & data);
        
        LILString stringRep();
        
        void addArgument(std::shared_ptr<LILNode> node);
        void prependArgument(std::shared_ptr<LILNode> node);
        std::vector<std::shared_ptr<LILNode>> getArguments() const;
        void setReturnType(std::shared_ptr<LILType> node);
        std::shared_ptr<LILType> getReturnType() const;
        void setReceivesReturnType(bool value);
        bool getReceivesReturnType() const;
        void setIsVariadic(bool value);
        bool getIsVariadic() const;
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::vector<std::shared_ptr<LILNode>> _arguments;
        std::shared_ptr<LILType> _returnType;
        bool _receivesReturnType;
        bool _isVariadic;
    };
}

#endif
