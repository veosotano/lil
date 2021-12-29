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
        bool equalTo(std::shared_ptr<LILNode> otherNode) override;
        void receiveNodeData(const LILString & data) override;

        void addType(std::shared_ptr<LILType> ty);
        void setTypes(std::vector<std::shared_ptr<LILType>> && tys);
        std::vector<std::shared_ptr<LILType>> getTypes() const;

        bool getIsWeakType() const override;
        void setIsWeakType(bool value);
        std::shared_ptr<LILType> getIntegerType() const override;

        void sortTypes();

        size_t indexOfType(LILType * ty) const;

    protected:
        std::shared_ptr<LILClonable> cloneImpl() const override;

    private:
        std::vector<std::shared_ptr<LILType>> _types;
        bool _isWeakType;
    };
}

#endif
