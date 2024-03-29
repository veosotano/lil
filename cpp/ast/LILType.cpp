/********************************************************************
 *
 *	  LIL Is a Language
 *
 *	  AUTHORS: Miro Keller
 *
 *	  COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *	  LICENSE: see LICENSE file
 *
 *	  This file encapsulates the type of a variable
 *
 ********************************************************************/

#include "LILType.h"
#include "LILMultipleType.h"
#include "LILPointerType.h"

using namespace LIL;

std::shared_ptr<LILType> LILType::merge(std::shared_ptr<LILType> typeA, std::shared_ptr<LILType> typeB)
{
	if (!typeA && !typeB) {
		return nullptr;
	}
	if (typeA && !typeB) {
		return typeA;
	}
	if (typeB && !typeA) {
		return typeB;
	}
	if (typeA->equalTo(typeB)) {
		return typeA;
	}
	
	if (LILType::isIntegerType(typeA.get()) && LILType::isIntegerType(typeB.get())) {
		return (typeA->getBitWidth() > typeB->getBitWidth() ? typeA : typeB);
	}

	auto multiA = std::dynamic_pointer_cast<LILMultipleType>(typeA);
	auto multiB = std::dynamic_pointer_cast<LILMultipleType>(typeB);
	
	if(multiB && !multiA){
		if (multiB->getIsWeakType()) {
			bool found = false;
			for (const auto & tyB : multiB->getTypes()) {
				if (typeA->equalTo(tyB)) {
					found = true;
					break;
				}
			}
			if (found){
				return typeA;
			}
		}
		multiA = std::make_shared<LILMultipleType>();
		multiA->addType(typeA);
	}
	if(multiA && !multiB){
		if (multiA->getIsWeakType()) {
			bool found = false;
			for (const auto & tyA : multiA->getTypes()) {
				if (typeB->equalTo(tyA)) {
					found = true;
					break;
				}
			}
			if (found){
				return typeB;
			} else {
				if (
					LILType::isIntegerType(typeB.get())
					&& (typeB->getBitWidth() > multiA->getIntegerType()->getBitWidth())
				) {
					return typeB;
				}
				return nullptr;
			}
		}
	}
	
	if (multiA && multiB) {
		bool aIsWeak = multiA->getIsWeakType();
		bool bIsWeak = multiB->getIsWeakType();
		if (aIsWeak == bIsWeak) {
			for (const auto & tyB : multiB->getTypes()) {
				bool found = false;
				for (const auto & tyA : multiA->getTypes()) {
					if (tyA->equalTo(tyB)) {
						found = true;
						break;
					}
				}
				if (!found) {
					multiA->addType(tyB);
				}
			}
			return multiA;
		} else if (aIsWeak){
			//a is weak, but b isn't
			std::shared_ptr<LILType> ret = nullptr;
			bool foundOne = false;
			for (const auto & tyB : multiB->getTypes()) {
				for (const auto & tyA : multiA->getTypes()) {
					if (tyA->equalTo(tyB)) {
						if (foundOne) {
							//more than one match -- cant merge
							return nullptr;
						}
						ret = tyB;
						foundOne = true;
					}
				}
				
			}
			return ret;
			
		} else {
			//b is weak, but a isn't
			for (const auto & tyA : multiA->getTypes()) {
				for (const auto & tyB : multiB->getTypes()) {
					if (tyB->equalTo(tyA)) {
						return tyA;
					}
				}
			}
			//if no type from b is found in a, we can't merge
			return nullptr;
		}
	} else if (multiA) {
		bool found = false;
		for (const auto & tyA : multiA->getTypes()) {
			if (tyA->equalTo(typeB)) {
				found = true;
				break;
			}
		}
		if (!found) {
			multiA->addType(typeB);
		}
		return multiA;
	}
	
	if (typeA->isA(TypeTypePointer) && LILType::isNumberType(typeB.get())) {
		return typeA;
	}
	if (typeB->isA(TypeTypePointer) && LILType::isNumberType(typeA.get())) {
		return typeB;
	}
	
	multiA = std::make_shared<LILMultipleType>();
	multiA->addType(typeA);
	multiA->addType(typeB);
	return multiA;
}

