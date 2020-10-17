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
 *      This file represents function declarations
 *
 ********************************************************************/


#ifndef LILFUNCTIONDECL_H
#define LILFUNCTIONDECL_H


#include "LILVarNode.h"
#include "LILFunctionType.h"

namespace LIL
{
    class LILFunctionDecl : public LILVarNode
    {
    public:
        bool needsNameMangling;

        LILFunctionDecl();
        LILFunctionDecl(const LILFunctionDecl &other);
        bool isTypedNode() const;
        void setType(std::shared_ptr<LILType> value);
        std::shared_ptr<LILType> getType() const;
        std::shared_ptr<LILFunctionType> getFnType() const;
        std::shared_ptr<LILFunctionDecl> clone() const;
        virtual ~LILFunctionDecl();
        virtual LILString stringRep();
        virtual void receiveNodeData(const LILString &data);

        void addEvaluable(std::shared_ptr<LILNode> evl);
        void prependEvaluable(std::shared_ptr<LILNode> evl);
        const std::vector<std::shared_ptr<LILNode>> & getBody() const;
        void setBody(std::vector<std::shared_ptr<LILNode>> newBody);
        
        void setReceivesFunctionBody(bool newValue);
        bool getReceivesFunctionBody() const;
        
        FunctionDeclType getFunctionDeclType() const;
        void setFunctionDeclType(FunctionDeclType newType);
        
        void setReturnType(std::shared_ptr<LILType> type);
        std::shared_ptr<LILType> getReturnType() const;
        
        LILString getName() const;
        void setName(LILString value);
        
        bool hasReturn() const;
        bool getIsConstructor() const;
        void setIsConstructor(bool value);
        
        std::shared_ptr<LILNode> getFinally() const;
        void setFinally(std::shared_ptr<LILNode> value);

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::vector<std::shared_ptr<LILNode>> _body;
        std::shared_ptr<LILNode> _finally;
        bool _receivesFunctionBody;
        FunctionDeclType _functionDeclType;
        LILString _name;
        bool _hasReturn;
        bool _isConstructor;
    };
}

#endif
