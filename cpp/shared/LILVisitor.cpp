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
 *	  This file defines the base class for visitors
 *
 ********************************************************************/

#include "LILVisitor.h"
#include "LILClassDecl.h"
#include "LILEnum.h"
#include "LILErrorMessage.h"
#include "LILFlowControl.h"
#include "LILFunctionCall.h"
#include "LILVarName.h"
#include "LILVarNode.h"
#include "LILRootNode.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILIndexAccessor.h"
#include "LILMultipleType.h"
#include "LILNumberLiteral.h"
#include "LILObjectType.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILRule.h"
#include "LILSIMDType.h"
#include "LILStaticArrayType.h"
#include "LILStringLiteral.h"
#include "LILValuePath.h"
#include "LILVaLueList.h"
#include "LILVarDecl.h"

using namespace LIL;

LILString LILVisitor__getTypeName(LILType * ty)
{
	switch (ty->getTypeType()) {
		case LIL::TypeTypePointer:
		{
			LILPointerType * ptrTy = static_cast<LILPointerType *>(ty);
			LILString ret = ty->getName();
			ret += "_"+LILVisitor__getTypeName(ptrTy->getArgument().get());
			return ret;
		}
			
		default:
			return ty->getName();
	}
}

LILVisitor::LILVisitor()
: inhibitSearchingForIfCastType(false)
, _printHeadline(true)
, _verbose(false)
, _debug(false)
{
}

LILVisitor::~LILVisitor()
{
}

void LILVisitor::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
	this->setRootNode(rootNode);
	std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
	for (const auto & node : nodes) {
		node->accept(this);
	}
}

void LILVisitor::visit(LILNode * node)
{
	//do nothing
}

bool LILVisitor::hasErrors() const
{
	size_t errorSize = this->errors.size();
	return errorSize > 0;
}

void LILVisitor::setPrintHeadline(bool value)
{
	this->_printHeadline = value;
}

bool LILVisitor::getPrintHeadline() const
{
	return this->_printHeadline;
}

void LILVisitor::setVerbose(bool value)
{
	this->_verbose = value;
}

bool LILVisitor::getVerbose() const
{
	return this->_verbose;
}

void LILVisitor::setDebug(bool value)
{
	this->_debug = value;
}

bool LILVisitor::getDebug() const
{
	return this->_debug;
}

std::shared_ptr<LILNode> LILVisitor::findNodeForVarName(LILVarName * name) const
{
	return this->findNodeForName(name->getName(), name->getParentNode().get());
}

std::shared_ptr<LILNode> LILVisitor::findNodeForName(LILString name, LILNode * parent) const
{
	while (parent) {
		if(parent->isVarNode()){
			LILVarNode * vn = static_cast<LILVarNode *>(parent);
			std::shared_ptr<LILNode> localVar = vn->getLocalVariable(name);
			if (localVar) {
				return localVar;
			}
		}
		bool isRoot = parent->isA(NodeTypeRoot);
		parent = parent->getParentNode().get();
		if (!parent && !isRoot) {
			std::cerr << "NODE WHICH IS NOT ROOT DID NOT HAVE A PARENT FAIL !!!!!!!!\n\n";
		}
	}
	return nullptr;
}

std::shared_ptr<LILNode> LILVisitor::findNodeForPropertyName(LILPropertyName * name) const
{
	std::shared_ptr<LILNode> ret;
	auto parent = name->getParentNode();
	if (parent->getNodeType() == NodeTypeAssignment) {
		auto grandpa = parent->getParentNode();
		auto gpTy = grandpa->getNodeType();
		if ( gpTy == NodeTypeObjectDefinition || gpTy == NodeTypeRule ) {
			auto grandpaTy = grandpa->getType();
			if (!grandpaTy && gpTy == NodeTypeRule) {
				grandpaTy = LILObjectType::make("container");
			}
			auto classDecl = this->findClassWithName(grandpaTy->getName());
			
			auto pnName = name->getName();
			ret = classDecl->getFieldNamed(pnName);
			if (!ret) {
				ret = this->findExpandedField(classDecl, pnName);
			}
		} else if (gpTy == NodeTypeEnum) {
			auto enm = std::static_pointer_cast<LILEnum>(grandpa);
			ret = enm->getValueNamed(name->getName());
		}
	}
	return ret;
}

