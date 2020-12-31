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
 *      This file encapsulates the type of a variable
 *
 ********************************************************************/

#ifndef LILTYPE_H
#define LILTYPE_H

#include "LILNode.h"

namespace LIL
{
    class LILType : public LILNode
    {
    public:
        static std::shared_ptr<LILType> merge(std::shared_ptr<LILType> typeA, std::shared_ptr<LILType> typeB);
        static std::shared_ptr<LILType> make(LILString name);
        static bool sortTyAlphabeticallyCompare(std::shared_ptr<LILType> typeA, std::shared_ptr<LILType> typeB) {
            return typeA->getName().data() < typeB->getName().data();
        };
        static bool isBuiltInType(LILString name);
        static bool isNumberType(LILString name);

        LILType();
        LILType(TypeType type);
        LILType(const LILType &other);
        std::shared_ptr<LILType> clone() const;
        virtual ~LILType();
        bool equalTo(std::shared_ptr<LILNode> otherNode) override;
        virtual void receiveNodeData(const LILString & data) override;

        const LILString getName() const;
        void setName(LILString newName);

        const LILString getStrongTypeName() const;
        void setStrongTypeName(LILString newName);

        virtual bool getIsWeakType() const;

        TypeType getTypeType() const override;
        void setTypeType(TypeType newType);
        virtual bool isA(TypeType otherType) const override;

        bool getIsNullable() const;
        void setIsNullable(bool newValue);

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const override;

    private:
        LILString _name;
        LILString _strongTypeName;
        TypeType _typeType;
        bool _isNullable;
    };
}

#endif
