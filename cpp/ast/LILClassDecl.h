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
 *      This file implements object definitions
 *
 ********************************************************************/


#ifndef LILCLASSDECL_H
#define LILCLASSDECL_H


#include "LILTypedNode.h"

namespace LIL
{
    class LILClassDecl : public LILTypedNode
    {
    public:
        LILClassDecl();
        LILClassDecl(const LILClassDecl &other);
        std::shared_ptr<LILClassDecl> clone() const;
        virtual ~LILClassDecl();
        LILString stringRep();
        
        std::shared_ptr<LILNode> getInheritType() const;
        void setInheritType(std::shared_ptr<LILNode> newType);
        
        bool getReceivesInherits() const;
        void setReceivesInherits(bool value);
        
        void addField(std::shared_ptr<LILNode> value);
        const std::vector<std::shared_ptr<LILNode>> & getFields() const;
        void addMethod(std::shared_ptr<LILNode> value);
        const std::vector<std::shared_ptr<LILNode>> & getMethods() const;
        LILString getName() const;
        
        std::shared_ptr<LILNode> getFieldNamed(const LILString & name) const;
        std::shared_ptr<LILNode> getMethodNamed(const LILString & name) const;

        bool getIsExtern() const;
        void setIsExtern(bool value);
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        bool _isExtern;
        std::shared_ptr<LILNode> _inheritType;
        bool _receivesInherits;
        std::vector<std::shared_ptr<LILNode>> _fields;
        std::vector<std::shared_ptr<LILNode>> _methods;
    };
}

#endif