std::shared_ptr<LILNode> LILVisitor::findNodeForValuePath(LILValuePath * vp) const
{
	auto nodes = vp->getNodes();
	std::shared_ptr<LILNode> currentNode;
	std::shared_ptr<LILType> currentTy;
	if (nodes.size() == 1) {
		currentNode = nodes.front();
		if (currentNode->isA(NodeTypeVarName)) {
			return this->recursiveFindNode(currentNode);
		}
	} else if (nodes.size() > 1){
		currentNode = nodes.front();

		if (currentNode->isA(NodeTypeVarName)) {
			auto localNode = this->recursiveFindNode(currentNode);
			if (localNode) {
				currentNode = localNode;
				currentTy = localNode->getType();
				if (!currentTy) {
					std::cerr << "CANNOT TRAVERSE VALUE PATH IF SUBJECT TYPE IS NULL FAIL !!!!!!!\n\n";
					return nullptr;
				}
				if (!this->inhibitSearchingForIfCastType && currentTy->isA(TypeTypeMultiple)) {
					size_t outStartIndex = 0;
					auto ifCastTy = this->findIfCastType(vp, outStartIndex);
					if (ifCastTy) {
						currentTy = ifCastTy;
					}
				}
			}
		}
		else if (currentNode->isA(SelectorTypeSelfSelector)) {
			auto classDecl = this->findAncestorClass(currentNode);
			currentTy = classDecl->getType();
		}
		else if (currentNode->isA(SelectorTypeThisSelector)) {
			auto rule = this->findAncestorRule(currentNode);
			currentTy = rule->getType();
		}
		for (size_t i=1, j=nodes.size(); i<j; ++i) {
			bool isLast = i==j-1;
			auto node = nodes[i];
			switch (node->getNodeType()) {
				case NodeTypeFunctionCall:
				{
					if (!currentTy->isA(TypeTypeObject)) {
						std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					std::shared_ptr<LILClassDecl> classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}

					auto fc = std::static_pointer_cast<LILFunctionCall>(node);
					auto method = classDecl->getMethodNamed(fc->getName());
					if (!method->isA(NodeTypeFunctionDecl)) {
						std::cerr << "!!!!!!!!!!NODE IS NOT FUNCTION DECL FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto fd = std::static_pointer_cast<LILFunctionDecl>(method);
					auto fnTy = fd->getFnType();
					auto retTy = fnTy->getReturnType();
					if (isLast) {
						return method;
					} else {
						currentNode = method;
						currentTy = retTy;
					}
					break;
				}
				case NodeTypePropertyName:
				{
					if (currentTy->getTypeType() == TypeTypePointer) {
						auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
						currentTy = ptrTy->getArgument();
					}
					if (!currentTy->isA(TypeTypeObject)) {
						std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					std::shared_ptr<LILClassDecl> classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}

					auto pn = std::static_pointer_cast<LILPropertyName>(node);
					auto pnName = pn->getName();
					auto field = classDecl->getFieldNamed(pnName);
					if (isLast) {
						return field;
					} else {
						currentNode = field;
						currentTy = field->getType();
					}
					break;
				}
				case NodeTypeIndexAccessor:
				{
					auto ia = std::static_pointer_cast<LILIndexAccessor>(node);
					if (currentTy->getTypeType() == TypeTypeObject) {
						std::shared_ptr<LILClassDecl> classDecl = this->findClassWithName(currentTy->getName());
						if (!classDecl) {
							std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
							return nullptr;
						}
						auto method = classDecl->getMethodNamed("at");
						if (!method->isA(NodeTypeFunctionDecl)) {
							std::cerr << "AT METHOD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
							return nullptr;
						}
						auto fd = std::static_pointer_cast<LILFunctionDecl>(method);
						auto fnTy = fd->getFnType();
						auto retTy = fnTy->getReturnType();
						if (isLast) {
							return method;
						} else {
							currentNode = method;
							currentTy = retTy;
						}
						
					} else if (currentTy->getTypeType() == TypeTypeStaticArray) {
						if (isLast) {
							//we can't return a node
							return nullptr;
						} else {
							auto saTy = std::static_pointer_cast<LILStaticArrayType>(currentTy);
							currentTy = saTy->getType();
						}
					}
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!VALUE PATH NODE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
		}
	}
	return nullptr;
}

std::shared_ptr<LILNode> LILVisitor::recursiveFindNode(std::shared_ptr<LILNode> node) const
{
	if (!node) {
		return nullptr;
	}
	switch (node->getNodeType()) {
		case NodeTypeVarName:
		{
			return this->recursiveFindNode(this->findNodeForVarName(static_cast<LILVarName *>(node.get())));
		}
		case NodeTypeValuePath:
		{
			return this->recursiveFindNode(this->findNodeForValuePath(static_cast<LILValuePath *>(node.get())));
		}
		case NodeTypePropertyName:
		{
			return this->recursiveFindNode(this->findNodeForPropertyName(static_cast<LILPropertyName *>(node.get())));
		}
		default:
			return node;
	}
}

LILString LILVisitor::decorate(LILString ns, LILString className, LILString name, std::shared_ptr<LILType> type) const
{
	LILString ret("_lil_");
	if (ns.length() > 0) {
		ret += LILString("n") + ns + "_";
	}
	if (className.length() > 0) {
		ret += LILString("c") + LILString::number((LILUnitI64)className.length()) + "_" + className + "_";
	}
	ret += LILString("f") + LILString::number((LILUnitI64)name.length()) + "_" + name;
	if(type){
		ret += this->typeToString(type);
	}

	return ret;
}

LILString LILVisitor::typeToString(std::shared_ptr<LILType> type) const
{
	LILString ret("");
	switch (type->getTypeType()) {
		case TypeTypeFunction:
		{
			const auto & args = std::static_pointer_cast<LILFunctionType>(type)->getArguments();
			if (args.size() == 0) {
				return ret;
			} else {
				ret = "_";
			}
			for (size_t i=0, j=args.size(); i<j; ++i) {
				const auto & arg = args[i];
				LILString tyName;
				if (arg->isA(NodeTypeType)) {
					tyName = this->typeToString(std::static_pointer_cast<LILType>(arg));
				} else if (arg->isA(NodeTypeVarDecl)){
					auto vd = std::static_pointer_cast<LILVarDecl>(arg);
					auto vdTy = vd->getType();
					if (vdTy) {
						tyName = this->typeToString(vdTy);
					}
				}
				
				if (arg->isA(TypeTypeFunction)) {
					ret += LILString("f0") + tyName;
				} else {
					ret += "a" + LILString::number((LILUnitI64)tyName.length())+"_"+tyName;
				}
				if (i<j-1) {
					ret += "_";
				}
			}
			break;
		}
		case TypeTypePointer:
		{
			auto ptrTy = std::static_pointer_cast<LILPointerType>(type);
			ret += "ptr_"+LILVisitor::typeToString(ptrTy->getArgument());
			break;
		}
		case TypeTypeStaticArray:
		{
			auto saTy = std::static_pointer_cast<LILStaticArrayType>(type);
			ret += "sarr_"+LILVisitor::typeToString(std::static_pointer_cast<LILType>(saTy->getArgument()));
			break;
		}
		case TypeTypeMultiple:
		{
			auto mt = std::static_pointer_cast<LILMultipleType>(type);
			ret += "mt";
			for (auto mtTy : mt->getTypes()) {
				ret += "_" + LILVisitor::typeToString(mtTy);
			}
			break;
		}
		case TypeTypeObject:
		case TypeTypeSingle:
		{
			ret += type->getName();
			for (const auto & node : type->getTmplParams()) {
				switch (node->getNodeType()) {
					case NodeTypeType:
					{
						ret += "_" + LILVisitor::typeToString(std::static_pointer_cast<LILType>(node));
						break;
					}
					case NodeTypeStringLiteral:
					{
						auto str = std::static_pointer_cast<LILStringLiteral>(node);
						ret += "_" + str->getValue();
						break;
					}
					case NodeTypeNumberLiteral:
					{
						auto num = std::static_pointer_cast<LILNumberLiteral>(node);
						ret += "_" + num->getValue();
						break;
					}
					default:
						std::cerr << "UNKNOWN NODE TYPE FAIL!!!!!!! !!!! !!!!!\n";
						return "error";
				}
			}
			break;
		}
		case TypeTypeSIMD:
		{
			auto simdTy = std::static_pointer_cast<LILSIMDType>(type);
			ret += simdTy->getName() + "x" + LILString::number((LILUnitI32) simdTy->getWidth());
			break;
		}
		case TypeTypeNone:
			//do nothing;
			break;
	}
	return ret;
}

void LILVisitor::setRootNode(std::shared_ptr<LILRootNode> value)
{
	this->_rootNode = value;
}

std::shared_ptr<LILRootNode> LILVisitor::getRootNode() const
{
	return this->_rootNode;
}

std::shared_ptr<LILClassDecl> LILVisitor::findClassWithName(const LILString & name) const
{
	return this->getRootNode()->findClassWithName(name);
}

std::shared_ptr<LILEnum> LILVisitor::findEnumWithName(const LILString & name) const
{
	return this->getRootNode()->findEnumWithName(name);
}

std::shared_ptr<LILClassDecl> LILVisitor::findAncestorClass(std::shared_ptr<LILNode> node) const
{
	auto parent = node->getParentNode();
	if (parent) {
		if (parent->isA(NodeTypeClassDecl)) {
			return std::static_pointer_cast<LILClassDecl>(parent);
		} else {
			return this->findAncestorClass(parent);
		}
	} else {
		return nullptr;
	}
}

std::shared_ptr<LILRule> LILVisitor::findAncestorRule(std::shared_ptr<LILNode> node) const
{
	auto parent = node->getParentNode();
	if (parent) {
		if (parent->isA(NodeTypeRule)) {
			return std::static_pointer_cast<LILRule>(parent);
		} else {
			return this->findAncestorRule(parent);
		}
	} else {
		return nullptr;
	}
}

std::shared_ptr<LILFlowControl> LILVisitor::findAncestorFor(std::shared_ptr<LILNode> node) const
{
	auto parent = node->getParentNode();
	if (parent) {
		if (parent->isA(NodeTypeFlowControl) && parent->getFlowControlType() == FlowControlTypeFor) {
			return std::static_pointer_cast<LILFlowControl>(parent);
		} else {
			return this->findAncestorFor(parent);
		}
	} else {
		return nullptr;
	}
}

std::shared_ptr<LILType> LILVisitor::findIfCastType(LILValuePath * vp, size_t & outStartIndex) const
{
	std::shared_ptr<LILType> ret;
	auto parent = vp->getParentNode();
	while (parent) {
		if (parent->isA(FlowControlTypeIfCast)) {
			auto fc = std::static_pointer_cast<LILFlowControl>(parent);
			auto args = fc->getArguments();
			if (args.size() != 2) {
				break;
			}
			auto firstArg = args.front();
			if (firstArg->isA(NodeTypeValuePath)) {
				auto ifCastVp = std::static_pointer_cast<LILValuePath>(firstArg);
				auto ifCastVpNodes = ifCastVp->getNodes();
				auto vpNodes = vp->getNodes();
				bool valid = true;
				if (ifCastVpNodes.size() > vpNodes.size()) {
					valid = false;
				} else {
					for (size_t i = 0, j = ifCastVpNodes.size(); i<j; ++i) {
						if (!vpNodes.at(i)->equalTo(ifCastVpNodes.at(i))) {
							valid = false;
						}
					}
				}
				if (valid) {
					auto ifCastTy = args.back();
					ret = std::static_pointer_cast<LILType>(ifCastTy);
					outStartIndex = ifCastVpNodes.size();
				}
			}
			else
			{
				auto nodes = vp->getNodes();
				if (firstArg->isA(NodeTypeVarName)) {
					if ( nodes.size() == 1) {
						if (firstArg->equalTo(nodes.front())) {
							auto ifCastTy = args.back();
							return std::static_pointer_cast<LILType>(ifCastTy);
						}
					} else {
						if (firstArg->equalTo(nodes.front())) {
							auto ifCastTy = args.back();
							outStartIndex = 1;
							return std::static_pointer_cast<LILType>(ifCastTy);
						}
					}
				}
			}
		}
		parent = parent->getParentNode();
	}
	return ret;
}

std::shared_ptr<LILType> LILVisitor::findIfCastTypeVN(LILVarName * vn) const
{
	std::shared_ptr<LILType> ret;
	auto parent = vn->getParentNode();
	while (parent) {
		if (parent->isA(FlowControlTypeIfCast)) {
			auto fc = std::static_pointer_cast<LILFlowControl>(parent);
			auto args = fc->getArguments();
			if (args.size() != 2) {
				break;
			}
			auto firstArg = args.front();
			if (firstArg->isA(NodeTypeVarName)) {
				if (firstArg->equalTo(vn->shared_from_this())) {
					auto ifCastTy = args.back();
					return std::static_pointer_cast<LILType>(ifCastTy);
				}
			}
		}
		parent = parent->getParentNode();
	}
	return ret;
}

std::shared_ptr<LILNode> LILVisitor::findExpandedField(std::shared_ptr<LILClassDecl> classDecl, const LILString & pnName) const
{
	std::shared_ptr<LILNode> ret = nullptr;
	bool found = false;
	for (auto field : classDecl->getFields()) {
		if (!field->isA(NodeTypeVarDecl)) {
			continue;
		}
		auto vd = std::static_pointer_cast<LILVarDecl>(field);
		if (vd->getIsExpanded()) {
			auto vdTy = vd->getType();
			if (vdTy && vdTy->isA(TypeTypeObject)) {
				classDecl = this->findClassWithName(vdTy->getName());
				if (!found) {
					ret = classDecl->getFieldNamed(pnName);
					if (!ret) {
						ret = this->findExpandedField(classDecl, pnName);
					}
					if (ret) {
						found = true;
					}
				} else {
					std::cerr << "EXPANDED FIELD CLASH FAIL!!!!!!!!!!!!!!!!\n";
					return nullptr;
				}
			}
		}
	}
	return ret;
}

std::shared_ptr<LILType> LILVisitor::findTypeForValueList(LILValueList * value) const
{
	std::shared_ptr<LILType> ret;
	std::vector<std::shared_ptr<LILType>> types;
	for (const auto & val : value->getValues()) {
		const auto & ty = val->getType();
		bool found = false;
		for (const auto & existingTy : types) {
			if (existingTy->equalTo(ty)) {
				found = true;
				break;
			}
		}
		if (!found) {
			types.push_back(ty);
		}
	}

	if (types.size() == 1) {
		ret = *(types.begin());
	} else if (types.size() > 0) {
		auto mTy = std::make_shared<LILMultipleType>();
		for (const auto & ty : types) {
			if (ty->getIsWeakType()) {
				mTy->addType(ty->getDefaultType());
			} else {
				mTy->addType(ty);
			}
		}
		bool intFound = false;
		bool onlyIntTypes = true;
		bool floatFound = false;
		bool onlyFloatTypes = true;
		for (const auto & mTyTy : mTy->getTypes()) {
			if (LILType::isIntegerType(mTyTy.get())) {
				intFound = true;
			} else {
				onlyIntTypes = false;
			}
			if (LILType::isFloatType(mTyTy.get())) {
				floatFound = true;
			} else {
				onlyFloatTypes = false;
			}
		}
		if (
			(intFound && onlyIntTypes)
			|| (floatFound && onlyFloatTypes)
		){
			std::shared_ptr<LILType> biggestType = nullptr;
			for (const auto & mTyTy : mTy->getTypes()) {
				if (!biggestType || (biggestType->getBitWidth() < mTyTy->getBitWidth())) {
					biggestType = mTyTy;
				}
			}
			if (biggestType) {
				return biggestType;
			}
		}
		ret = mTy;
	}
	return ret;
}