std::shared_ptr<LILType> LILType::make(LILString name)
{
	auto ret = std::make_shared<LILType>();
	ret->setTypeType(TypeTypeSingle);
	ret->setName(name);
	return ret;
}

bool LILType::isBuiltInType(LILType * ty)
{
	const auto & name = ty->getName();
	if (
		name == "i8"
		|| name == "i16"
		|| name == "i32"
		|| name == "i64"
		|| name == "i128"
		|| name == "f32"
		|| name == "f64"
		|| name == "bool"
		|| name == "null"
		|| name == "i8%"
		|| name == "i16%"
		|| name == "i32%"
		|| name == "i64%"
		|| name == "f32%"
		|| name == "f64%"
		) {
		return true;
	}
	return false;
}

bool LILType::isNumberType(LILType * ty)
{
	const auto & name = ty->getName();
	if (
		name == "i8"
		|| name == "i16"
		|| name == "i32"
		|| name == "i64"
		|| name == "i128"
		|| name == "f32"
		|| name == "f64"
	) {
		return true;
	}
	return false;
}

bool LILType::isIntegerType(LILType * ty)
{
	const auto & name = ty->getName();
	if (
		name == "i8"
		|| name == "i16"
		|| name == "i32"
		|| name == "i64"
		|| name == "i128"
	) {
		return true;
	}
	return false;
}

bool LILType::isFloatType(LILType * ty)
{
	const auto & name = ty->getName();
	if (
		name == "f32"
		|| name == "f64"
	) {
		return true;
	}
	return false;
}

bool LILType::combinesWithPointer(LILType * ty)
{
	const auto & name = ty->getName();
	if (
		name == "i8"
		|| name == "i16"
		|| name == "i32"
		|| name == "i64"
		|| name == "i128"
		) {
		return true;
	}
	return false;
}

bool LILType::typesCompatible(LILType * ty1, LILType * ty2)
{
	switch (ty1->getTypeType())
	{
		case TypeTypePointer:
		{
			auto ty1p = static_cast<LILPointerType *>(ty1);
			if (!ty2->isA(TypeTypePointer)) {
				if (ty1p->getIsNullable() && ty2->getName() == "null") {
					return true;
				}
				return ty2->equalTo(ty1p->getArgument());
			}
			auto arg = ty1p->getArgument();
			if (arg) {
				if (arg->getName() == "any") {
					return true;
				} else {
					auto ty2p = static_cast<LILPointerType *>(ty2);
					auto arg2 = ty2p->getArgument();
					if (arg2->getName() == "any") {
						return true;
					}

					return ty1->equalTo(ty2->shared_from_this());
				}
			}
			break;
		}
			
		case TypeTypeSingle:
		{
			if (LILType::isNumberType(ty1) && LILType::isNumberType(ty2)) {
				if (ty1->equalTo(ty2->shared_from_this())) {
					return true;
				} else {
					auto dstName = ty1->getName();
					auto srcName = ty2->getName();
					if (dstName == "i16") {
						return srcName == "i8";
					} else if (dstName == "i32") {
						return srcName == "i8" || srcName == "i16";
					} else if (dstName == "i64") {
						return srcName == "i8" || srcName == "i16" || srcName == "i32";
					} else if (dstName == "i128") {
						return srcName == "i8" || srcName == "i16" || srcName == "i32" || srcName == "i64";
					}
					
				}
			}
			return ty1->equalTo(ty2->shared_from_this());
		}
			
		default:
		{
			if (ty2->isA(TypeTypePointer)) {
				auto ptrTy = static_cast<LILPointerType *>(ty2);
				return ty1->equalTo(ptrTy->getArgument());
			}
			return ty1->equalTo(ty2->shared_from_this());
		}
	}
	return false;
}

LILType::LILType()
: LIL::LILNode(NodeTypeType)
, _typeType(TypeTypeSingle)
, _isNullable(false)
{
	
}

