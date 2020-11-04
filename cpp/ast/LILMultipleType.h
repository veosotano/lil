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
 *      This file encapsulates multiple types for one variable
 *
 ********************************************************************/

#ifndef LILMULTIPLETYPE_H
#define LILMULTIPLETYPE_H

#include "LILType.h"

namespace LIL
{
    class LILMultipleType : public LILType
    {
    public:
        LILMultipleType();
        LILMultipleType(const LILMultipleType &other);
        std::shared_ptr<LILMultipleType> clone() const;
        virtual ~LILMultipleType();
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        virtual void receiveNodeData(const LILString & data);
        
        LILString stringRep();
        
        void addType(std::shared_ptr<LILType> ty);
        void setTypes(std::vector<std::shared_ptr<LILType>> tys);
        std::vector<std::shared_ptr<LILType>> getTypes() const;
        
        bool getIsWeakType() const;
        void setIsWeakType(bool value);

        void sortTypes();
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        std::vector<std::shared_ptr<LILType>> _types;
        bool _isWeakType;
    };
}

#endif
