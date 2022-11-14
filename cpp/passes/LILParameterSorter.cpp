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
 *	  This file sorts the fields of classes for optimal memory usage
 *
 ********************************************************************/

#include "LILNodeToString.h"
#include "LILParameterSorter.h"
#include "LILPointerType.h"
#include "LILStaticArrayType.h"
#include "LILVarNode.h"

using namespace LIL;

LILParameterSorter::LILParameterSorter()
{
}

LILParameterSorter::~LILParameterSorter()
{
}

void LILParameterSorter::initializeVisit()
{
	if (this->getVerbose()) {
		std::cerr << "\n\n";
		std::cerr << "============================\n";
		std::cerr << "=====  PARAMETER SORTING   =====\n";
		std::cerr << "============================\n\n";
	}
}

void LILParameterSorter::visit(LILNode *node)
{
	this->process(node);
}

void LILParameterSorter::process(LILNode * node)
{
	if (this->getDebug()) {
		std::cerr << "## sorting parameters " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node).data() + " ##\n";
	}
	switch (node->getNodeType()) {
		case NodeTypeBoolLiteral:
		{
			LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeNumberLiteral:
		{
			LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
			this->_process(value);
			break;
		}
		case NodeTypePercentage:
		{
			LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeExpression:
		{
			LILExpression * value = static_cast<LILExpression *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeUnaryExpression:
		{
			LILUnaryExpression * value = static_cast<LILUnaryExpression *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeStringLiteral:
		{
			LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeStringFunction:
		{
			LILStringFunction * value = static_cast<LILStringFunction *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeNull:
		{
			LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeVarDecl:
		{
			LILVarDecl * value = static_cast<LILVarDecl *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeAliasDecl:
		case NodeTypeTypeDecl:
		case NodeTypeConversionDecl:
		case NodeTypeEnum:
		{
			//do nothing
			break;
		}
		case NodeTypeClassDecl:
		{
			LILClassDecl * value = static_cast<LILClassDecl *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeObjectDefinition:
		{
			LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeAssignment:
		{
			LILAssignment * value = static_cast<LILAssignment *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeValuePath:
		{
			LILValuePath * value = static_cast<LILValuePath *>(node);
			this->_process(value);
			break;
		}
		case NodeTypePropertyName:
		{
			LILPropertyName * value = static_cast<LILPropertyName *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeVarName:
		{
			LILVarName * value = static_cast<LILVarName *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeRule:
		{
			LILRule * value = static_cast<LILRule *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeSimpleSelector:
		{
			LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeSelectorChain:
		{
			LILSelectorChain * value = static_cast<LILSelectorChain *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeSelector:
		{
			LILSelector * value = static_cast<LILSelector *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeCombinator:
		{
			LILCombinator * value = static_cast<LILCombinator *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeFilter:
		{
			LILFilter * value = static_cast<LILFilter *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeFlag:
		{
			LILFlag * value = static_cast<LILFlag *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeFunctionDecl:
		{
			LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeFunctionCall:
		{
			LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeFlowControl:
		{
			LILFlowControl * value = static_cast<LILFlowControl *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeFlowControlCall:
		{
			LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeInstruction:
		{
			LILInstruction * value = static_cast<LILInstruction *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeIfInstruction:
		{
			LILIfInstruction * value = static_cast<LILIfInstruction *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeType:
		{
			break;
		}
		case NodeTypeValueList:
		{
			LILValueList * value = static_cast<LILValueList *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeDocumentation:
		{
			LILDocumentation * value = static_cast<LILDocumentation *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeIndexAccessor:
		{
			LILIndexAccessor * value = static_cast<LILIndexAccessor *>(node);
			this->_process(value);
			break;
		}
		case NodeTypeSnippetInstruction:
		{
			//ignore
			break;
		}
			
		default:
			std::cerr << "Error: unkonwn node type to process\n";
			break;
	}
}

void LILParameterSorter::_process(LILBoolLiteral * value)
{
	
}

void LILParameterSorter::_process(LILNumberLiteral * value)
{
	
}

void LILParameterSorter::_process(LILPercentageLiteral * value)
{
	
}

void LILParameterSorter::_process(LILExpression * value)
{
	this->process(value->getLeft().get());
	this->process(value->getRight().get());
}

void LILParameterSorter::_process(LILUnaryExpression * value)
{
	this->process(value->getValue().get());
}

void LILParameterSorter::_process(LILStringLiteral * value)
{
	
}

void LILParameterSorter::_process(LILStringFunction * value)
{
	this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILNullLiteral * value)
{
	
}

void LILParameterSorter::_process(LILVarDecl * value)
{
	auto initVal = value->getInitVal();
	if (initVal) {
		this->process(initVal.get());
	}
}

void LILParameterSorter::_process(LILClassDecl * value)
{
	if (!value->getIsExtern()) {
		this->processChildren(value->getFields());
		for (const auto & methodPair : value->getMethods())
		{
			this->process(methodPair.second.get());
		};
	}
}

void LILParameterSorter::_process(LILObjectDefinition * value)
{
	this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILAssignment * value)
{
	this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILValuePath * value)
{
	this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILPropertyName * value)
{
}

void LILParameterSorter::_process(LILRule * value)
{
	this->processChildren(value->getChildNodes());
}

void LILParameterSorter::_process(LILSimpleSelector * value)
{
	
}

void LILParameterSorter::_process(LILSelectorChain * value)
{
	this->processChildren(value->getNodes());
}

void LILParameterSorter::_process(LILSelector * value)
{
	
}

void LILParameterSorter::_process(LILCombinator * value)
{
}

void LILParameterSorter::_process(LILFilter * value)
{
}

void LILParameterSorter::_process(LILFlag * value)
{
}

void LILParameterSorter::_process(LILVarName * value)
{
}

void LILParameterSorter::_process(LILFunctionDecl * value)
{
	this->processChildren(value->getBody());
}

void LILParameterSorter::_process(LILFunctionCall * value)
{
	if (value->getFunctionCallType() != FunctionCallTypeValuePath) {
		const auto & callName = value->getName();
		auto remoteNode = this->findNodeForName(callName, value->getParentNode().get());
		if (remoteNode && remoteNode->getNodeType() == NodeTypeFunctionDecl) {
			auto fd = std::static_pointer_cast<LILFunctionDecl>(remoteNode);
			this->_processArguments(value, fd.get());
		}
		return;
	}

	auto vp = std::static_pointer_cast<LILValuePath>(value->getParentNode());
	
	auto childNodes = vp->getNodes();
	if (childNodes.size() > 1)
	{
		auto it = childNodes.begin();
		auto firstNode = *it;
		bool isExtern = false;
		std::shared_ptr<LILType> currentTy;
		LILString varName;
		
		if (firstNode->isA(NodeTypeVarName)) {
			auto vn = std::static_pointer_cast<LILVarName>(firstNode);
			std::shared_ptr<LILNode> subjectNode = this->recursiveFindNode(vn);
			if (subjectNode) {
				if (subjectNode->isA(NodeTypeVarDecl)) {
					auto vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
					varName = vd->getName();
					if (vd->getIsExtern()) {
						isExtern = true;
					}
					currentTy = vd->getType();
				}
			} else {
				return;
			}
		} else if (firstNode->isA(SelectorTypeSelfSelector)) {
			auto cd = this->findAncestorClass(firstNode);
			currentTy = cd->getType();
		}
		
		if (isExtern) {
			return;
		}
		
		++it;
		
		while (it != childNodes.end()) {
			auto currentNode = *it;
			switch (currentNode->getNodeType()) {
				case NodeTypeFunctionCall:
				{
					auto fc = std::static_pointer_cast<LILFunctionCall>(currentNode);
					if (!this->inhibitSearchingForIfCastType && currentTy->isA(TypeTypeMultiple)) {
						size_t outStartIndex = 0;
						auto ifCastTy = this->findIfCastType(fc->getSubject().get(), outStartIndex);
						if (ifCastTy) {
							currentTy = ifCastTy;
						}
					}
					if (currentTy->isA(TypeTypePointer)) {
						auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
						currentTy = ptrTy->getArgument();
					}
					if (currentTy->isA(TypeTypeStaticArray)) {
						auto saTy = std::static_pointer_cast<LILStaticArrayType>(currentTy);
						currentTy = saTy->getType();
					}
					if (!currentTy->isA(TypeTypeObject)) {
						std::cerr << "VALUE PATH NODE DOES NOT POINT TO OBJECT FAIL!!!!!!!!\n";
						return;
					}
					auto classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "CLASS NOT FOUND FAIL!!!!!!!!\n";
						return;
					}
					auto methodNode = classDecl->getMethodNamed(fc->getName());
					if (!methodNode) {
						std::cerr << "METHOD NOT FOUND FAIL!!!!!!!!\n";
						return;
					}
					if (!methodNode->isA(NodeTypeFunctionDecl)) {
						std::cerr << "METHOD WAS NOT FUNCTION DECL FAIL!!!!!!!!\n";
						return;
					}
					auto method = std::static_pointer_cast<LILFunctionDecl>(methodNode);
					auto fnTy = method->getFnType();
					currentTy = fnTy->getReturnType();
					if (currentTy && currentTy->isA(TypeTypePointer)) {
						auto ptrRetTy = std::static_pointer_cast<LILPointerType>(currentTy);
						auto ptrRetArg = ptrRetTy->getArgument();
						if (ptrRetArg->isA(TypeTypeObject)) {
							currentTy = ptrRetArg;
						}
					}
					
					this->_processArguments(fc.get(), method.get());
					break;
				}
					
				case NodeTypePropertyName:
				{
					auto pn = std::static_pointer_cast<LILPropertyName>(currentNode);
					if (currentTy && currentTy->isA(TypeTypePointer)) {
						auto ptrRetTy = std::static_pointer_cast<LILPointerType>(currentTy);
						auto ptrRetArg = ptrRetTy->getArgument();
						if (ptrRetArg->isA(TypeTypeObject)) {
							currentTy = ptrRetArg;
						}
					}
					if (!currentTy->isA(TypeTypeObject)) {
						std::cerr << "VALUE PATH NODE DOES NOT POINT TO OBJECT FAIL!!!!!!!!\n";
						return;
					}
					auto classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "CLASS NOT FOUND FAIL!!!!!!!!\n";
						return;
					}
					auto field = classDecl->getFieldNamed(pn->getName());
					if (!field) {
						std::cerr << "FIELD NOT FOUND FAIL!!!!!!!!\n";
						return;
					}
					if (!field->isA(NodeTypeVarDecl)) {
						std::cerr << "FIELD WAS NOT VAR DECL FAIL!!!!!!!!\n";
						return;
					}
					currentTy = field->getType();
					break;
				}

				default:
					break;
			}
			
			++it;
		}
	}
}

void LILParameterSorter::_processArguments(LILFunctionCall * fc, LILFunctionDecl * fd)
{
	auto fnTy = fd->getFnType();
	auto declArgs = fnTy->getArguments();
	auto callArgs = fc->getArguments();
	std::vector<std::shared_ptr<LILAssignment>> asgmtArgs;
	std::vector<std::shared_ptr<LILNode>> plainArgs;

	for (auto callArg : callArgs) {
		if (callArg->isA(NodeTypeAssignment)) {
			asgmtArgs.push_back(std::static_pointer_cast<LILAssignment>(callArg));
		} else {
			plainArgs.push_back(callArg);
		}
	}

	std::vector<std::shared_ptr<LILNode>> newArgs;

	//in the order of the arguments in the declaration
	size_t plainArgCount = 0;
	for (auto declArg : declArgs) {
		if (!declArg->isA(NodeTypeVarDecl)) {
			if ( plainArgs.size() >= (plainArgCount+1) ) {
				newArgs.push_back(plainArgs[plainArgCount]);
				plainArgCount += 1;
			}
			continue;
		}
		auto declVd = std::static_pointer_cast<LILVarDecl>(declArg);
		if (declVd->getName() == "@self") {
			continue;
		}
		
		//find the argument in the call
		bool found = false;
		for (auto asgmtArg : asgmtArgs) {
			auto callAsgmtSubj = asgmtArg->getSubject();
			if (!callAsgmtSubj->isA(NodeTypeVarName)) {
				std::cerr << "!!!!!!!!!!SUBJECT OF ASSIGNMENT WAS NOT VAR NAME FAIL!!!!!!!!!!!!!!!!\n";
				return;
			}
			auto caVn = std::static_pointer_cast<LILVarName>(callAsgmtSubj);
			if (declVd->getName() == caVn->getName()) {
				found = true;
				newArgs.push_back(asgmtArg);
				break;
			}
		}
		if (!found && plainArgs.size() >= plainArgCount+1) {
			auto newAsgmt = std::make_shared<LILAssignment>();
			newAsgmt->setSourceLocation(fc->getSourceLocation());
			auto newVn = std::make_shared<LILVarName>();
			newVn->setName(declVd->getName());
			newVn->setSourceLocation(fc->getSourceLocation());
			auto callArg = plainArgs[plainArgCount];
			auto callArgTy = callArg->getType();
			if (!callArg || !callArgTy) {
				std::cerr << "!!!!!!!!!!CALL ARG HAD NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
				return;
			}
			newAsgmt->setType(callArgTy->clone());
			
			newAsgmt->setSubject(newVn);
			newAsgmt->setValue(callArg);
			newArgs.push_back(newAsgmt);
			++plainArgCount;
			found = true;
		}
		
		//if we need the default value
		if (!found && declVd->getInitVal()) {
			newArgs.push_back(this->_varDeclToAssignment(declVd));
		}
	}

	if (plainArgs.size() > plainArgCount) {
		for (size_t i=plainArgCount; i<plainArgs.size(); i+=1) {
			newArgs.push_back(plainArgs[i]);
		}
	}
	
	fc->setArguments(std::move(newArgs));
	std::vector<std::shared_ptr<LILType>> newArgumentTypes;
	for (const auto & arg : fc->getArguments()) {
		auto ty = arg->getType();
		if (!ty) {
			std::cerr << "!!!!!!!!!!CALL ARG HAD NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
			continue;
		}
		newArgumentTypes.push_back(ty);
	}
	fc->setArgumentTypes(newArgumentTypes);
}

void LILParameterSorter::_process(LILFlowControl * value)
{
	this->processChildren(value->getThen());
	this->processChildren(value->getElse());
}

void LILParameterSorter::_process(LILFlowControlCall * value)
{
	
}

void LILParameterSorter::_process(LILInstruction * value)
{
}

void LILParameterSorter::_process(LILIfInstruction * value)
{
	for (auto thenNode : value->getThen()) {
		this->process(thenNode.get());
	}
	for (auto elseNode : value->getElse()) {
		this->process(elseNode.get());
	}
}

void LILParameterSorter::_process(LILValueList * value)
{
	this->processChildren(value->getValues());
}

void LILParameterSorter::_process(LILDocumentation * value)
{
}

void LILParameterSorter::_process(LILIndexAccessor * value)
{
	auto arg = value->getArgument();
	if (arg) {
		this->process(arg.get());
	}
}

void LILParameterSorter::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
	for (auto it = children.begin(); it!=children.end(); ++it)
	{
		this->process((*it).get());
	};
}

std::shared_ptr<LILAssignment> LILParameterSorter::_varDeclToAssignment(std::shared_ptr<LILVarDecl> vd)
{
	std::shared_ptr<LILAssignment> ret = std::make_shared<LILAssignment>();
	std::shared_ptr<LILVarName> vn = std::make_shared<LILVarName>();
	vn->setName(vd->getName());
	ret->setSubject(vn);
	auto initVal = vd->getInitVal();
	ret->setType(initVal->getType());
	ret->setValue(vd->getInitVal());
	return ret;
	
}
