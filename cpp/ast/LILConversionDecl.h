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
 *      This file is used to declare implicit conversion between types
 *
 ********************************************************************/

#ifndef LILCONVERSIONDECL_H
#define LILCONVERSIONDECL_H

#include "LILTypedNode.h"

namespace LIL
{
    class LILVarDecl;
    class LILConversionDecl : public LILTypedNode
    {
    public:
        LILConversionDecl();
        LILConversionDecl(const LILConversionDecl &other);
        std::shared_ptr<LILConversionDecl> clone() const;
        virtual ~LILConversionDecl();
        void setType(std::shared_ptr<LILType> value) override;
        
        std::shared_ptr<LILVarDecl> getVarDecl() const;
        void setVarDecl(std::shared_ptr<LILVarDecl> value);
        
        void receiveNodeData(const LIL::LILString &data) override;
        
        void addEvaluable(std::shared_ptr<LILNode> node);
        const std::vector<std::shared_ptr<LILNode>> & getBody() const;
        void setBody(std::vector<std::shared_ptr<LILNode>> newBody);
        void clearBody();
        LILString encodedName();

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;
        
    private:
        std::shared_ptr<LILVarDecl> _varDecl;
        std::vector<std::shared_ptr<LILNode>> _body;
        LILString _srcTyName;
        LILString _destTyName;
    };
}

#endif