LILType::LILType(TypeType type)
: LIL::LILNode(NodeTypeType)
, _typeType(type)
, _isNullable(false)
{
	
}

LILType::LILType(const LILType &other)
: LILNode(other)
, _name(other._name)
, _strongTypeName(other._strongTypeName)
, _typeType(other._typeType)
, _isNullable(other._isNullable)
, _tmplParams(other._tmplParams)
{

}

std::shared_ptr<LILType> LILType::clone() const
{
	return std::static_pointer_cast<LILType> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILType::cloneImpl() const
{
	std::shared_ptr<LILType> clone(new LILType(*this));
	clone->_tmplParams.clear();
	for (auto tmplParam : this->_tmplParams) {
		clone->addTmplParam(tmplParam->clone());
	}
	return clone;
}

LILType::~LILType()
{
	
}

bool LILType::equalTo(std::shared_ptr<LILNode> otherNode)
{
	if ( ! LILNode::equalTo(otherNode)) return false;
	std::shared_ptr<LILType> castedNode = std::static_pointer_cast<LILType>(otherNode);
	if ( this->_name != castedNode->_name ) return false;
	if ( this->_strongTypeName != castedNode->_strongTypeName ) return false;
	if ( this->_typeType != castedNode->_typeType ) return false;
	if ( this->_isNullable != castedNode->_isNullable )  return false;
	if ( this->_tmplParams.size() != castedNode->_tmplParams.size() ) return false;
	return true;
}

void LILType::receiveNodeData(const LIL::LILString &data)
{
	auto currentName = this->getName();
	if (currentName.length() == 0) {
		this->setName(data);
	} else {
		LILString newStr = currentName + data;
		this->setName(newStr);
	}
}

const LILString LILType::getName() const
{
	return this->_name;
}

void LILType::setName(LILString newName)
{
	this->_name = newName;
}

const LILString LILType::getStrongTypeName() const
{
	return this->_strongTypeName;
}

void LILType::setStrongTypeName(LILString newName)
{
	this->_strongTypeName = newName;
}

bool LILType::getIsWeakType() const
{
	return false;
}

std::shared_ptr<LILType> LILType::getDefaultType() const
{
	if (this->getIsWeakType()) {
		auto intTy = this->getIntegerType();
		if (intTy) {
			return intTy;
		}
		auto floatTy = this->getFloatType();
		if (floatTy) {
			return floatTy;
		}
	}
	static auto intType = std::make_shared<LILType>();
	intType->setName("i64");
	return intType;
}

std::shared_ptr<LILType> LILType::getIntegerType() const
{
	return nullptr;
}

std::shared_ptr<LILType> LILType::getFloatType() const
{
	return nullptr;
}

TypeType LILType::getTypeType() const
{
	return this->_typeType;
}

void LILType::setTypeType(TypeType newType)
{
	this->_typeType = newType;
}

bool LILType::isA(TypeType otherType) const
{
	return this->_typeType == otherType;
}

bool LILType::getIsNullable() const
{
	return this->_isNullable;
}

void LILType::setIsNullable(bool newValue)
{
	this->_isNullable = newValue;
}

const std::vector<std::shared_ptr<LILNode>> & LILType::getTmplParams() const
{
	return this->_tmplParams;
}

void LILType::addTmplParam(std::shared_ptr<LILNode> value)
{
	value->setParentNode(this->shared_from_this());
	this->_tmplParams.push_back(value);
}

void LILType::setTmplParams(const std::vector<std::shared_ptr<LILNode>> && values)
{
	this->_tmplParams = std::move(values);
}

size_t LILType::getBitWidth() const
{
	size_t ret = 0;
	auto name = this->getName();
	if (name == "bool") {
		ret = 1;
	} else if (name == "i8") {
		ret = 8;
	} else if (name == "i16") {
		ret = 16;
	} else if (name == "i32") {
		ret = 32;
	} else if (name == "i64") {
		ret = 64;
	} else if (name == "i128") {
		ret = 128;
	} else if (name == "f32") {
		ret = 32;
	} else if (name == "f64") {
		ret = 64;
	}
	return ret;
}
