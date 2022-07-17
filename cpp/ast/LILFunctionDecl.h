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
    class LILDocumentation;

    class LILFunctionDecl : public LILVarNode
    {
    public:
        LILFunctionDecl();
        LILFunctionDecl(const LILFunctionDecl &other);
        std::shared_ptr<LILType> getType() const;
        void setType(std::shared_ptr<LILType> value);
        std::shared_ptr<LILFunctionType> getFnType() const;
        std::shared_ptr<LILFunctionDecl> clone() const;
        virtual ~LILFunctionDecl();
        virtual void receiveNodeData(const LILString &data);

        void addEvaluable(std::shared_ptr<LILNode> evl);
        void prependEvaluable(std::shared_ptr<LILNode> evl);
        const std::vector<std::shared_ptr<LILNode>> & getBody() const;
        void setBody(const std::vector<std::shared_ptr<LILNode>> & newBody);
        void clearBody();

        bool getReceivesFunctionBody() const;
        void setReceivesFunctionBody(bool newValue);

        std::shared_ptr<LILType> getReturnType() const;
        void setReturnType(std::shared_ptr<LILType> type);

        LILString getName() const;
        void setName(LILString value);
        LILString getUnmangledName() const;
        void setUnmangledName(LILString value);
        
        bool hasReturn() const;
        bool getIsConstructor() const;
        void setIsConstructor(bool value);
        
        std::shared_ptr<LILNode> getFinally() const;
        void setFinally(std::shared_ptr<LILNode> value);

        bool getIsExtern() const;
        void setIsExtern(bool value);

        bool getHasMultipleImpls() const;
        void setHasMultipleImpls(bool value);
        
        const std::vector<std::shared_ptr<LILFunctionDecl>> & getImpls() const;
        void addImpl(std::shared_ptr<LILFunctionDecl> fd);
        void setImpls(const std::vector<std::shared_ptr<LILFunctionDecl>> & impls);
        void clearImpls();
        
        void addDoc(std::shared_ptr<LILDocumentation> value);
        const std::vector<std::shared_ptr<LILDocumentation>> & getDocs() const;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::vector<std::shared_ptr<LILNode>> _body;
        std::vector<std::shared_ptr<LILFunctionDecl>> _impls;
        std::vector<std::shared_ptr<LILDocumentation>> _docs;
        std::shared_ptr<LILNode> _fnlly;
        std::shared_ptr<LILFunctionType> _fnType;
        bool _receivesFunctionBody;
        LILString _name;
        LILString _unmangledName;
        bool _hasReturn;
        bool _isConstructor;
        bool _isExtern;
        bool _hasMultipleImpls;
    };
}

#endif
