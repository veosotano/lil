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
 *	  This file converts the ast into IR representation
 *
 ********************************************************************/


#include "LILIREmitter.h"

#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILEnum.h"
#include "LILIndexAccessor.h"
#include "LILMultipleType.h"
#include "LILNodeToString.h"
#include "LILPointerType.h"
#include "LILRootNode.h"
#include "LILSIMDType.h"
#include "LILStaticArrayType.h"
#include "LILTypeDecl.h"

#include "LLVMIRParser.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/AsmParser/LLParser.h"

#include "../shared/LILDOMBuilder.h"


#define LILIREMITTEROPTIMIZE
#define LIL_GEP_INDEX_SIZE 32
#define LIL_ARRAY_SMALL_BUFFER_SIZE 2
#define LIL_ARRAY_BIG_BUFFER_MIN_SIZE 10


using namespace LIL;

namespace LIL
{
	class LILIREmitterPrivate
	{
		friend class LILIREmitter;

		LILIREmitterPrivate(LILString name)
		: llvmContext()
		, irBuilder(llvmContext)
		, needsReturnValue(false)
		, currentAlloca(nullptr)
		, returnAlloca(nullptr)
		, ruleCount(0)
		, llvmModule(name.data(), llvmContext)
		{
		}
		llvm::LLVMContext llvmContext;
		llvm::IRBuilder<> irBuilder;
		llvm::Module llvmModule;
		std::unique_ptr<llvm::legacy::FunctionPassManager> functionPassManager;
		std::map<std::string, llvm::Value*> namedValues;
		std::vector<std::map<std::string, llvm::Value*>> hiddenLocals;
		std::map<std::string, llvm::StructType *> classTypes;
		bool needsReturnValue;
		llvm::Value * currentAlloca;
		llvm::Value * returnAlloca;
		llvm::BasicBlock * finallyBB;
		llvm::BasicBlock * afterLoopBB;
		int ruleCount;
		std::shared_ptr<LILElement> dom;
	};
}

LILIREmitter::LILIREmitter(LILString name)
: d(new LILIREmitterPrivate(name))
, _debug(false)
{
	d->functionPassManager = std::make_unique<llvm::legacy::FunctionPassManager>(&d->llvmModule);
	d->functionPassManager->add(llvm::createSROAPass());
	d->functionPassManager->add(llvm::createLICMPass());
	d->functionPassManager->add(llvm::createDeadCodeEliminationPass());
	d->functionPassManager->add(llvm::createAggressiveDCEPass());
	d->functionPassManager->add(llvm::createReassociatePass());
	d->functionPassManager->add(llvm::createGVNPass());
	d->functionPassManager->add(llvm::createCFGSimplificationPass());
	d->functionPassManager->doInitialization();
}

LILIREmitter::~LILIREmitter()
{
	delete d;
}

void LILIREmitter::reset()
{

}

llvm::Module * LILIREmitter::getLLVMModule() const
{
	return &d->llvmModule;
}

void LILIREmitter::initializeVisit()
{
	if (this->getVerbose()){
		std::cerr << "\n\n";
		std::cerr << "============================\n";
		std::cerr << "== LLVM IR REPRESENTATION ==\n";
		std::cerr << "============================\n\n";
	}
}

void LILIREmitter::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
	this->setRootNode(rootNode);
	this->emitRuleNames(rootNode.get());
	this->hoistDeclarations(rootNode);
	
	std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
	for (const auto & node : nodes) {
		auto nodeType = node->getNodeType();
		//global vars have already been emitted in the hoisting step
		if (nodeType != NodeTypeVarDecl) {
			this->emit(node.get());
		}
	}
	
	if (rootNode->hasRules()) {
		//terminate the apply rules fn if necessary
		std::string applyFnName = "LIL__applyRules";
		if (d->namedValues.count(applyFnName)) {
			llvm::Function * applyFn = llvm::cast<llvm::Function>(d->namedValues[applyFnName]);
			d->irBuilder.SetInsertPoint(&applyFn->getBasicBlockList().back());
			d->irBuilder.CreateRetVoid();
		}
	}
}

void LILIREmitter::emitRuleNames(LILRootNode * rootNode)
{
	for (const auto & rule : rootNode->getRules()) {
		this->emitRuleName(rule.get());
	}
}

void LILIREmitter::emitRuleName(LILRule * rule)
{
	auto newName = this->_newRuleFnName();
	rule->setFnName(newName);
	
	for (const auto & childRule : rule->getChildRules()) {
		this->emitRuleName(childRule.get());
	}
}

void LILIREmitter::hoistDeclarations(std::shared_ptr<LILRootNode> rootNode)
{
	std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
	for (const auto & node : nodes) {
		if (this->_debug) {
			std::cerr << "## hoisting " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " to the top ##\n";
		}
		
		switch (node->getNodeType()) {
			case NodeTypeClassDecl:
			{
				LILClassDecl * value = std::static_pointer_cast<LILClassDecl>(node).get();
				this->_emitClassSignature(value);
				break;
			}
			case NodeTypeFunctionDecl:
			{
				auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
				auto name = fd->getName().data();
				auto fun = this->_emitFnSignature(name, fd->getFnType().get());
				d->namedValues[name] = fun;
				break;
			}
			case NodeTypeVarDecl:
			{
				this->emit(node.get());
				break;
			}
			case NodeTypeRule:
			{
				auto rule = std::static_pointer_cast<LILRule>(node);
				this->emitRuleFnSignature(rule.get());
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

void LILIREmitter::emitRuleFnSignature(LILRule * rule)
{
	auto ty = rule->getType();
	assert(ty && "Rule had no type");
	const auto & tyName = ty->getName();
	if (tyName == "mainMenu" || tyName == "menuItem" || tyName == "menu") {
		return;
	}
	auto fnTy = std::make_shared<LILFunctionType>();
	auto fnName = rule->getFnName();
	auto ptrTy = std::make_shared<LILPointerType>();
	ptrTy->setArgument(ty->clone());
	auto thisVd = std::make_shared<LILVarDecl>();
	thisVd->setName("@this");
	thisVd->setType(ptrTy);
	fnTy->addArgument(thisVd);
	auto indexVd = std::make_shared<LILVarDecl>();
	indexVd->setName("@index");
	auto numTy = LILType::make("i64");
	indexVd->setType(numTy);
	fnTy->addArgument(indexVd);
	auto ruleFn = this->_emitFnSignature(fnName.data(), fnTy.get());
	d->namedValues[fnName.data()] = ruleFn;
	llvm::BasicBlock::Create(d->llvmContext, "entry", ruleFn);
	
	for (const auto & childRule : rule->getChildRules()) {
		this->emitRuleFnSignature(childRule.get());
	}
}

llvm::Value * LILIREmitter::emit(LILNode * node)
{
	if (this->_debug) {
		std::cerr << "## emitting " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node).data() + " ##\n";
	}

	switch (node->getNodeType()) {
		case NodeTypeBoolLiteral:
		{
			LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
			return this->_emitBool(value);
		}
		case NodeTypeNumberLiteral:
		{
			LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
			return this->_emitNum(value);
		}
		case NodeTypePercentage:
		{
			LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
			return this->_emitPercent(value);
		}
		case NodeTypeExpression:
		{
			LILExpression * value = static_cast<LILExpression *>(node);
			if (value->isA(ExpressionTypeCast)) {
				return this->_emitCast(value);
			} else {
				return this->_emitExp(value);
			}
		}
		case NodeTypeUnaryExpression:
		{
			LILUnaryExpression * value = static_cast<LILUnaryExpression *>(node);
			if (value->getUnaryExpressionType() == UnaryExpressionTypeNot) {
				return this->_emitNotExp(value);
			} else {
				return this->_emitUExp(value);
			}
		}
		case NodeTypeStringLiteral:
		{
			LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
			return this->_emitStr(value);
		}
		case NodeTypeNull:
		{
			LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
			return this->_emitNull(value);
		}
		case NodeTypeVarDecl:
		{
			LILVarDecl * value = static_cast<LILVarDecl *>(node);
			return this->_emitVarDecl(value);
		}
		case NodeTypeAliasDecl:
		case NodeTypeTypeDecl:
		case NodeTypeEnum:
		{
			//do nothing
			return nullptr;
		}
		case NodeTypeConversionDecl:
		{
			LILConversionDecl * value = static_cast<LILConversionDecl *>(node);
			return this->_emitConvDecl(value);
		}
		case NodeTypeClassDecl:
		{
			LILClassDecl * value = static_cast<LILClassDecl *>(node);
			return this->_emitClassDecl(value);
		}
		case NodeTypeObjectDefinition:
		{
			LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
			return this->_emitObjDef(value);
		}
		case NodeTypeAssignment:
		{
			LILAssignment * value = static_cast<LILAssignment *>(node);
			return this->_emitAsgmt(value);
		}
		case NodeTypeValuePath:
		{
			LILValuePath * value = static_cast<LILValuePath *>(node);
			return this->_emitVP(value);
		}
		case NodeTypePropertyName:
		{
			LILPropertyName * value = static_cast<LILPropertyName *>(node);
			return this->_emitPN(value);
		}
		case NodeTypeVarName:
		{
			LILVarName * value = static_cast<LILVarName *>(node);
			return this->_emitVN(value);
		}
		case NodeTypeRule:
		{
			LILRule * value = static_cast<LILRule *>(node);
			this->_emitRule(value);
			return nullptr;
		}
		case NodeTypeSimpleSelector:
		{
			LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
			return this->_emitSSel(value);
		}
		case NodeTypeSelector:
		{
			LILSelector * value = static_cast<LILSelector *>(node);
			return this->_emitSel(value);
		}
		case NodeTypeCombinator:
		{
			LILCombinator * value = static_cast<LILCombinator *>(node);
			return this->_emitComb(value);
		}
		case NodeTypeFilter:
		{
			LILFilter * value = static_cast<LILFilter *>(node);
			return this->_emitFlt(value);
		}
		case NodeTypeFlag:
		{
			LILFlag * value = static_cast<LILFlag *>(node);
			return this->_emitFlag(value);
		}
		case NodeTypeFunctionDecl:
		{
			LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
			return this->_emitFnDecl(value);
		}
		case NodeTypeFunctionCall:
		{
			LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
			return this->_emitFC(value);
		}
		case NodeTypeFlowControl:
		{
			LILFlowControl * value = static_cast<LILFlowControl *>(node);
			return this->_emitFlowC(value);
		}
		case NodeTypeFlowControlCall:
		{
			LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
			return this->_emitFlowCCall(value);
		}
		case NodeTypeInstruction:
		{
			LILInstruction * value = static_cast<LILInstruction *>(node);
			return this->_emitInstr(value);
		}
		case NodeTypeIfInstruction:
		case NodeTypeDocumentation:
		{
			//ignore
			return nullptr;
		}
		case NodeTypeForeignLang:
		{
			LILForeignLang * value = static_cast<LILForeignLang *>(node);
			return this->_emitForeignLang(value);
		}
		case NodeTypeValueList:
		{
			LILValueList * value = static_cast<LILValueList *>(node);
			return this->_emitValList(value);
		}
		case NodeTypeSnippetInstruction:
		{
			//ignore
			return nullptr;
		}

		default:
		{
			std::cerr << "Error: unkonwn node type to emit\n";
			return nullptr;
		}
	}
}

llvm::Value * LILIREmitter::_emitBool(LILBoolLiteral * value)
{
	return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(1, value->getValue(), false));
}

llvm::Value * LILIREmitter::_emitNum(LILNumberLiteral * value)
{
	const auto & tyNode = value->getType();
	if (!tyNode) {
		std::cerr << "!!!!!!!!!!NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	const auto & ty = std::static_pointer_cast<LILType>(tyNode);
	if (ty->getName() == "i8") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, value->getValue().toChar(), true));
	} else if (ty->getName() == "i16") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(16, value->getValue().toInt(), true));
	} else if (ty->getName() == "i32") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(32, value->getValue().toLong(), true));
	} else if (ty->getName() == "i64") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, value->getValue().toLongLong(), true));
	} else if (ty->getName() == "f32") {
		return llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(value->getValue().toFloat()));
	} else if (ty->getName() == "f64") {
		return llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(value->getValue().toDouble()));
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitPercent(LILPercentageLiteral * value)
{
	const auto & tyNode = value->getType();
	if (!tyNode) {
		std::cerr << "!!!!!!!!!!NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	const auto & ty = std::static_pointer_cast<LILType>(tyNode);
	if (ty->getName() == "i8%") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, value->getValue().toChar(), true));
	} else if (ty->getName() == "i16%") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(16, value->getValue().toInt(), true));
	} else if (ty->getName() == "i32%") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(32, value->getValue().toLong(), true));
	} else if (ty->getName() == "i64%") {
		return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, value->getValue().toLongLong(), true));
	} else if (ty->getName() == "f32%") {
		return llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(value->getValue().toFloat()));
	} else if (ty->getName() == "f64%") {
		return llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(value->getValue().toDouble()));
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitCast(LILExpression * value)
{
	std::shared_ptr<LILNode> left = value->getLeft();
	if (!left) {
		std::cerr << "LEFT NODE WAS NULL FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	std::shared_ptr<LILNode> right = value->getRight();
	if (!right || !right->isA(NodeTypeType)) {
		std::cerr << "RIGHT NODE WAS NOT TYPE FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}

	llvm::Value * leftV = this->emit(left.get());
	auto rightTy = std::static_pointer_cast<LILType>(right);
	auto leftTy = left->getType();
	if (leftTy && leftTy->equalTo(rightTy)) {
		return leftV;
	}

	auto llvmType = this->llvmTypeFromLILType(rightTy.get());
	//pointer to integer
	if (leftTy->isA(TypeTypePointer) && LILType::isNumberType(rightTy.get())) {
		return d->irBuilder.CreatePtrToInt(leftV, llvmType);
	//integer to pointer
	} else if (LILType::isNumberType(leftTy.get()) && rightTy->isA(TypeTypePointer)) {
		return d->irBuilder.CreateIntToPtr(leftV, llvmType);
	} else {
		auto leftLlvmTy = this->llvmTypeFromLILType(leftTy.get());
		//integer to floating point
		if (leftLlvmTy->isIntegerTy() && llvmType->isFloatingPointTy()) {
			return d->irBuilder.CreateSIToFP(leftV, llvmType);
		//floating point to integer
		} else if (leftLlvmTy->isFloatingPointTy() && llvmType->isIntegerTy()) {
			return d->irBuilder.CreateFPToSI(leftV, llvmType);
		//integers, either to bigger or to smaller type
		} else if (leftLlvmTy->isIntegerTy() && llvmType->isIntegerTy()) {
			return d->irBuilder.CreateSExtOrTrunc(leftV, llvmType);
		} else if (leftLlvmTy->isFloatingPointTy() && llvmType->isFloatingPointTy()) {
			//floating point, to bigger type
			if (leftLlvmTy->getPrimitiveSizeInBits() < llvmType->getPrimitiveSizeInBits()) {
				return d->irBuilder.CreateFPExt(leftV, llvmType);
			//floating point, to smaller type
			} else {
				return d->irBuilder.CreateFPTrunc(leftV, llvmType);
			}
		}
		//last resort, just cast the bits
		return d->irBuilder.CreateBitCast(leftV, llvmType);
	}
}

llvm::Value * LILIREmitter::_emitExp(LILExpression * value)
{
	std::shared_ptr<LILNode> left = value->getLeft();
	std::shared_ptr<LILNode> right = value->getRight();

	llvm::Value * leftV = this->emit(left.get());
	llvm::Value * rightV = this->emit(right.get());
	if (!leftV || !rightV) {
		std::cerr << "!!!!!!!!!!LEFT OR RIGHT EMIT FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	auto ty = value->getType();
	if (!ty) {
		std::cerr << "EXPRESION HAD NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	auto leftTy = left->getType();
	if (!leftTy) {
		std::cerr << "LEFT NODE OF EXPRESSION HAD NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	auto rightTy = right->getType();
	if (!rightTy) {
		std::cerr << "RIGHT NODE OF EXPRESSION HAD NO TYPE FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	if (leftTy->isA(TypeTypePointer) || rightTy->isA(TypeTypePointer)) {
		auto numLlvmTy = this->llvmTypeFromLILType(LILType::make("i64").get());
		if (leftTy->isA(TypeTypePointer) && LILType::isNumberType(rightTy.get())) {
			auto ptrTy = std::static_pointer_cast<LILPointerType>(leftTy);
			auto ptrArgTy = ptrTy->getArgument();
			leftV = d->irBuilder.CreatePtrToInt(leftV, numLlvmTy);
			
			if (!LILType::isIntegerType(rightTy.get())) {
				std::cerr << "POINTER ARITHMETIC NEEDS INTEGER FAIL!!!!!!!!!!!!!!!!\n";
				return nullptr;
			}
			size_t ptrArgSize = this->getSizeOfType(ptrArgTy);
			rightV = d->irBuilder.CreateSExt(rightV, numLlvmTy);
			rightV = d->irBuilder.CreateMul(rightV, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, ptrArgSize/8, false)));

		} else if (LILType::isNumberType(leftTy.get()) && rightTy->isA(TypeTypePointer)) {
			auto ptrTy = std::static_pointer_cast<LILPointerType>(rightTy);
			auto ptrArgTy = ptrTy->getArgument();
			rightV = d->irBuilder.CreatePtrToInt(rightV, numLlvmTy);
			
			if (!LILType::isIntegerType(leftTy.get())) {
				std::cerr << "POINTER ARITHMETIC NEEDS INTEGER FAIL!!!!!!!!!!!!!!!!\n";
				return nullptr;
			}
			size_t ptrArgSize = this->getSizeOfType(ptrArgTy);
			if (ptrArgSize >= 8){
				leftV = d->irBuilder.CreateSExt(leftV, numLlvmTy);
				leftV = d->irBuilder.CreateMul(leftV, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, ptrArgSize/8, false)));
			}
		} else if (leftTy->isA(TypeTypePointer) && rightTy->isA(TypeTypePointer)){
			leftV = d->irBuilder.CreatePtrToInt(leftV, numLlvmTy);
			rightV = d->irBuilder.CreatePtrToInt(rightV, numLlvmTy);
		} else if (leftTy->isA(TypeTypePointer) && rightTy->getName() == "null") {
			auto ptrTy = std::static_pointer_cast<LILPointerType>(leftTy);
			leftV = d->irBuilder.CreatePtrToInt(leftV, numLlvmTy);
			rightV = d->irBuilder.CreateSExt(rightV, numLlvmTy);
		} else if (leftTy->getName() == "null" && rightTy->isA(TypeTypePointer)) {
			leftV = d->irBuilder.CreateSExt(leftV, numLlvmTy);
			auto ptrTy = std::static_pointer_cast<LILPointerType>(rightTy);
			rightV = d->irBuilder.CreatePtrToInt(rightV, numLlvmTy);
		}
		auto result = this->_emitExpression(value->getExpressionType(), leftV, rightV);
		return d->irBuilder.CreateIntToPtr(result, this->llvmTypeFromLILType(ty.get()));
	}
	auto tyName = ty->getName();
	if (tyName == "f32" || tyName == "f64") {
		if (leftV->getType()->getTypeID() == llvm::Type::IntegerTyID) {
			leftV = d->irBuilder.CreateSIToFP(leftV, this->llvmTypeFromLILType(ty.get()));
		}
		if (rightV->getType()->getTypeID() == llvm::Type::IntegerTyID) {
			rightV = d->irBuilder.CreateSIToFP(rightV, this->llvmTypeFromLILType(ty.get()));
		}
	}

	if (leftV->getType() != rightV->getType()) {
		auto leftTy = left->getType();
		auto rightTy = right->getType();
		if (
			leftTy && LILType::isIntegerType(leftTy.get())
			&& rightTy && LILType::isIntegerType(rightTy.get())
		) {
			if (this->getSizeOfType(leftTy) > this->getSizeOfType(rightTy)) {
				rightV = d->irBuilder.CreateSExt(rightV, leftV->getType());
			} else {
				leftV = d->irBuilder.CreateSExt(leftV, rightV->getType());
			}
		} else {
			std::cerr << "!!!!!!!!!!LEFT AND RIGHT TYPE DONT MATCH FAIL!!!!!!!!!!!!!!!!\n";
			return nullptr;
		}
	}
	return this->_emitExpression(value->getExpressionType(), leftV, rightV);
}

llvm::Value * LILIREmitter::_emitExpression(ExpressionType expType, llvm::Value * leftV, llvm::Value * rightV)
{
	auto llvmTy = leftV->getType();
	if (llvmTy->getTypeID() == llvm::Type::FixedVectorTyID) {
		llvmTy = llvmTy->getContainedType(0);
	}
	switch (expType) {
		case ExpressionTypeSum:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
				{
					return d->irBuilder.CreateFAdd(leftV, rightV);
					break;
				}
				case llvm::Type::IntegerTyID:
				{
					return d->irBuilder.CreateAdd(leftV, rightV);
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
			break;
		}

		case ExpressionTypeSubtraction:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
				{
					return d->irBuilder.CreateFSub(leftV, rightV);
					break;
				}
				case llvm::Type::IntegerTyID:
				{
					return d->irBuilder.CreateSub(leftV, rightV);
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
			break;
		}

		case ExpressionTypeMultiplication:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
				{
					return d->irBuilder.CreateFMul(leftV, rightV);
					break;
				}
				case llvm::Type::IntegerTyID:
				{
					return d->irBuilder.CreateMul(leftV, rightV);
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
			break;
		}

		case ExpressionTypeDivision:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
				{
					return d->irBuilder.CreateFDiv(leftV, rightV);
					break;
				}
				case llvm::Type::IntegerTyID:
				{
					return d->irBuilder.CreateSDiv(leftV, rightV);
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
			break;
		}

		case ExpressionTypeBiggerComparison:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::IntegerTyID:
					return d->irBuilder.CreateICmpSGT(leftV, rightV);
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
					return d->irBuilder.CreateFCmpOGT(leftV, rightV);

				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
		}

		case ExpressionTypeBiggerOrEqualComparison:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::IntegerTyID:
					return d->irBuilder.CreateICmpSGE(leftV, rightV);
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
					return d->irBuilder.CreateFCmpOGE(leftV, rightV);
					
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
		}

		case ExpressionTypeSmallerComparison:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::IntegerTyID:
					return d->irBuilder.CreateICmpSLT(leftV, rightV);
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
					return d->irBuilder.CreateFCmpOLT(leftV, rightV);
					
				default:
					std::cerr << "!!!!!!!!!! UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
		}

		case ExpressionTypeSmallerOrEqualComparison:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::IntegerTyID:
					return d->irBuilder.CreateICmpSLE(leftV, rightV);
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
					return d->irBuilder.CreateFCmpOLE(leftV, rightV);
					
				default:
					std::cerr << "!!!!!!!!!! UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
		}
			
		case ExpressionTypeEqualComparison:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::IntegerTyID:
					return d->irBuilder.CreateICmpEQ(leftV, rightV);
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
					return d->irBuilder.CreateFCmpOEQ(leftV, rightV);

				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
		}
			
		case ExpressionTypeNotEqualComparison:
		{
			switch (llvmTy->getTypeID()) {
				case llvm::Type::IntegerTyID:
					return d->irBuilder.CreateICmpNE(leftV, rightV);
				case llvm::Type::FloatTyID:
				case llvm::Type::DoubleTyID:
					return d->irBuilder.CreateFCmpONE(leftV, rightV);
					
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
					break;
			}
		}

		case ExpressionTypeLogicalAnd:
		case ExpressionTypeBitwiseAnd:
		{
			return d->irBuilder.CreateAnd(leftV, rightV);
		}

		case ExpressionTypeLogicalOr:
		case ExpressionTypeBitwiseOr:
		{
			return d->irBuilder.CreateOr(leftV, rightV);
		}
		
		case ExpressionTypeXor:
		{
			return d->irBuilder.CreateXor(leftV, rightV);
		}

		case ExpressionTypeShiftLeft:
		{
			return d->irBuilder.CreateShl(leftV, rightV);
		}

		case ExpressionTypeShiftRight:
		{
			//using arithmetic shift because type is always signed
			return d->irBuilder.CreateAShr(leftV, rightV);
		}
		case ExpressionTypeMod:
		{
			return d->irBuilder.CreateSRem(leftV, rightV);
		}
			
		default:
			std::cerr << "!!!!!!!!!!UNKNOWN EXPRESSION TYPE FAIL!!!!!!!!!!!!!!!!\n";
			return nullptr;
	}

	std::cerr << "!!!!!!!!!!EMIT EXPRESSION FAIL!!!!!!!!!!!!!!!!\n";
	return nullptr;

}

llvm::Value * LILIREmitter::_emitUExp(LILUnaryExpression * value)
{
	std::shared_ptr<LILNode> val = value->getValue();
	auto subject = value->getSubject();
	llvm::Value * subjectV = this->emitPointer(subject.get());
	llvm::Value * valV = this->emit(val.get());
	if (!valV) {
		std::cerr << "!!!!!!!!!!VALUE EMIT FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	auto temp = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(subject->getType().get()), subjectV);
	auto expTy = temp->getType();
	if (
		valV->getType()->isIntegerTy()
		&& expTy->isIntegerTy()
		&& (expTy->getIntegerBitWidth() > valV->getType()->getIntegerBitWidth())
		) {
		valV = d->irBuilder.CreateSExt(valV, expTy);
	}
	llvm::Value * expVal = this->_emitExpression(LILUnaryExpression::uexpToExpType(value->getUnaryExpressionType()), temp, valV);
	if (expVal) {
		d->irBuilder.CreateStore(expVal, subjectV);
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitNotExp(LILUnaryExpression * value)
{
	std::shared_ptr<LILNode> val = value->getValue();
	llvm::Value * valV = this->emit(val.get());
	if (!valV) {
		std::cerr << "!!!!!!!!!!VALUE EMIT FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	switch (valV->getType()->getTypeID()) {
		case llvm::Type::IntegerTyID:
			return d->irBuilder.CreateNot(valV);

		default:
			std::cerr << "!!!!!!! UNKNOWN TYPE IN NOT EXPRESSION FAIL!!!!!!!!!!!!!!!!\n";
			return nullptr;
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitStr(LILStringLiteral * value)
{
	LILString stringLiteral = value->getValue();
	LILString stringWithoutQuotes = stringLiteral.stripQuotes();
	LILString stringEscaped = stringWithoutQuotes.replaceEscapes();
	auto str = stringEscaped.data();
	auto charType = llvm::IntegerType::get(d->llvmContext, 8);
	auto strLength = str.length();
	std::vector<llvm::Constant *> chars(strLength);
	for(unsigned int i = 0; i < str.size(); i++) {
		chars[i] = llvm::ConstantInt::get(charType, str[i]);
	}

	//add \0
	chars.push_back(llvm::ConstantInt::get(charType, 0));

	auto stringType = llvm::ArrayType::get(charType, chars.size());
	
	auto globalDeclaration = new llvm::GlobalVariable(d->llvmModule, stringType, true, llvm::GlobalVariable::ExternalLinkage, nullptr, "str");
	
	globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, chars));
	globalDeclaration->setConstant(true);
	globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
	globalDeclaration->setUnnamedAddr (llvm::GlobalValue::UnnamedAddr::Global);
	
	auto i8PtrTy = charType->getPointerTo();
	auto castedGlobal = llvm::ConstantExpr::getBitCast(globalDeclaration, i8PtrTy);
	
	if (value->getIsCString()) {
		return castedGlobal;
	}
	else
	{
		auto stringTy = d->classTypes["string"];
		//store the length
		auto lengthGep = this->_emitGEP(d->currentAlloca, stringTy, true, 0, "length", true, false, 0);
		d->irBuilder.CreateStore(llvm::ConstantInt::get(llvm::IntegerType::getInt64Ty(d->llvmContext), strLength), lengthGep);
		if (str.size() > 55) {
			std::vector<llvm::Value *> mallocArgs;
			auto intTy = llvm::IntegerType::get(d->llvmContext, 64);
			mallocArgs.push_back(llvm::ConstantInt::get(intTy, llvm::APInt(64, strLength, false)));
			auto mallocResult = d->irBuilder.CreateCall(d->llvmModule.getFunction("malloc"), mallocArgs);
			d->irBuilder.CreateMemCpy(mallocResult, llvm::MaybeAlign(), castedGlobal, llvm::MaybeAlign(), strLength+1);
			auto bufferGep = this->_emitGEP(d->currentAlloca, stringTy, true, 1, "buffer", true, false, 0);
			auto castedBuffer = d->irBuilder.CreatePointerCast(bufferGep, i8PtrTy->getPointerTo());
			d->irBuilder.CreateStore(mallocResult, castedBuffer);

		} else {
			//store the chars
			auto bufferGep = this->_emitGEP(d->currentAlloca, stringTy, true, 1, "buffer", true, false, 0);
			auto castedBuffer = d->irBuilder.CreatePointerCast(bufferGep, i8PtrTy);
			//length is +1 because we want the \0 too
			d->irBuilder.CreateMemCpy(castedBuffer, llvm::MaybeAlign(), castedGlobal, llvm::MaybeAlign(), strLength+1);
		}

		auto cd = this->findClassWithName("string");
		if (!cd) {
			std::cerr << "STRING CLASS NOT FOUND FAIL!!!!\n\n";
			return nullptr;
		}
		
		auto ctor = cd->getMethodNamed("construct");
		if (ctor) {
			if (!ctor->isA(NodeTypeFunctionDecl)) {
				std::cerr << "CONSTRUCTOR NODE WAS NOT FUNCTION DECL FAIL!!!\n\n";
				return nullptr;
			}
			auto ctorFd = std::static_pointer_cast<LILFunctionDecl>(ctor);
			auto fnTy = ctorFd->getFnType();
			LILString fnName = ctorFd->getName();
			llvm::Function* fun = d->llvmModule.getFunction(fnName.data());
			if (!fun) {
				fun = this->_emitFnSignature(fnName.data(), fnTy.get());
			}
			if (fun) {
				std::vector<llvm::Value *> argsvect;
				argsvect.push_back(d->currentAlloca);
				d->irBuilder.CreateCall(fun, argsvect);
			} else {
				std::cerr << "COULD NOT CALL DESTRUCTOR FAIL!!!!\n\n";
				return nullptr;
			}
		}
		
		return nullptr;
		
	}
}

llvm::Value * LILIREmitter::_emitNull(LILNullLiteral * value)
{
	return llvm::Constant::getNullValue(this->llvmTypeFromLILType(value->getType().get()));
}

llvm::Value * LILIREmitter::_emitVarDecl(LILVarDecl * value)
{
	auto name = value->getName().data();
	auto ty = value->getType();
	if (!ty) {
		std::cerr << "VAR DECL HAD NO TYPE FAIL!!!!\n\n";
		return nullptr;
	}
	if (ty->isA(TypeTypeFunction)) {
		if (value->getIsExtern()) {
			return this->_emitFnSignature(name, static_cast<LILFunctionType *>(ty.get()));
		} else {
			auto initVal = value->getInitVal();
			if (initVal) {
				return this->emit(initVal.get());
			} else {
				return nullptr;
			}
		}
	} else {
		if (value->getParentNode()->isA(NodeTypeRoot)) {
			if (value->getIsConst()) {
				//fixme
				return nullptr;
			}
			
			d->llvmModule.getOrInsertGlobal(name, this->llvmTypeFromLILType(ty.get()));
			auto globalVar = d->llvmModule.getNamedGlobal(name);
			globalVar->setLinkage( (value->getIsExtern() || value->getIsExported())  ? llvm::GlobalValue::ExternalLinkage : llvm::GlobalValue::InternalLinkage);
			d->namedValues[name] = globalVar;

			if (!value->getIsExtern()) {
				auto initVal = value->getInitVal();
				if (initVal) {
					switch (initVal->getNodeType()) {
						case NodeTypeBoolLiteral:
						{
							auto bl = this->emit(initVal.get());
							if (llvm::isa<llvm::Constant>(bl)) {
								globalVar->setInitializer(llvm::cast<llvm::Constant>(bl));
							} else {
								globalVar->setInitializer(llvm::Constant::getNullValue(this->llvmTypeFromLILType(ty.get())));
							}
						}
						case NodeTypeNumberLiteral:
						{
							auto iv = this->emit(initVal.get());
							if (llvm::isa<llvm::Constant>(iv)) {
								globalVar->setInitializer(llvm::cast<llvm::Constant>(iv));
							} else {
								globalVar->setInitializer(llvm::Constant::getNullValue(this->llvmTypeFromLILType(ty.get())));
							}
							break;
						}
						case NodeTypeStringLiteral:
						case NodeTypeObjectDefinition:
						case NodeTypeValueList:
						case NodeTypeVarName:
						case NodeTypeValuePath:
						case NodeTypeFunctionCall:
						{
							globalVar->setInitializer(llvm::Constant::getNullValue(this->llvmTypeFromLILType(ty.get())));
							
							//create the global initializator fn
							auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(d->llvmContext), false);
							llvm::Function * fun = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, d->llvmModule);
							llvm::BasicBlock * bb = llvm::BasicBlock::Create(d->llvmContext, "entry", fun);
							d->irBuilder.SetInsertPoint(bb);
							d->currentAlloca = globalVar;
							auto iv = this->emit(initVal.get());
							if (iv) {
								d->irBuilder.CreateStore(iv, globalVar);
							}
							d->irBuilder.CreateRetVoid();
							d->currentAlloca = nullptr;
							llvm::appendToGlobalCtors(d->llvmModule, fun, 0);
							break;
						}
						default:
						{
							globalVar->setInitializer(llvm::Constant::getNullValue(this->llvmTypeFromLILType(ty.get())));
							break;
						}
					}
				} else {
					globalVar->setInitializer(llvm::Constant::getNullValue(this->llvmTypeFromLILType(ty.get())));
				}
			}
			return globalVar;
		} else {
			//backup if needed
			llvm::Value * namedValue = nullptr;
			if (d->namedValues.count(name)) {
				namedValue = d->namedValues[name];
			}
			if (namedValue) {
				d->hiddenLocals.back()[name] = namedValue;
			}
			
			llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
			d->hiddenLocals.back()[name] = d->namedValues[name];
			auto localVarAlloca = this->createEntryBlockAlloca(fun, name, this->llvmTypeFromLILType(ty.get()));
			d->currentAlloca = localVarAlloca;
			d->namedValues[name] = localVarAlloca;
			
			auto initVal = value->getInitVal();
			if (initVal) {
				llvm::Value * llvmValue;
				if (ty->isA(TypeTypeMultiple))
				{
					auto multiTy = std::static_pointer_cast<LILMultipleType>(ty);
					llvmValue = this->emitForMultipleType(initVal.get(), multiTy);
				}
				else if (ty->getIsNullable())
				{
					llvmValue = this->emitNullable(initVal.get(), ty.get());
				}
				else
				{
					llvmValue = this->emit(initVal.get());
				}
				if (llvmValue) {
					this->_convertLlvmValueIfNeeded(&llvmValue, ty.get(), initVal->getType().get());
					if (
						!ty->isA(TypeTypePointer)
						&& initVal->getType()->isA(TypeTypePointer)
						) {
							auto valPtrTy = std::static_pointer_cast<LILPointerType>(initVal->getType());
							llvmValue = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(valPtrTy->getArgument().get()), llvmValue);
					}
					d->irBuilder.CreateStore(llvmValue, d->currentAlloca);
				}
			}
			d->currentAlloca = nullptr;
			return localVarAlloca;
		}
	}
	return nullptr;
}

bool LILIREmitter::_isAnyPtrTy(LILType * ty) const
{
	if (!ty->isA(TypeTypePointer)) {
		return false;
	}
	auto ptrTy = static_cast<LILPointerType *>(ty);
	auto arg = ptrTy->getArgument();
	if (!arg) {
		return false;
	}
	return arg->getName() == "any";
}

void LILIREmitter::_convertLlvmValueIfNeeded(llvm::Value ** llvmValue, LILType * dstTy, LILType * srcTy)
{
	if (this->_isAnyPtrTy(dstTy) || this->_isAnyPtrTy(srcTy)) {
		*llvmValue = d->irBuilder.CreatePointerCast(*llvmValue, this->llvmTypeFromLILType(dstTy));
	}
	const auto & dstName = dstTy->getName();
	const auto & srcName = srcTy->getName();
	if (dstName == "i8") {
		if (srcName == "bool") {
			*llvmValue = d->irBuilder.CreateSExt(*llvmValue, this->llvmTypeFromLILType(dstTy));
		}
	} else if (dstName == "i16") {
		if (srcName == "bool" || srcName == "i8") {
			*llvmValue = d->irBuilder.CreateSExt(*llvmValue, this->llvmTypeFromLILType(dstTy));
		}
	} else if (dstName == "i32") {
		if (srcName == "bool" || srcName == "i8" || srcName == "i16" ) {
			*llvmValue = d->irBuilder.CreateSExt(*llvmValue, this->llvmTypeFromLILType(dstTy));
		}
	} else if (dstName == "i64") {
		if (srcName == "bool" || srcName == "i8" || srcName == "i16" || srcName == "i32" ) {
			*llvmValue = d->irBuilder.CreateSExt(*llvmValue, this->llvmTypeFromLILType(dstTy));
		}
	} else if (dstName == "i128") {
		if (srcName == "bool" || srcName == "i8" || srcName == "i16" || srcName == "i32" || srcName == "i64" ) {
			*llvmValue = d->irBuilder.CreateSExt(*llvmValue, this->llvmTypeFromLILType(dstTy));
		}
	}
}

llvm::Value * LILIREmitter::_emitConvDecl(LILConversionDecl * value)
{
	auto vd = value->getVarDecl();
	auto encodedName = value->encodedName();
	auto body = value->getBody();
	auto ty = value->getType();
	if (!vd || body.size() == 0 || !ty) {
		std::cout << "!!!! EMIT CONVERSION DECL FAIL !!!!!!\n\n";
		return nullptr;
	}
	
	auto fd = std::make_shared<LILFunctionDecl>();
	auto fnTy = std::make_shared<LILFunctionType>();
	fnTy->setName("fn");
	fnTy->addArgument(vd);
	fnTy->setReturnType(ty);

	fd->setType(fnTy);
	
	LILString fnName = "_lil_conversion_"+encodedName;
	fd->setName(fnName);

	fd->setBody(body);
	this->getRootNode()->addNode(fd);
	
	this->_emitFn(fd.get());
	
	return nullptr;
}

llvm::Value * LILIREmitter::_emitClassSignature(LILClassDecl * value)
{
	if (value->isTemplate()) {
		return nullptr;
	}
	std::string name = value->getName().data();

	this->extractStructFromClass(value);

	for (auto methodPair : value->getMethods()) {
		auto methodNode = methodPair.second;
		auto fd = std::static_pointer_cast<LILFunctionDecl>(methodNode);
		if (fd->getHasMultipleImpls()) {
			for (const auto & impl: fd->getImpls()) {
				auto name = impl->getName().data();
				auto fun = this->_emitFnSignature(name, impl->getFnType().get());
				d->namedValues[name] = fun;
			}
		} else {
			auto name = fd->getName().data();
			auto fun = this->_emitFnSignature(name, fd->getFnType().get());
			d->namedValues[name] = fun;
		}
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitClassDecl(LILClassDecl * value)
{
	if (value->isTemplate()) {
		return nullptr;
	}
	std::string name = value->getName().data();

	this->extractStructFromClass(value);

	for (auto methodPair : value->getMethods()) {
		auto methodNode = methodPair.second;
		auto fd = std::static_pointer_cast<LILFunctionDecl>(methodNode);
		if (fd->getHasMultipleImpls()) {
			for (const auto & impl: fd->getImpls()) {
				this->_emitFn(impl.get());
			}
		} else {
			this->_emitFn(fd.get());
		}
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitObjDef(LILObjectDefinition * value)
{
	if (d->currentAlloca == nullptr) {
		std::cerr << "CURRENT ALLOCA WAS NULL FAIL !!!!!!!!\n\n";
		return nullptr;
	}
	LILString classNameStr = value->getType()->getName();
	std::string className = classNameStr.data();

	auto classValue = this->findClassWithName(classNameStr);
	size_t theIndex = 0;
	std::vector<std::shared_ptr<LILNode>> callValues;
	llvm::Value * alloca = d->currentAlloca;
	for (const auto & field : classValue->getFields()) {
		auto vd = std::static_pointer_cast<LILVarDecl>(field);
		
		bool needsStore = !vd->getIsVVar();

		auto varName = vd->getName();
		
		std::shared_ptr<LILNode> theVal = nullptr;
		for (auto node : value->getNodes()) {
			if (node->isA(NodeTypeAssignment)) {
				auto asgmt = std::static_pointer_cast<LILAssignment>(node);
				auto subj = asgmt->getSubject();
				std::shared_ptr<LILPropertyName> pn;
				if (subj) {
					if (subj->isA(NodeTypePropertyName)) {
						pn = std::static_pointer_cast<LILPropertyName>(subj);
						
					} else if (subj->isA(NodeTypeValuePath)){
						auto vp = std::static_pointer_cast<LILValuePath>(subj);
						std::shared_ptr<LILNode> firstNode = vp->getNodes().front();
						if (firstNode->isA(NodeTypePropertyName)) {
							pn = std::static_pointer_cast<LILPropertyName>(firstNode);
						}
					} else {
						std::cerr << "SUBJECT WAS NOT PROPERTY NAME OR VALUE PATH FAIL !!!!!!!!\n\n";
						return nullptr;
					}

					if (pn && pn->getName() == varName) {
						theVal = asgmt->getValue();
						break;
					}
				}
			} else {
				std::cerr << "NODE WAS NOT ASSIGNMENT FAIL !!!!!!!!\n\n";
				return nullptr;
			}
		}
		
		if (!theVal) {
			theVal = vd->getInitVal();
		}

		if (theVal) {
			if (needsStore) {
				auto gep = this->_emitGEP(alloca, classValue->getName(), theIndex, varName, true);
				auto allocaBackup = d->currentAlloca;
				d->currentAlloca = gep;
				auto vdTy = vd->getType();
				llvm::Value * llvmValue;
				if (vdTy->isA(TypeTypeMultiple)){
					llvmValue = this->emitForMultipleType(theVal.get(), std::static_pointer_cast<LILMultipleType>(vdTy));
				} else if (vdTy->getIsNullable()) {
					llvmValue = this->emitNullable(theVal.get(), vdTy.get());
				} else {
					llvmValue = this->emit(theVal.get());
				}
				if (llvmValue) {
					auto ty2 = theVal->getType();
					if (vdTy && ty2) {
						this->_convertLlvmValueIfNeeded(&llvmValue, vdTy.get(), ty2.get());
					}
					d->irBuilder.CreateStore(llvmValue, gep);
				}
				d->currentAlloca = allocaBackup;
			}
			else
			{
				callValues.push_back(theVal);
			}
		}
		if (!vd->getIsVVar()) {
			++theIndex;
		}
	}

	theIndex = 0;
	for (const auto & field : classValue->getFields()) {
		auto vd = std::static_pointer_cast<LILVarDecl>(field);
		if (vd->getIsIVar() || vd->getIsVVar()) {
			std::shared_ptr<LILNode> callVal;
			if (callValues.size() > theIndex) {
				callVal = callValues[theIndex];
			}
			if (!callVal) {
				continue;
			}
			LILString name = vd->getName();
			LILString methodName = "set" + name.toUpperFirstCase();
			auto methodNode = classValue->getMethodNamed(methodName);
			if (methodNode && methodNode->isA(NodeTypeFunctionDecl)) {
				auto methodFd = std::static_pointer_cast<LILFunctionDecl>(methodNode);
				auto fc = std::make_shared<LILFunctionCall>();
				fc->setName(methodName);
				fc->setParentNode(callVal->getParentNode());

				fc->addArgument(callVal);
				fc->addArgumentType(callVal->getType());
				if (methodFd->getHasMultipleImpls()) {
					this->_emitFCMultipleValues(methodFd->getImpls(), fc.get(), d->currentAlloca, value->getType());
				} else {
					LILString newName = this->decorate("", classValue->getName(), methodName, methodFd->getType());
					this->_emitFunctionCall(fc.get(), newName, methodFd->getFnType().get(), d->currentAlloca);
				}
			}
			theIndex += 1;
		}
	}

	//call the constructor
	if (classValue->getMethodNamed("construct")) {
		LILString decoratedName = this->decorate("", className, "construct", nullptr);
		llvm::Function* fun = d->llvmModule.getFunction(decoratedName.data());
		if (fun) {
			std::vector<llvm::Value *> argsvect;
			argsvect.push_back(alloca);
			d->irBuilder.CreateCall(fun, argsvect);
		}
	}

	return nullptr;
}

llvm::Value * LILIREmitter::_emitAsgmt(LILAssignment * asgmt)
{
	auto allocaBackup = d->currentAlloca;
	auto theValue = asgmt->getValue();
	if (theValue) {
		auto ty = asgmt->getType();
		auto subjectVal = asgmt->getSubject();
		if (subjectVal->isA(NodeTypeVarName)) {
			auto vd = std::static_pointer_cast<LILVarName>(subjectVal);
			d->currentAlloca = d->namedValues[vd->getName().data()];
			llvm::Value * llvmValue;
			if (ty->isA(TypeTypeMultiple))
			{
				auto multiTy = std::static_pointer_cast<LILMultipleType>(ty);
				llvmValue = this->emitForMultipleType(theValue.get(), multiTy);
			} else {
				llvmValue = this->emit(theValue.get());
			}

			if (llvmValue) {
				if (
					ty->isA(TypeTypePointer)
					&& llvmValue->getType()->getTypeID() != llvm::Type::PointerTyID
				){
					auto ptrTy = std::static_pointer_cast<LILPointerType>(ty);
					auto argTy = ptrTy->getArgument();
					if (this->_isAnyPtrTy(ptrTy.get())) {
						this->_convertLlvmValueIfNeeded(&llvmValue, ptrTy.get(), theValue->getType().get());
					} else {
						this->_convertLlvmValueIfNeeded(&llvmValue, argTy.get(), theValue->getType().get());
					}
					auto vdTy = vd->getType();
					d->currentAlloca = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(vdTy.get()), d->currentAlloca);
					d->irBuilder.CreateStore(llvmValue, d->currentAlloca);
				} else {
					this->_convertLlvmValueIfNeeded(&llvmValue, ty.get(), theValue->getType().get());
					d->irBuilder.CreateStore(llvmValue, d->currentAlloca);
				}
			}
			d->currentAlloca = allocaBackup;
			return nullptr;
		}

		if (!subjectVal->isA(NodeTypeValuePath)) {
			std::cerr << "!!!!!!!!!!SUBJECT OF ASSIGNMENT WAS NOT VALUE PATH OR VAR NAME FAIL !!!!!!!!!!!!!!!!\n";
			return nullptr;
		}

		auto vp = std::static_pointer_cast<LILValuePath>(subjectVal);
		
		const auto & childNodes = vp->getNodes();
		auto it = childNodes.begin();
		auto firstNode = *it;
		std::shared_ptr<LILType> currentTy;
		LILString instanceName;
		LILString stringRep;
		
		if (firstNode->isA(NodeTypeVarName)) {
			auto vn = std::static_pointer_cast<LILVarName>(firstNode);
			std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
			if (subjectNode && subjectNode->isA(NodeTypeVarDecl)) {
				auto vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
				auto instanceName = vd->getName().data();
				if (!d->namedValues.count(instanceName)) {
					std::cerr << instanceName + " NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
					return nullptr;
				}
				d->currentAlloca = d->namedValues[instanceName];
				stringRep = vn->getName();
				
				size_t startIndex = 1;
				std::shared_ptr<LILType> ifCastType;
				if (!this->inhibitSearchingForIfCastType) {
					ifCastType = this->findIfCastType(vp.get(), startIndex);
				}
				
				auto vdTy = vd->getType();
				if (!vdTy) {
					std::cerr << "TYPE OF VAR DECL WAS NULL FAIL !!!!!!!!!!!!!!!!\n";
					return nullptr;
				}
				
				if (ifCastType) {
					if (vdTy->getTypeType() == TypeTypeMultiple) {
						d->currentAlloca = this->emitUnwrappedPointerFromMT(d->currentAlloca, ifCastType.get(), static_cast<LILMultipleType *>(vdTy.get()));
					}
					currentTy = ifCastType;
				} else {
					currentTy = vdTy;
				}
			}
		} else {
			//selector
			auto sel = std::static_pointer_cast<LILSelector>(firstNode);
			switch (sel->getSelectorType()) {
				case SelectorTypeSelfSelector:
				{
					d->currentAlloca = d->namedValues["@self"];
					auto classDecl = this->findAncestorClass(asgmt->shared_from_this());
					currentTy = classDecl->getType();
					stringRep = "@self";
					break;
				}
				case SelectorTypeThisSelector:
				{
					d->currentAlloca = d->namedValues["@this"];
					auto rule = this->findAncestorRule(asgmt->shared_from_this());
					currentTy = rule->getType();
					stringRep = "@this";
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!\n";
					return nullptr;
			}
		}
		
		++it;
		size_t i = 0;
		
		while (it != childNodes.end()) {
			auto currentNode = *it;
			++it;
			++i;
			bool isLastNode = false;
			if (it == childNodes.end()) {
				isLastNode = true;
			}
			switch (currentNode->getNodeType()) {
				case NodeTypeFunctionCall:
				{
					if (isLastNode) {
						std::cerr << "!!!!!!!!!!CANNOT ASSIGN TO FUNCTION CALL FAIL!!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto fc = std::static_pointer_cast<LILFunctionCall>(currentNode);

					auto classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto method = classDecl->getMethodNamed(fc->getName());
					if (!method->isA(NodeTypeFunctionDecl)) {
						std::cerr << "!!!!!!!!!!NODE IS NOT FUNCTION DECL FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}

					auto targetFn = std::static_pointer_cast<LILFunctionDecl>(method);

					auto fnTy = targetFn->getFnType();
					if (targetFn->getHasMultipleImpls()) {
						d->currentAlloca = this->_emitFCMultipleValues(targetFn->getImpls(), fc.get(), d->currentAlloca, currentTy);
					} else {
						d->currentAlloca = this->_emitFunctionCall(fc.get(), targetFn->getName(), fnTy.get(), d->currentAlloca);
						stringRep += "()";
					}

					currentTy = fnTy->getReturnType();
					if (currentTy && currentTy->isA(TypeTypePointer)) {
						auto ptrRetTy = std::static_pointer_cast<LILPointerType>(currentTy);
						auto ptrRetArg = ptrRetTy->getArgument();
						if (ptrRetArg->isA(TypeTypeObject)) {
							currentTy = ptrRetArg;
						}
					}
					if (!currentTy) {
						std::cerr << "FUNCTION CALL IN ASSIGNMENT HAD NO RETURN TY FAIL!!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					break;
				}
					
				case NodeTypePropertyName:
				{
					if (d->currentAlloca == nullptr) {
						std::cerr << "!!!!!!!!!!SUBJECT OF VALUE PATH WAS NULL!!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto pn = std::static_pointer_cast<LILPropertyName>(currentNode);
					const auto & pnName = pn->getName();
					stringRep += "." + pnName;

					if (currentTy->isA(TypeTypePointer)) {
						d->currentAlloca = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(currentTy.get()), d->currentAlloca);
						auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
						currentTy = ptrTy->getArgument();
					}
					auto classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto field = classDecl->getFieldNamed(pnName);
					if (!field || !field->isA(NodeTypeVarDecl)) {
						std::cerr << "!!!!!!!!!!NODE IS NOT VAR DECL FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto vd = std::static_pointer_cast<LILVarDecl>(field);

					if (
						(!isLastNode || !vp->getPreventEmitCallToIVar() )
						&& field
						&& field->isA(NodeTypeVarDecl)
						&& ( vd->getIsIVar() || vd->getIsVVar() )
						) {
						LILString methodName = (isLastNode ? "set" : "get") + pnName.toUpperFirstCase();
						auto methodNode = classDecl->getMethodNamed(methodName);
						if (!methodNode->isA(NodeTypeFunctionDecl)) {
							std::cerr << "NODE IS NOT FUNCTION DECL FAIL !!!!!!!!!!!!!!!!\n";
							return nullptr;
						}
						auto fd = std::static_pointer_cast<LILFunctionDecl>(methodNode);
						auto fnTy = fd->getFnType();
						auto fc = std::make_shared<LILFunctionCall>();
						fc->setParentNode(asgmt->shared_from_this());
						fc->setName(methodName);
						if (isLastNode) {
							fc->addArgument(theValue);
							fc->addArgumentType(theValue->getType());
						}
						if (fd->getHasMultipleImpls()) {
							d->currentAlloca = this->_emitFCMultipleValues(fd->getImpls(), fc.get(), d->currentAlloca, currentTy);
						} else {
							d->currentAlloca = this->_emitFunctionCall(fc.get(), fd->getName(), fnTy.get(), d->currentAlloca);
						}
						if (isLastNode) {
							d->currentAlloca = allocaBackup;
							return nullptr;
						} else {
							auto retTy = fnTy->getReturnType();
							if (!retTy) {
								std::cerr << "RET TY WAS NULL FAIL !!!!!!!!!!!!!!\n";
								return nullptr;
							}
							currentTy = retTy;
						}
						
					} else {
						std::string name = pn->getName().data();
						auto vdTy = vd->getType();

						bool fieldFound = false;
						size_t theIndex = classDecl->getIndexOfField(field, fieldFound);
						if (!fieldFound) {
							std::cerr << "FIELD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n\n";
							return nullptr;
						}
						d->currentAlloca = this->_emitGEP(d->currentAlloca, currentTy->getName(), theIndex, stringRep, true);
						currentTy = vdTy;

						if (isLastNode)
						{
							llvm::Value * llvmValue;
							if (vdTy->isA(TypeTypeMultiple))
							{
								auto multiTy = std::static_pointer_cast<LILMultipleType>(vdTy);
								llvmValue = this->emitForMultipleType(theValue.get(), multiTy);
							} else {
								llvmValue = this->emit(theValue.get());
							}
							if (llvmValue) {
								if (
									ty->isA(TypeTypePointer)
									&& llvmValue->getType()->getTypeID() != llvm::Type::PointerTyID
								){
									auto ptrTy = std::static_pointer_cast<LILPointerType>(ty);
									ty = ptrTy->getArgument();
									d->currentAlloca = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(ty.get()), d->currentAlloca);
								}
								this->_convertLlvmValueIfNeeded(&llvmValue, ty.get(), theValue->getType().get());
								d->irBuilder.CreateStore(llvmValue, d->currentAlloca);
							}
							d->currentAlloca = allocaBackup;
							return nullptr;
						}
						else if (!this->inhibitSearchingForIfCastType && vdTy->isA(TypeTypeMultiple))
						{
							size_t outStartIndex;
							auto ifCastTy = this->findIfCastType(vp.get(), outStartIndex);
							if (outStartIndex != i+1) {
								std::cerr << "!!!!!!!!!!WRONG VALUE PATH FAIL !!!!!!!!!!!!!!!!\n\n";
								return nullptr;
							}
							d->currentAlloca = this->emitUnwrappedPointerFromMT(d->currentAlloca, ifCastTy.get(), static_cast<LILMultipleType *>(vdTy.get()));
							currentTy = ifCastTy;
						}
					}
					break;
				}
					
				case NodeTypeIndexAccessor:
				{
					auto ia = std::static_pointer_cast<LILIndexAccessor>(currentNode);
					auto arg = ia->getArgument();
					switch (currentTy->getTypeType()) {
						case TypeTypeStaticArray:
						{
							auto saTy = std::static_pointer_cast<LILStaticArrayType>(currentTy);
							
							std::vector<llvm::Value *> idList;
							//step through the pointer
							idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
							//add the index into the array
							auto argIr = this->emit(arg.get());
							if (!argIr) {
								std::cerr << "CODEGEN OF ARGUMENT OF INDEX ACCESSOR FAILED!!!!!!!!!!!!!!!!\n";
								return nullptr;
							}
							idList.push_back(argIr);
							d->currentAlloca = llvm::GetElementPtrInst::Create(this->llvmTypeFromLILType(currentTy.get()), d->currentAlloca, idList, "", d->irBuilder.GetInsertBlock());
							if (isLastNode) {
								llvm::Value * llvmValue = nullptr;
								auto saChildTy = saTy->getType();
								if (saChildTy->isA(TypeTypeMultiple))
								{
									auto multiTy = std::static_pointer_cast<LILMultipleType>(saChildTy);
									llvmValue = this->emitForMultipleType(theValue.get(), multiTy);
								} else {
									llvmValue = this->emit(theValue.get());
								}
								this->_convertLlvmValueIfNeeded(&llvmValue, ty.get(), theValue->getType().get());

								if (llvmValue) {
									d->irBuilder.CreateStore(llvmValue, d->currentAlloca);
								}
								d->currentAlloca = allocaBackup;
								return nullptr;
							} else {
								currentTy = saTy->getType();
							}
							break;
						}
						default:
							std::cerr << "!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
							return nullptr;
					}
					break;
				}
					
				default:
					break;
			}
		}
	} else {
		std::cerr << "!!!!!!!!!!ASSIGNMENT HAD NO VALUE !!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	d->currentAlloca = allocaBackup;
	return nullptr;
}

llvm::Value * LILIREmitter::_emitVP(LILValuePath * value)
{
	const auto & childNodes = value->getNodes();
	if (childNodes.size() == 1)
	{
		return this->emit(childNodes.at(0).get());
	}
	else if (childNodes.size() > 1)
	{
		auto it = childNodes.begin();
		auto firstNode = *it;
		llvm::Value * llvmSubject = nullptr;
		std::shared_ptr<LILType> currentTy;
		LILString instanceName;
		LILString stringRep;

		if (firstNode->isA(NodeTypeVarName)) {
			auto vn = std::static_pointer_cast<LILVarName>(firstNode);
			std::shared_ptr<LILNode> subjectNode = this->recursiveFindNode(vn);
			if (subjectNode) {
				if (subjectNode->isA(NodeTypeVarDecl)) {
					auto vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
					instanceName = vd->getName();
					llvmSubject = d->namedValues[instanceName.data()];
					stringRep = vn->getName();

					size_t startIndex = 1;
					std::shared_ptr<LILType> ifCastType;
					if (!this->inhibitSearchingForIfCastType) {
						ifCastType = this->findIfCastType(value, startIndex);
					}
					auto vdTy = vd->getType();
					if (!vdTy) {
						std::cerr << "TYPE OF VAR DECL WAS NULL FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					if (ifCastType) {
						if (vdTy->getTypeType() == TypeTypeMultiple) {
							llvmSubject = this->emitUnwrappedPointerFromMT(llvmSubject, ifCastType.get(), static_cast<LILMultipleType *>(vdTy.get()));
						}
						currentTy = ifCastType;
					} else {
						currentTy = vdTy;
					}
					
				} else if (subjectNode->isA(NodeTypeEnum)) {
					auto enm = std::static_pointer_cast<LILEnum>(subjectNode);
					auto pnNode = childNodes.at(1);
					if (pnNode->getNodeType() == NodeTypePropertyName) {
						auto pn = std::static_pointer_cast<LILPropertyName>(pnNode);
						auto enmValue = enm->getValueNamed(pn->getName());
						if (!enmValue) {
							std::cerr << "ENUM VALUE NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
							return nullptr;
						}
						return this->emit(enmValue.get());
					}
				}
			}
		} else {
			//selector
			auto sel = std::static_pointer_cast<LILSelector>(firstNode);
			switch (sel->getSelectorType()) {
				case SelectorTypeSelfSelector:
				{
					llvmSubject = d->namedValues["@self"];
					auto classDecl = this->findAncestorClass(value->shared_from_this());
					currentTy = classDecl->getType();
					stringRep = "@self";
					break;
				}
				case SelectorTypeThisSelector:
				{
					llvmSubject = d->namedValues["@this"];
					auto rule = this->findAncestorRule(value->shared_from_this());
					currentTy = rule->getType();
					stringRep = "@this";
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!\n";
					return nullptr;
			}
		}

		++it;
		size_t i = 0;

		while (it != childNodes.end()) {
			auto currentNode = *it;
			++it;
			++i;
			bool isLastNode = it == childNodes.end();
			if (llvmSubject == nullptr) {
				std::cerr << "!!!!!!!!!!SUBJECT OF VALUE PATH WAS NULL!!!!!!!!!!!!!!!!\n";
				return nullptr;
			}
			switch (currentNode->getNodeType()) {
				case NodeTypeFunctionCall:
				{
					auto fc = std::static_pointer_cast<LILFunctionCall>(currentNode);
					if (currentTy->isA(TypeTypePointer)) {
						llvmSubject = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(currentTy.get()), llvmSubject);
						auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
						currentTy = ptrTy->getArgument();
					}
					auto classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto method = classDecl->getMethodNamed(fc->getName());
					if (!method) {
						std::cerr << "METHOD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					if (!method->isA(NodeTypeFunctionDecl)) {
						std::cerr << "!!!!!!!!!!NODE IS NOT FUNCTION DECL FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}

					auto targetFn = std::static_pointer_cast<LILFunctionDecl>(method);
					auto fnTy = targetFn->getFnType();
					if (targetFn->getHasMultipleImpls()) {
						std::vector<std::shared_ptr<LILFunctionDecl>> impls = targetFn->getImpls();
						llvmSubject = this->_emitFCMultipleValues(impls, fc.get(), llvmSubject, currentTy);
					} else {
						llvmSubject = this->_emitFunctionCall(fc.get(), targetFn->getName(), targetFn->getFnType().get(), llvmSubject);
					}
					stringRep += "()";
					
					if (isLastNode) {
						return llvmSubject;
					} else {
						auto retTy = fnTy->getReturnType();
						if (!retTy) {
							std::cerr << "RETURN TYPE WAS NULL FAIL !!!!!!!!!!!!!!!!\n";
							return nullptr;
						}
						currentTy = retTy;
					}
					break;
				}

				case NodeTypePropertyName:
				{
					auto pn = std::static_pointer_cast<LILPropertyName>(currentNode);
					const auto & pnName = pn->getName();
					stringRep += "." + pnName;

					if (currentTy->isA(TypeTypePointer)) {
						llvmSubject = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(currentTy.get()), llvmSubject);
						auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
						currentTy = ptrTy->getArgument();
					}
					if (!currentTy->isA(TypeTypeObject)) {
						std::cerr << "CURRENT TYPE WAS NOT OBJECT TYPE FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}

					auto classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto field = classDecl->getFieldNamed(pnName);
					if (!field) {
						std::cerr << "!!!!!!!!!!FIELD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					if (!field->isA(NodeTypeVarDecl)) {
						std::cerr << "!!!!!!!!!!NODE IS NOT VAR DECL FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto vd = std::static_pointer_cast<LILVarDecl>(field);
					if (
						(!isLastNode || !value->getPreventEmitCallToIVar() )
						&& field
						&& field->isA(NodeTypeVarDecl)
						&& ( vd->getIsIVar() || vd->getIsVVar() )
					) {
						LILString methodName = "get" + pnName.toUpperFirstCase();
						auto methodVd = classDecl->getMethodNamed(methodName);

						auto fnTy = std::static_pointer_cast<LILFunctionType>(methodVd->getType());
						auto fnTyWithoutSelf = fnTy->clone();
						fnTyWithoutSelf->removeFirstArgument();
						LILString newName = this->decorate("", classDecl->getName(), methodName, fnTyWithoutSelf);
						llvm::Function* fun = d->llvmModule.getFunction(newName.data());
						if (!fun) {
							fun = this->_emitFnSignature(newName.data(), fnTy.get());
						}
						std::vector<llvm::Value *> argsvect;
						argsvect.push_back(llvmSubject);
						if (fun) {
							llvmSubject = d->irBuilder.CreateCall(fun, argsvect);
							if (isLastNode) {
								return llvmSubject;
							} else {
								auto ty = methodVd->getType();
								if (!ty->isA(TypeTypeFunction)) {
									std::cerr << "!!!!!!!!!!TYPE IS NOT FUNCTION TYPE FAIL !!!!!!!!!!!!!!!!\n";
									return nullptr;
								}
								auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
								auto retTy = fnTy->getReturnType();
								if (!retTy) {
									std::cerr << "RET TY WAS NULL FAIL !!!!!!!!!!!!!!\n";
									return nullptr;
								}
								currentTy = retTy;
							}
						} else {
							std::cerr << "!!!!!!!!!!COULD NOT FIND GETTER FAIL!!!!!!!!!!!!!!!!\n";
						}

					} else {
						
						
						std::string name = pn->getName().data();
						auto vdTy = vd->getType();
						if (!this->inhibitSearchingForIfCastType && vdTy->isA(TypeTypeMultiple)) {
							size_t outStartIndex;
							auto ifCastTy = this->findIfCastType(value, outStartIndex);
							if (outStartIndex != i+1) {
								std::cerr << "!!!!!!!!!!WRONG VALUE PATH FAIL !!!!!!!!!!!!!!!!\n\n";
								return nullptr;
							}
							llvmSubject = this->emitUnwrappedPointerFromMT(llvmSubject, ifCastTy.get(), static_cast<LILMultipleType *>(vdTy.get()));
							currentTy = ifCastTy;
						} else {
							bool fieldFound = false;
							size_t theIndex = classDecl->getIndexOfField(field, fieldFound);
							if (!fieldFound) {
								std::cerr << "FIELD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n\n";
								return nullptr;
							}
							llvmSubject = this->_emitGEP(llvmSubject, currentTy->getName(), theIndex, stringRep, true);
							currentTy = vdTy;
						}
						if (isLastNode) {
							return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(currentTy.get()), llvmSubject, name.data());
						}
					}
					break;
				}
					
				case NodeTypeIndexAccessor:
				{
					auto ia = std::static_pointer_cast<LILIndexAccessor>(currentNode);
					auto arg = ia->getArgument();
					switch (currentTy->getTypeType()) {
						case TypeTypeStaticArray:
						{
							auto saTy = std::static_pointer_cast<LILStaticArrayType>(currentTy);
							
							std::vector<llvm::Value *> idList;
							//step through the pointer
							idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
							//add the index into the array
							auto argIr = this->emit(arg.get());
							if (!argIr) {
								std::cerr << "CODEGEN OF ARGUMENT OF INDEX ACCESSOR FAILED!!!!!!!!!!!!!!!!\n";
								return nullptr;
							}
							idList.push_back(argIr);
							llvmSubject = llvm::GetElementPtrInst::Create(this->llvmTypeFromLILType(currentTy.get()), llvmSubject, idList, "", d->irBuilder.GetInsertBlock());
							if (isLastNode) {
								auto elemTy = saTy->getType();
								return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(elemTy.get()), llvmSubject);
							} else {
								currentTy = saTy->getType();
							}
							break;
						}
						case TypeTypeObject:
						{
							const auto & className = currentTy->getName();
							const auto & cd = this->findClassWithName(className);
							auto method = cd->getMethodNamed("at");
							if (!method) {
								std::cerr << "CLASS " + className.data() + " HAD NOT at METHOD FAIL!!!!\n\n";
								return nullptr;
							}
							if (!method->isA(NodeTypeFunctionDecl)) {
								std::cerr << "BAD METHOD FAIL!!!!!!!\n\n";
								return nullptr;
							}
							auto fd = std::static_pointer_cast<LILFunctionDecl>(method);
							auto fc = std::make_shared<LILFunctionCall>();  
							fc->setFunctionCallType(FunctionCallTypeValuePath);
							fc->setName(fd->getName());
							fc->addArgument(ia->getArgument());
							fc->setParentNode(value->shared_from_this());
							llvmSubject = this->_emitFunctionCall(fc.get(), fd->getName(), fd->getFnType().get(), llvmSubject);
							break;
						}
						default:
							std::cerr << "!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
							return nullptr;
					}
					break;
				}

				default:
					break;
			}
		}
		return llvmSubject;
	}
	std::cerr << "!!!!!!!!!!EMIT VALUE PATH FAIL!!!!!!!!!!!!!!!!\n";
	return nullptr;
}

llvm::Value * LILIREmitter::_emitGEP(llvm::Value * llvmValue, LILString className, LILUnitI32 fieldIndex, LILString fieldName, bool stepThroughPointer)
{
	auto name = className.data();
	if (d->classTypes.count(name)) {
		return this->_emitGEP(llvmValue, d->classTypes[name], true, fieldIndex, fieldName, stepThroughPointer, false, 0);
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitGEP(llvm::Value * llvmValue, llvm::Type * llvmType, bool useField, LILUnitI32 fieldIndex, LILString fieldName, bool stepThroughPointer, bool useArrayIndex, LILUnitI32 arrayIndex)
{
	std::vector<llvm::Value *> idList;
	if (stepThroughPointer) {
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	}
	if (useArrayIndex) {
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, arrayIndex, false)));
	}
	if (useField) {
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, fieldIndex, false)));
	}
	return llvm::GetElementPtrInst::Create(llvmType, llvmValue, idList, fieldName.data(), d->irBuilder.GetInsertBlock());
}

llvm::Value * LILIREmitter::_emitPN(LILPropertyName * value)
{
	std::cerr << "!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
	return nullptr;
}

llvm::Value * LILIREmitter::_emitRule(LILRule * value)
{
	auto parentId = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, -1, true));
	auto parentIndex = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 0, true));
	auto ret = this->_emitRuleInner(value);
	this->_connectRule(value, parentId, parentIndex, nullptr);
	return ret;
}

llvm::Value * LILIREmitter::_emitRuleInner(LILRule * value)
{
	auto ty = value->getType();
	assert(ty && "Rule must have a type");
	const auto & tyName = ty->getName();
	if (tyName == "mainMenu" || tyName == "menuItem" || tyName == "menu") {
		return nullptr;
	}
	const auto & fnName = value->getFnName();
	auto ruleFn = llvm::cast<llvm::Function>(d->namedValues[fnName.data()]);
	d->irBuilder.SetInsertPoint(&ruleFn->getBasicBlockList().back());

	auto thisArg = ruleFn->args().begin();
	d->namedValues["@this"] = thisArg;
	thisArg->setName("@this");
	
	auto indexArg = ruleFn->args().begin() + 1;
	d->namedValues["@index"] = indexArg;
	d->namedValues["@i"] = indexArg;
	indexArg->setName("@index");

	for (const auto & val : value->getValues()) {
		this->emit(val.get());
	}

	d->irBuilder.CreateRetVoid();
	
	const auto & childRules = value->getChildRules();
	if (childRules.size() > 0) {
		for (auto child : childRules) {
			this->_emitRuleInner(child.get());
		}
	}
	
	return ruleFn;
}

void LILIREmitter::_connectRule(LILRule * value, llvm::Value * parentId, llvm::Value * parentIndex, llvm::Value * thisObj)
{
	auto ty = value->getType();
	assert(ty && "Rule must have a type");
	const auto & tyName = ty->getName();
	if (tyName == "mainMenu" || tyName == "menuItem" || tyName == "menu") {
		return;
	}
	const auto & fnName = value->getFnName();
	auto ruleFn = llvm::cast<llvm::Function>(d->namedValues[fnName.data()]);

	//insert call into apply rules function
	std::string applyFnName = "LIL__applyRules";
	llvm::Function * applyFn;
	if (d->namedValues.count(applyFnName)) {
		applyFn = llvm::cast<llvm::Function>(d->namedValues[applyFnName]);
		if (applyFn->getBasicBlockList().size() == 0) {
			llvm::BasicBlock * bb = llvm::BasicBlock::Create(d->llvmContext, "entry", applyFn);
			d->irBuilder.SetInsertPoint(bb);
		} else {
			d->irBuilder.SetInsertPoint(&applyFn->getBasicBlockList().back());
		}
	} else {
		auto emptyFnTy = std::make_shared<LILFunctionType>();
		applyFn = this->_emitFnSignature(applyFnName, emptyFnTy.get());
		llvm::BasicBlock * bb = llvm::BasicBlock::Create(d->llvmContext, "entry", applyFn);
		d->irBuilder.SetInsertPoint(bb);
	}

	const auto & selChNode = value->getSelectorChain();
	if (!selChNode || selChNode->getNodeType() != NodeTypeSelectorChain) {
		std::cerr << "NODE WAS NOT SELECTOR CHAIN FAIL!!!!!!!\n\n";
		return;
	}
	auto selCh = std::static_pointer_cast<LILSelectorChain>(selChNode);

	//create element if needed
	const auto & instrNode = value->getInstruction();
	if (instrNode && instrNode->getInstructionType() == InstructionTypeNew) {
		long int iterations = 1;
		auto instr = std::static_pointer_cast<LILInstruction>(instrNode);
		const auto & arg = instr->getArgument();
		if (arg && arg->getNodeType() == NodeTypeNumberLiteral) {
			auto numLit = std::static_pointer_cast<LILNumberLiteral>(arg);
			iterations = stol(numLit->getValue().data());
		}
		for (long int index = 0; index < iterations; index += 1) {
			d->currentAlloca = d->irBuilder.CreateAlloca(this->llvmTypeFromLILType(ty.get()));
			auto cd = this->findClassWithName(ty->getName());
			
			auto initializeMethod = cd->getMethodNamed("initialize");
			if (initializeMethod) {
				if (!initializeMethod->isA(NodeTypeFunctionDecl)) {
					std::cerr << "NODE WAS NOT FUNCTION DECL FAIL!!!!!!!!!!!!!!!!\n\n";
					return;
				}
				auto fd = std::static_pointer_cast<LILFunctionDecl>(initializeMethod);
				llvm::Function* initializeFn = d->llvmModule.getFunction(fd->getName().data());
				if (initializeFn) {
					std::vector<llvm::Value *> argsvect;
					//@self
					argsvect.push_back(d->currentAlloca);
					//name
					argsvect.push_back(this->_getContainerNameFromSelectorChain(selCh));
					//parentId
					argsvect.push_back(parentId);
					auto initializeReturn = d->irBuilder.CreateCall(initializeFn, argsvect);
					
					auto llvmTy = this->llvmTypeFromLILType(ty.get());
					d->currentAlloca = d->irBuilder.CreateAlloca(llvmTy, 0, "");
					//hack: using the array index to get to the id field of super
					auto gep = this->_emitGEP(d->currentAlloca, llvmTy, true, 0, "id", true, true, 0);
					d->irBuilder.CreateStore(initializeReturn, gep);
					std::vector<llvm::Value *> ruleFnArgs;
					ruleFnArgs.push_back(d->currentAlloca);
					llvm::Value * indexLlvmVal;
					if (iterations > 1) {
						indexLlvmVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, index, true));
					} else {
						indexLlvmVal = parentIndex;
					}
					ruleFnArgs.push_back(indexLlvmVal);
					d->irBuilder.CreateCall(ruleFn, ruleFnArgs);

					const auto & childRules = value->getChildRules();
					if (childRules.size() > 0) {
						for (auto child : childRules) {
							this->_connectRule(child.get(), initializeReturn, indexLlvmVal, d->currentAlloca);
						}
					}
				} else {
					std::cerr << "COULD NOT CALL INITIALIZE FUNCTION FAIL!!!!!!!!!!!!!!!!\n\n";
					return;
				}
			} else {
				std::cerr << "INITIALIZE METHOD NOT FOUND FAIL!!!!!!!!!!!!!!!!\n\n";
				return;
			}
		}
		return;
	}

	bool outIsId = false;
	bool outNeedsAlloca = false;
	auto selection = this->_emitSelCh(selCh.get(), outIsId, outNeedsAlloca, parentIndex, thisObj);
	auto llvmTy = this->llvmTypeFromLILType(ty.get());
	if (!selection) {
		const auto & childRules = value->getChildRules();
		if (childRules.size() > 0) {
			for (auto child : childRules) {
				this->_connectRule(child.get(), parentId, parentIndex, thisObj);
			}
		}
		return;
	}
	if (outIsId) {
		if (outNeedsAlloca) {
			d->currentAlloca = d->irBuilder.CreateAlloca(llvmTy, 0, "");
			//hack: using the array index to get to the id field of super
			auto gep = this->_emitGEP(d->currentAlloca, llvmTy, true, 0, "id", true, true, 0);
			d->irBuilder.CreateStore(selection, gep);
			std::vector<llvm::Value *> argsvect;
			argsvect.push_back(d->currentAlloca);
			argsvect.push_back(parentIndex);
			d->irBuilder.CreateCall(ruleFn, argsvect);
			
			const auto & childRules = value->getChildRules();
			if (childRules.size() > 0) {
				auto i64Ty = llvm::Type::getInt64Ty(d->llvmContext);
				auto idValue = d->irBuilder.CreateLoad(i64Ty, gep);
				for (auto child : childRules) {
					this->_connectRule(child.get(), idValue, parentIndex, d->currentAlloca);
				}
			}
		} else {
			std::vector<llvm::Value *> argsvect;
			argsvect.push_back(selection);
			argsvect.push_back(parentIndex);
			d->irBuilder.CreateCall(ruleFn, argsvect);
			
			const auto & childRules = value->getChildRules();
			if (childRules.size() > 0) {
				auto i64Ty = llvm::Type::getInt64Ty(d->llvmContext);
				auto gep = this->_emitGEP(selection, llvmTy, true, 0, "id", true, true, 0);
				auto idValue = d->irBuilder.CreateLoad(i64Ty, gep);
				for (auto child : childRules) {
					this->_connectRule(child.get(), idValue, parentIndex, selection);
				}
			}
		}
	} else {
		auto loopBB = llvm::BasicBlock::Create(d->llvmContext, "sel_loop", applyFn);
		auto afterBB = llvm::BasicBlock::Create(d->llvmContext, "sel_loop.after");
		//var.i64 counter: 0;
		auto i64Ty = llvm::Type::getInt64Ty(d->llvmContext);
		auto counter = d->irBuilder.CreateAlloca(i64Ty, 0, "counter");
		auto zeroVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 0, true));
		d->irBuilder.CreateStore(zeroVal, counter);
		//counter<selection.size
		std::vector<unsigned int> indices;
		indices.push_back(1);
		auto selectionSize = d->irBuilder.CreateExtractValue(selection, indices);
		auto condition = d->irBuilder.CreateICmpSLT(zeroVal, selectionSize);

		d->irBuilder.CreateCondBr(condition, loopBB, afterBB);
		
		d->irBuilder.SetInsertPoint(loopBB);
		std::vector<llvm::Value *> idList;
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
		idList.push_back(d->irBuilder.CreateLoad(i64Ty, counter));
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
		auto selClassTy = d->classTypes["sel"];
		auto selAlloca = d->irBuilder.CreateAlloca(selClassTy);
		d->irBuilder.CreateStore(selection, selAlloca);
		auto currentIdGep = llvm::GetElementPtrInst::Create(selClassTy, selAlloca, idList, "currentId", d->irBuilder.GetInsertBlock());
		auto currentId = d->irBuilder.CreateLoad(i64Ty, currentIdGep);
		
		bool needsCast = tyName != "container";
		llvm::Type * containerTy;
		if (needsCast) {
			containerTy = d->classTypes.at("container");
		} else {
			containerTy = llvmTy;
		}
		d->currentAlloca = d->irBuilder.CreateAlloca(containerTy);
		//hack: using the array index to get to the id field of super
		auto gep = this->_emitGEP(d->currentAlloca, containerTy, true, 0, "id", true, true, 0);
		d->irBuilder.CreateStore(currentId, gep);
		std::vector<llvm::Value *> argsvect;
		if (needsCast) {
			d->currentAlloca = d->irBuilder.CreatePointerCast(d->currentAlloca, llvmTy->getPointerTo());
		}
		argsvect.push_back(d->currentAlloca);

		auto counterVal = d->irBuilder.CreateLoad(i64Ty, counter);
		argsvect.push_back(counterVal);

		d->irBuilder.CreateCall(ruleFn, argsvect);

		const auto & childRules = value->getChildRules();
		if (childRules.size() > 0) {
			auto i64Ty = llvm::Type::getInt64Ty(d->llvmContext);
			auto idValue = d->irBuilder.CreateLoad(i64Ty, gep);
			for (auto child : childRules) {
				this->_connectRule(child.get(), idValue, parentIndex, d->currentAlloca);
			}
		}

		//counter +: 1
		auto oneVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 1, true));
		auto increased = d->irBuilder.CreateAdd(counterVal, oneVal);
		d->irBuilder.CreateStore(increased, counter);
		
		auto condition2 = d->irBuilder.CreateICmpSLT(increased, selectionSize);
		applyFn->getBasicBlockList().push_back(afterBB);
		d->irBuilder.CreateCondBr(condition2, loopBB, afterBB);
		d->irBuilder.SetInsertPoint(afterBB);
	}
}

llvm::Value* LILIREmitter::_getContainerNameFromSelectorChain(std::shared_ptr<LILSelectorChain> selCh)
{
	llvm::Value * ret = nullptr;
	auto lastNode = selCh->getLastNode();
	if (lastNode->isA(NodeTypeSimpleSelector)) {
		auto simpleSel = std::static_pointer_cast<LILSimpleSelector>(lastNode);
		auto nameNode = simpleSel->getFirstNode();
		if (nameNode->isA(NodeTypeSelector)) {
			auto sel = std::static_pointer_cast<LILSelector>(nameNode);
			auto name = sel->getName();
			auto stringLit = std::make_shared<LILStringLiteral>();
			stringLit->setValue(name);
			stringLit->setIsCString(true);
			return this->_emitStr(stringLit.get());
		}
	}
	return ret;
}

LILString LILIREmitter::_newRuleFnName()
{
	LILString ret = "_lil_apply_rule"+LILString::number((LILUnitI32)d->ruleCount);
	d->ruleCount += 1;
	return ret;
}

llvm::Value * LILIREmitter::_emitSSel(LILSimpleSelector * value)
{
	std::cerr << "!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!\n";
	return nullptr;

}

llvm::Value * LILIREmitter::_emitSelCh(LILSelectorChain * value, bool & outIsId, bool & outNeedsAlloca, llvm::Value* parentIndex, llvm::Value * thisObj)
{
	if (value->getNodes().size() == 1) {
		auto sChNode = value->getNodes().front();
		if (sChNode->getNodeType() == NodeTypeSimpleSelector) {
			auto ss = std::static_pointer_cast<LILSimpleSelector>(sChNode);
			if (ss->getNodes().size() == 1) {
				auto firstSelNode = ss->getNodes().front();
				auto selNodeTy = firstSelNode->getNodeType();
				if (selNodeTy == NodeTypeSelector) {
					auto sel = std::static_pointer_cast<LILSelector>(firstSelNode);
					switch (sel->getSelectorType()) {
						case SelectorTypeRootSelector:
						{
							outIsId = true;
							outNeedsAlloca = true;
							std::vector<llvm::Constant *> elemMembers;
							elemMembers.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 0, true)));
							auto elemStruct = llvm::ConstantStruct::get(d->classTypes["element"], elemMembers);
							std::vector<llvm::Constant *> containerMembers;
							containerMembers.push_back(elemStruct);
							return llvm::ConstantStruct::get(d->classTypes["container"], containerMembers);
						}

						case SelectorTypeNameSelector:
						{
							for (const auto & elem : this->getDOM()->children) {
								if (elem->name == sel->getName()) {
									outIsId = true;
									outNeedsAlloca = true;
									std::vector<llvm::Constant *> elemMembers;
									elemMembers.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, elem->id, true)));
									auto elemStruct = llvm::ConstantStruct::get(d->classTypes["element"], elemMembers);
									std::vector<llvm::Constant *> containerMembers;
									containerMembers.push_back(elemStruct);
									return llvm::ConstantStruct::get(d->classTypes["container"], containerMembers);
								}
							}
							std::cerr << "!!!!COULD NOT FIND ELEMENT FAIL!!!!!!!!!!\n";
							return nullptr;
						}
						case SelectorTypeThisSelector:
						{
							outIsId = true;
							outNeedsAlloca = false;
							return thisObj;
						}
						default:
							std::cerr << "!!!!UNIMPLEMENTED FAIL!!!!!!!!!!\n";
							return nullptr;
							break;
					}
				} else if (selNodeTy == NodeTypeFlag) {
					auto flag = std::static_pointer_cast<LILFlag>(firstSelNode);
					if (flag->getIsOnByDefault()) {
						outIsId = true;
						outNeedsAlloca = false;
						return thisObj;
					}
					return nullptr;
				}
			}
		}
	} else {

		for (const auto & schNode : value->getNodes()) {
			if (schNode->getNodeType() != NodeTypeSimpleSelector) {
				std::cerr << "UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!!\n";
				return nullptr;
			}
			auto ss = std::static_pointer_cast<LILSimpleSelector>(schNode);
			for (const auto & ssNode : ss->getNodes()) {
				if (ssNode->getNodeType() != NodeTypeSelector) {
					std::cerr << "UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!!\n";
					return nullptr;
				}
				auto sel = std::static_pointer_cast<LILSelector>(ssNode);
				switch (ssNode->getSelectorType()) {
					case SelectorTypeRootSelector:
					{
						break;
					}
						
					case SelectorTypeNameSelector:
					{
						const auto & name = sel->getName();
						auto nameToIdFc = std::make_shared<LILFunctionCall>();
						LILString nameToIdFnName = "LIL__nameToNameId";
						nameToIdFc->setName(nameToIdFnName);
						auto strLit = std::make_shared<LILStringLiteral>();
						strLit->setIsCString(true);
						strLit->setValue(name);
						nameToIdFc->addArgument(strLit);
						
						
						auto nameToIdFnNode = this->findNodeForName(nameToIdFnName, value->getParentNode().get());
						if (!nameToIdFnNode || !nameToIdFnNode->isA(NodeTypeFunctionDecl)){
							std::cerr << "NAME TO ID FUNCTION NOT FOUND FAIL\n\n";
							break;
						}
						auto nameToIdFn = std::static_pointer_cast<LILFunctionDecl>(nameToIdFnNode);
						std::shared_ptr<LILFunctionType> fnTy = nameToIdFn->getFnType();
						auto nameId = this->_emitFunctionCall(nameToIdFc.get(), nameToIdFn->getName(), fnTy.get(), nullptr);

						LILString selectFnName = "LIL__selectByName";
						auto selectFnNode = this->findNodeForName(selectFnName, value->getParentNode().get());
						if (!selectFnNode || !selectFnNode->isA(NodeTypeFunctionDecl)){
							std::cerr << "SELECT BY NAME FUNCTION NOT FOUND FAIL\n\n";
							break;
						}
						auto selectFn = std::static_pointer_cast<LILFunctionDecl>(selectFnNode);
						auto selectFnTy = selectFn->getFnType();
						llvm::Function* selectFun = d->llvmModule.getFunction(selectFn->getName().data());
						std::vector<llvm::Value *> selectArgs;
						selectArgs.push_back(nameId);
						selectArgs.push_back(parentIndex);
						return d->irBuilder.CreateCall(selectFun, selectArgs, "selection");
					}
						
					default:
						break;
				}
			}
			
		}
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitSel(LILSelector * value)
{
	switch (value->getSelectorType()) {
		case SelectorTypeKey:
		{
			LILString name = value->getName();
			auto namestr = name.data();
			auto vn = std::make_shared<LILVarName>();
			vn->setName(namestr);
			vn->setParentNode(value->getParentNode());
			return this->_emitVN(vn.get());
		}
			
		case SelectorTypeValue:
		{
			LILString name = value->getName();
			auto namestr = name.data();
			auto forBlock = this->findAncestorFor(value->shared_from_this());
			auto subjectNode = forBlock->getSubject();
			auto subjTy = subjectNode->getType();
			if (!subjTy) {
				std::cerr << "SUBJECT HAD NO TYPE FAIL!!!!!!!!!!!!!!\n";
				return nullptr;
			}
			if (subjTy->getTypeType() == TypeTypeObject) {
				auto cd = this->findClassWithName(subjTy->getName());
				if (!cd) {
					std::cerr << "CLASS " + subjTy->getName().data() +  " NOT FOUND FAIL!!!!!!!!!!!!!!\n";
					return nullptr;
				}
				auto meth = cd->getMethodNamed("value");
				if (meth) {
					if (!meth->isA(NodeTypeFunctionDecl)) {
						std::cerr << "VALUE METHOD NODE WAS NOT FUNCTION DECL FAIL!!!\n\n";
						return nullptr;
					}
					auto fd = std::static_pointer_cast<LILFunctionDecl>(meth);
					llvm::Function* fun = d->llvmModule.getFunction(fd->getName().data());
					if (fun) {
						auto vn = std::make_shared<LILVarName>();
						vn->setName("@key");
						vn->setParentNode(value->getParentNode());
						auto keyVal = this->_emitVN(vn.get());
						
						std::vector<llvm::Value *> argsvect;
						argsvect.push_back(this->emitPointer(subjectNode.get()));
						argsvect.push_back(keyVal);
						return d->irBuilder.CreateCall(fun, argsvect);
					} else {
						std::cerr << "COULD NOT CALL VALUE METHOD FAIL!!!!\n\n";
						return nullptr;
					}
				}
				
				
			} else {
				auto vn = std::make_shared<LILVarName>();
				vn->setName(namestr);
				vn->setParentNode(value->getParentNode());
				return this->_emitVN(vn.get());
			}
			break;
		}
			
		case SelectorTypeSelfSelector:
		{
			return d->namedValues["@self"];
		}

		case SelectorTypeIndex:
		{
			const auto indexStr = "@index";
			if (d->namedValues.count(indexStr)) {
				LILString name = value->getName();
				auto vn = std::make_shared<LILVarName>();
				vn->setName(indexStr);
				vn->setParentNode(value->getParentNode());
				return this->_emitVN(vn.get());
			}
			const auto valueStr = "@value";
			if (d->namedValues.count(valueStr)) {
				auto ptr = d->namedValues[valueStr];
				return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(value->getType().get()), ptr, valueStr);
			}
			std::cerr << "COULD NOT EMIT @index SELECTOR FAIL!!!!!!!!!!!!!!\n";
			return nullptr;
		}
			
		case SelectorTypeI:
		{
			const auto iStr = "@i";
			if (d->namedValues.count(iStr)) {
				auto iPtr = d->namedValues[iStr];
				return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(value->getType().get()), iPtr, iStr);
			}
			const auto indexStr = "@index";
			if (d->namedValues.count(indexStr)) {
				auto indexPtr = d->namedValues[indexStr];
				return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(value->getType().get()), indexPtr, indexStr);
			}
			std::cerr << "COULD NOT EMIT @i SELECTOR FAIL!!!!!!!!!!!!!!\n";
			return nullptr;
		}
			
		case SelectorTypeJ:
		{
			const auto jStr = "@j";
			if (d->namedValues.count(jStr)) {
				auto jPtr = d->namedValues[jStr];
				return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(value->getType().get()), jPtr, jStr);
			}
			const auto indexStr = "@index";
			if (d->namedValues.count(indexStr)) {
				auto indexPtr = d->namedValues[indexStr];
				return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(value->getType().get()), indexPtr, indexStr);
			}
			std::cerr << "COULD NOT EMIT @j SELECTOR FAIL!!!!!!!!!!!!!!\n";
			return nullptr;
		}
			
		case SelectorTypeK:
		{
			const auto kStr = "@k";
			if (d->namedValues.count(kStr)) {
				auto kPtr = d->namedValues[kStr];
				return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(value->getType().get()), kPtr, kStr);
			}
			const auto indexStr = "@index";
			if (d->namedValues.count(indexStr)) {
				auto indexPtr = d->namedValues[indexStr];
				return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(value->getType().get()), indexPtr, indexStr);
			}
			std::cerr << "COULD NOT EMIT @k SELECTOR FAIL!!!!!!!!!!!!!!\n";
			return nullptr;
		}

		default:
			std::cerr << "!!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!\n";
			break;
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitComb(LILCombinator * value)
{
	std::cerr << "!!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!\n";
	return nullptr;
}

llvm::Value * LILIREmitter::_emitFlt(LILFilter * value)
{
	std::cerr << "!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!!\n";
	return nullptr;
}

llvm::Value * LILIREmitter::_emitFlag(LILFlag * value)
{
	std::cerr << "!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!!\n";
	return nullptr;
}

llvm::Value * LILIREmitter::_emitVN(LILVarName * value)
{
	auto remoteNode = recursiveFindNode(value->shared_from_this());
	if (!remoteNode) {
		std::cerr << "REMOTE NODE NOT FOUND FAIL!!!!\n";
		return nullptr;
	}
	if (remoteNode->isA(NodeTypeVarDecl)) {
		auto vd = std::static_pointer_cast<LILVarDecl>(remoteNode);
		if (vd->getIsConst()) {
			return this->emit(vd->getInitVal().get());
		}
	}
	LILString name = value->getName();
	auto namestr = name.data();
	llvm::Value * val = d->namedValues[namestr];
	if (!val) {
		std::cerr << "!!!!!!!!!!UNKNOWN VARIABLE " + namestr + " OMG ALKJDLFJA FAIL FAIL FAIL!!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	auto ty = remoteNode->getType();
	if (!this->inhibitSearchingForIfCastType && ty->isA(TypeTypeMultiple)) {
		auto ifCastTy = this->findIfCastTypeVN(value);
		if (ifCastTy) {
			return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(ifCastTy.get()), val, namestr);
		}
	}
	return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(ty.get()), val, namestr);
}

llvm::Function * LILIREmitter::_emitFnDecl(LILFunctionDecl * value)
{
	if (value->getIsExtern()) {
		return nullptr;
	}
	if (value->getHasMultipleImpls()) {
		for (auto impl : value->getImpls()) {
			this->_emitFnDecl(impl.get());
		}
		return nullptr;
	}
	return this->_emitFn(value);
}

llvm::Function * LILIREmitter::_emitFnSignature(std::string name, LILFunctionType * fnTy)
{
	auto ft = this->_emitLlvmFnType(fnTy);
	auto existingFun = d->llvmModule.getFunction(name);
	if (existingFun) {
		return existingFun;
	} else {
		llvm::Function * fun = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, &d->llvmModule);
		return fun;
	}
}

llvm::FunctionType * LILIREmitter::_emitLlvmFnType(LILFunctionType * fnTy)
{
	std::vector<llvm::Type*> types;
	auto arguments = fnTy->getArguments();
	for (auto & arg : arguments) {
		llvm::Type * llvmTy = nullptr;
		std::shared_ptr<LILType> ty;
		if (arg->isA(NodeTypeType)) {
			ty = std::static_pointer_cast<LILType>(arg);
		} else if (arg->isA(NodeTypeVarDecl)){
			ty = std::static_pointer_cast<LILVarDecl>(arg)->getType();
		}
		if (ty) {
			if (ty->getName() == "null") {
				continue;
			}
			llvmTy = this->llvmTypeFromLILType(ty.get());
		}
		if (llvmTy) {
			types.push_back(llvmTy);
		} else {
			std::cerr << "!!!!!!!!!!EMIT FN SIGNATURE FAIL!!!!!!!!!!!!!!!!\n";
		}
	}
	
	std::shared_ptr<LILType> retTy = fnTy->getReturnType();
	llvm::Type * returnType = nullptr;
	if (retTy) {
		returnType = this->llvmTypeFromLILType(retTy.get());
	}
	if (!returnType) {
		returnType = llvm::Type::getVoidTy(d->llvmContext);
	}
	return llvm::FunctionType::get(returnType, types, fnTy->getIsVariadic());
}

llvm::Function * LILIREmitter::_emitFn(LILFunctionDecl * value)
{
	auto fnTy = std::static_pointer_cast<LILFunctionType>(value->getType());
	auto arguments = fnTy->getArguments();
	std::string fnName = value->getName().data();
	llvm::Function * fun = this->_emitFnSignature(fnName, fnTy.get());
	d->namedValues[fnName] = fun;
	if (value->getIsExtern()) {
		return fun;
	}

	size_t argIndex = 0;
	for (auto & llvmArg : fun->args()) {
		auto arg = arguments[argIndex];
		if(!arg){
			std::cerr << "!!!!!!!!!!ARGUMENT WAS NULL FAIL!!!!!!!!!!!!!!!!\n";
			return nullptr;
		}

		if (arg->isA(NodeTypeVarDecl)) {
			std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(arg);
			llvmArg.setName(vd->getName().data());
		}

		++argIndex;
	}

	llvm::BasicBlock * bb = llvm::BasicBlock::Create(d->llvmContext, "entry", fun);
	d->irBuilder.SetInsertPoint(bb);


	std::map<std::string, llvm::Value *> scope;
	d->hiddenLocals.push_back(scope);

	for (unsigned int i = 0; i<fun->arg_size(); i+=1) {
		auto arg = fun->getArg(i);
		auto name = std::string(arg->getName());
		if (name == "@self") {
			d->namedValues[name] = arg;
			continue;
		}
		llvm::AllocaInst * alloca = this->createEntryBlockAlloca(fun, name, arg->getType());
		d->irBuilder.CreateStore(arg, alloca);
		if (d->namedValues.count(name)) {
			d->hiddenLocals.back()[name] = d->namedValues[name];
		}
		d->namedValues[name] = alloca;
	}

	this->_emitFnBody(fun, value);

	//clear args from local values
	for (llvm::Value & arg : fun->args()) {
		auto name = std::string(arg.getName());
		d->namedValues.erase(name);
	}
	//restore hidden locals
	const std::map<std::string, llvm::Value *> & hiddenLocals = d->hiddenLocals.back();
	for (auto it = hiddenLocals.begin(); it != hiddenLocals.end(); ++it) {
		d->namedValues[it->first] = it->second;
	}
	d->hiddenLocals.pop_back();
	return nullptr;
}

llvm::Function * LILIREmitter::_emitFnBody(llvm::Function * fun, LILFunctionDecl * value)
{
	auto body = value->getBody();

	auto ty = value->getReturnType();
	if (ty && ty->getName() != "null") {
		d->returnAlloca = d->irBuilder.CreateAlloca(fun->getReturnType(), 0, "return");
	}
	d->finallyBB = llvm::BasicBlock::Create(d->llvmContext, "finally");

	this->_emitEvaluables(body);

	fun->getBasicBlockList().push_back(d->finallyBB);
	d->irBuilder.CreateBr(d->finallyBB);
	d->irBuilder.SetInsertPoint(d->finallyBB);

	auto finally = value->getFinally();
	if (finally) {
		this->emit(finally.get());
	}
	for (auto it : value->getLocalVariables()) {
		auto localValue = it.second;
		if (localValue->isA(NodeTypeVarDecl)) {
			auto vd = std::static_pointer_cast<LILVarDecl>(localValue);
			auto ty = vd->getType();
			if (ty && ty->isA(TypeTypeObject)) {
				const auto & className = ty->getName();
				auto cd = this->findClassWithName(className);
				if (!cd) {
					std::cerr << "CLASS NOT FOUND FAIL!!!!\n\n";
					continue;
				}
				const auto & name = vd->getName();
				auto ir = d->namedValues[name.data()];
				this->_emitDestructors(ir, cd, name);
			}
		}
	}

	if (d->needsReturnValue) {
		llvm::Value * loadInstr = d->irBuilder.CreateLoad(fun->getReturnType(), d->returnAlloca);
		d->irBuilder.CreateRet(loadInstr);
	} else {
		d->irBuilder.CreateRetVoid();
	}

	d->returnAlloca = nullptr;
	d->needsReturnValue = false;
	d->finallyBB = nullptr;

	llvm::verifyFunction(*fun);
#ifdef LILIREMITTEROPTIMIZE
	d->functionPassManager->run(*fun);
#endif
	return fun;
}

void LILIREmitter::_emitDestructors(llvm::Value * ir, std::shared_ptr<LILClassDecl> cd, const LILString & name)
{
	size_t fieldIndex = 0;
	for (auto field : cd->getFields()) {
		auto fldTy = field->getType();
		if(
		   fldTy && fldTy->isA(TypeTypeObject)
		   && field->isA(NodeTypeVarDecl)
		) {
			auto vd = std::static_pointer_cast<LILVarDecl>(field);
			if (vd->getIsVVar()) {
				continue;
			}
			const auto & fldClassName = fldTy->getName();
			auto fldCd = this->findClassWithName(fldClassName);
			if (!fldCd) {
				std::cerr << "CLASS NOT FOUND FAIL!!!!\n\n";
				continue;
			}
			const auto & fldName = vd->getName();
			auto fldIr = this->_emitGEP(ir, cd->getName(), fieldIndex, fldName, true);
			this->_emitDestructors(fldIr, fldCd, fldName);
		}
		fieldIndex += 1;
	}
	
	auto dtor = cd->getMethodNamed("destruct");
	if (dtor) {
		if (!dtor->isA(NodeTypeFunctionDecl)) {
			std::cerr << "DESTRUCTOR NODE WAS NOT FUNCTION DECL FAIL!!!\n\n";
			return;
		}
		
		auto fd = std::static_pointer_cast<LILFunctionDecl>(dtor);
		auto fnName = fd->getName();
		auto fnTy = fd->getFnType();
		
		llvm::Function* fun = d->llvmModule.getFunction(fnName.data());
		if (!fun) {
			fun = this->_emitFnSignature(fnName.data(), fnTy.get());
		}
		if (fun) {
			std::vector<llvm::Value *> argsvect;
			argsvect.push_back(ir);
			d->irBuilder.CreateCall(fun, argsvect);
		} else {
			std::cerr << "COULD NOT CALL DESTRUCTOR FAIL!!!!\n\n";
			return;
		}
	}
}

llvm::Value * LILIREmitter::_emitEvaluables(const std::vector<std::shared_ptr<LILNode>> & nodes)
{
	for (auto & node : nodes) {
		bool breakAfter = false;
		if (node->isA(FlowControlCallTypeReturn)) {
			breakAfter = true;;
		}
		this->emit(node.get());
		if (breakAfter) {
			break;
		}
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitFCMultipleValues(std::vector<std::shared_ptr<LILFunctionDecl>> funcDecls, LILFunctionCall * value, llvm::Value * instance, std::shared_ptr<LILType> instanceTy)
{
	auto arguments = value->getArguments();
	std::vector<std::shared_ptr<LILType>> fcTypes;
	if (instance == nullptr) {
		fcTypes = value->getArgumentTypes();
	} else {
		fcTypes.push_back(instanceTy);
		const auto & argTys = value->getArgumentTypes();
		fcTypes.insert(fcTypes.end(), argTys.begin(), argTys.end());
	}
	llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
	size_t i = 0;
	bool needsRuntimeSelect = false;
	for (auto fcTy : fcTypes) {
		if (fcTy->isA(TypeTypeMultiple) || fcTy->getIsNullable()) {
			needsRuntimeSelect = true;
			break;
		}
		i += 1;
	}
	if (needsRuntimeSelect) {
		auto fcTy = fcTypes[i];
		if (fcTy->isA(TypeTypeMultiple)) {
			auto multiTy = std::static_pointer_cast<LILMultipleType>(fcTy);
			auto argument = arguments[i];
			std::string namestr;
			if (argument->isA(NodeTypeVarName)) {
				namestr = std::static_pointer_cast<LILVarName>(argument)->getName().data();
			} else {
				namestr = value->getName().data();
			}
			
			auto llvmIr = this->emitPointer(argument.get());
			
			std::vector<llvm::Value *> gepIndices1;
			gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
			gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
			auto argTy = this->llvmTypeFromLILType(argument->getType().get());
			auto gep = d->irBuilder.CreateGEP(argTy, llvmIr, gepIndices1);
			
			auto argVal = d->irBuilder.CreateLoad(argTy, gep, namestr + "_lil_type_index");
			
			llvm::BasicBlock * defaultBB = llvm::BasicBlock::Create(d->llvmContext, namestr+ ".null");
			llvm::SwitchInst * switchInstr = d->irBuilder.CreateSwitch(argVal, defaultBB);
			llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".merge");

			fun->getBasicBlockList().push_back(defaultBB);
			d->irBuilder.SetInsertPoint(defaultBB);
			
			auto mfcTys = multiTy->getTypes();
			if (multiTy->getIsNullable()){
				mfcTys.push_back(LILType::make("null"));
			}
			size_t j = 1;
			for (auto mfcTy : mfcTys) {
				llvm::BasicBlock * bb;
				if (mfcTy->getName() == "null") {
					bb = defaultBB;
				} else {
					bb = llvm::BasicBlock::Create(d->llvmContext, namestr+"."+mfcTy->getName().data(), fun);
					switchInstr->addCase(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, j, false)), bb);
				}
				
				std::vector<std::shared_ptr<LILType>> types;
				for (size_t k=0; k<fcTypes.size(); ++k) {
					if (k == i) {
						types.push_back(mfcTy);
					} else {
						types.push_back(fcTy);
					}
				}
				auto fd = this->chooseFnByType(funcDecls, types);
				d->irBuilder.SetInsertPoint(bb);
				this->_emitFunctionCallMT(value, fd->getName(), types, fd->getFnType().get(), instance);
				d->irBuilder.CreateBr(mergeBB);
				j += 1;
			}
			
			fun->getBasicBlockList().push_back(mergeBB);
			d->irBuilder.SetInsertPoint(mergeBB);
		}
		else if (fcTy->getIsNullable())
		{
			auto argument = arguments[i];
			std::string namestr;
			if (argument->isA(NodeTypeVarName)) {
				namestr = std::static_pointer_cast<LILVarName>(argument)->getName().data();
			} else {
				namestr = value->getName().data();
			}
			
			auto ir = this->emit(argument.get());
			llvm::Value * cond;
			bool isPtr = fcTy->isA(TypeTypePointer);
			if (isPtr) {
				auto ptrType = llvm::cast<llvm::PointerType>(this->llvmTypeFromLILType(fcTy.get()));
				auto zeroVal = llvm::ConstantPointerNull::get(ptrType);
				cond = d->irBuilder.CreateICmpNE(ir, zeroVal, namestr + ".not.null.cond");
			} else {
				auto zeroVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 0, true));
				cond = d->irBuilder.CreateICmpSGE(ir, zeroVal, namestr + ".not.null.cond");
			}

			llvm::BasicBlock * notNullBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".if.not.null", fun);
			llvm::BasicBlock * isNullBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".if.null");
			llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".if.end");
			d->irBuilder.CreateCondBr(cond, notNullBB, isNullBB);
			
			//when not null
			d->irBuilder.SetInsertPoint(notNullBB);
			auto currentTy = std::static_pointer_cast<LILPointerType>(fcTy);
			currentTy->setIsNullable(false);
			std::vector<std::shared_ptr<LILType>> types;
			for (size_t k=0; k<fcTypes.size(); ++k) {
				if (k == i) {
					types.push_back(currentTy);
				} else {
					types.push_back(fcTy);
				}
			}
			auto fd = this->chooseFnByType(funcDecls, types);
			currentTy->setIsNullable(true);
			this->_emitFunctionCall(value, fd->getName(), fd->getFnType().get(), instance);
			d->irBuilder.CreateBr(mergeBB);

			fun->getBasicBlockList().push_back(isNullBB);
			d->irBuilder.SetInsertPoint(isNullBB);
			
			//when null
			std::vector<std::shared_ptr<LILType>> types2;
			auto nullTy = LILType::make("null");
			for (size_t k=0; k<fcTypes.size(); ++k) {
				if (k == i) {
					types2.push_back(nullTy);
				} else {
					types2.push_back(fcTy);
				}
			}
			auto fd2 = this->chooseFnByType(funcDecls, types2);
			this->_emitFunctionCall(value, fd2->getName(), fd2->getFnType().get(), instance, true, i);
			d->irBuilder.CreateBr(mergeBB);
			
			fun->getBasicBlockList().push_back(mergeBB);
			d->irBuilder.SetInsertPoint(mergeBB);
		}
	} else {
		auto fd = this->chooseFnByType(funcDecls, fcTypes);
		if (!fd) {
			std::cerr << "COULD NOT CHOOSE FN BY TYPE FAIL\n\n";
			return nullptr;
		}
		return this->_emitFunctionCall(value, fd->getName(), fd->getFnType().get(), instance);
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitFC(LILFunctionCall * value)
{
	switch (value->getFunctionCallType()) {
		case FunctionCallTypeNone:
		{
			LILString name = value->getName();
			auto node = this->findNodeForName(name, value->getParentNode().get());
			if (
				!node
				|| !(node->isA(NodeTypeVarDecl) || node->isA(NodeTypeFunctionDecl))
			){
				std::cerr << "!!!!!!!!!!!!!!!!FUNCTION NOT FOUND FAIL\n\n";
				break;
			}
			std::shared_ptr<LILFunctionType> fnTy;
			if (node->isA(NodeTypeVarDecl)) {
				auto vd = std::static_pointer_cast<LILVarDecl>(node);
				auto ty = node->getType();
				if (!ty) {
					std::cerr << "!!!!!!!!!!!!!!!!NODE HAD NO TYPE FAIL\n\n";
					break;
				}
				if (ty->isA(TypeTypePointer)) {
					auto ptrTy = std::static_pointer_cast<LILPointerType>(ty);
					auto argTy = ptrTy->getArgument();
					if (argTy->isA(TypeTypeFunction)) {
						fnTy = std::static_pointer_cast<LILFunctionType>(argTy);
					}
				} else if (ty->isA(TypeTypeFunction)){
					fnTy = std::static_pointer_cast<LILFunctionType>(ty);
				}
				if (!fnTy) {
					std::cerr << "TYPE IS NOT FUNCTION TYPE FAIL !!!!!!!!!!!!!!!!\n\n";
					return nullptr;
				}
				auto initVal = vd->getInitVal();
				if (!initVal) {
					auto namestr = name.data();
					if (d->namedValues.count(namestr)) {
						llvm::Value * val = d->namedValues[namestr];
						val = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(ty.get()), val);
						return this->_emitFunctionCallPointer(val, value, fnTy.get(), nullptr);
					} else {
						std::cerr << "FUNCTION " + namestr + " NOT FOUND FAIL!!!!!\n\n";
						return nullptr;
					}
				}
				if (initVal->isA(NodeTypeFunctionDecl)) {
					auto fd = std::static_pointer_cast<LILFunctionDecl>(initVal);
					fnTy = fd->getFnType();
					if (!fnTy) {
						std::cerr << "!!!!!!!FUNCTION DECL HAD NO FN TYPE FAIL!!!\n\n";
						return nullptr;
					}
					
					if (fd->getHasMultipleImpls()) {
						std::vector<std::shared_ptr<LILFunctionDecl>> impls = fd->getImpls();
						return this->_emitFCMultipleValues(impls, value);
					} else {
						return this->_emitFunctionCall(value, value->getName(), fnTy.get(), nullptr);
					}
					
				} else {
					auto fnPtrTy = initVal->getType();
					if (fnPtrTy->isA(TypeTypePointer)) {
						fnPtrTy = std::static_pointer_cast<LILPointerType>(fnPtrTy)->getArgument();
					}
					if (!fnPtrTy->isA(TypeTypeFunction)) {
						std::cerr << "NODE DOESN'T POINT TO FUNCTION FAIL!!!\n\n";
						return nullptr;
					} else {
						fnTy = std::static_pointer_cast<LILFunctionType>(fnPtrTy);
					}
					auto llvmVal = this->emit(initVal.get());
					if (llvmVal) {
						return this->_emitFunctionCallPointer(llvmVal, value, fnTy.get(), nullptr);
					} else {
						std::cerr << "EMIT SUBJECT NODE OF FUNCTION CALL FAIL!!!\n\n";
						return nullptr;
					}
				}
			}
			else if (node->isA(NodeTypeFunctionDecl))
			{
				auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
				fnTy = fd->getFnType();
				if (!fnTy) {
					std::cerr << "!!!!!!!FUNCTION DECL HAD NO FN TYPE FAIL!!!\n\n";
					return nullptr;
				}
				
				if (fd->getHasMultipleImpls()) {
					std::vector<std::shared_ptr<LILFunctionDecl>> impls = fd->getImpls();
					return this->_emitFCMultipleValues(impls, value);
				} else {
					return this->_emitFunctionCall(value, value->getName(), fnTy.get(), nullptr);
				}

			} else {
				std::cerr << "UNKOWN NODE FAIL !!!!!!!!!!!!!!!!\n\n";
				return nullptr;
			}
			break;
		}
		case FunctionCallTypePointerTo:
		{
			auto firstArg = value->getArguments().front();
			return this->emitPointer(firstArg.get());
		}
		case FunctionCallTypeValueOf:
		{
			auto firstArg = value->getArguments().front();
			auto llvmValue = this->emit(firstArg.get());
			auto firstArgTy = firstArg->getType();
			if (firstArgTy->getTypeType() == TypeTypePointer) {
				firstArgTy = std::static_pointer_cast<LILPointerType>(firstArgTy)->getArgument();
			}
			return d->irBuilder.CreateLoad(this->llvmTypeFromLILType(firstArgTy.get()), llvmValue);
		}
		case FunctionCallTypeSizeOf:
		{
			auto firstArg = value->getArguments().front();
			llvm::Type * llvmTy;
			if (firstArg->isA(NodeTypeTypeDecl)) {
				auto td = std::static_pointer_cast<LILTypeDecl>(firstArg);
				llvmTy = this->llvmTypeFromLILType(td->getSrcType().get());
			} else {
				auto llvmVal = this->emit(firstArg.get());
				llvmTy = llvmVal->getType();
			}
			auto nullConst = llvm::Constant::getNullValue(llvmTy->getPointerTo());
			auto gep = this->_emitGEP(nullConst, llvmTy, false, 0, "", false, true, 1);
			auto numTy = LILType::make("i64");
			return d->irBuilder.CreatePointerCast(gep, this->llvmTypeFromLILType(numTy.get()));
		}
		case FunctionCallTypeSet:
		{
			auto args = value->getArguments();
			if (args.size() != 2) {
				std::cerr << "SET NEEDS TWO ARGUMENTS FAIL\n";
				return nullptr;
			}
			auto ptr = this->emit(args[0].get());
			auto val = this->emit(args[1].get());
			return d->irBuilder.CreateStore(val, ptr);
		}
		case FunctionCallTypeConversion:
		{
			LILString name = value->getName();
			auto conv = this->getRootNode()->getConversionNamed(name);
			
			auto fnTy = std::make_shared<LILFunctionType>();
			fnTy->setName("fn");
			for (auto arg : value->getArguments()) {
				fnTy->addArgument(arg);
			}
			fnTy->setReturnType(conv->getType());
			return this->_emitFunctionCall(value, "_lil_conversion_"+name, fnTy.get(), nullptr);
		}
		case FunctionCallTypeSel:
		{
			bool outIsId = false;
			bool outNeedsAlloca = false;
			const auto & args = value->getArguments();
			const auto & selChNode = args.front();
			if (selChNode->getNodeType() != NodeTypeSelectorChain) {
				std::cerr << "SEL CALL ARG IS NOT SEL CHAIN FAIL!!!!\n";
				return nullptr;
			}
			auto selCh = std::static_pointer_cast<LILSelectorChain>(selChNode);
			auto zeroLit = std::make_shared<LILNumberLiteral>();
			zeroLit->setValue("0");
			auto zeroTy = LILType::make("i64");
			zeroLit->setType(zeroTy);
			auto zeroLitVal = this->_emitNum(zeroLit.get());
			auto llvmIr = this->_emitSelCh(selCh.get(), outIsId, outNeedsAlloca, zeroLitVal, nullptr);
			if (args.size() > 1) {
				const auto & flagNode = args.back();
				if (flagNode->getNodeType() == NodeTypeFlag) {
					auto rule = this->findRuleForSelChAndFlag(selCh.get(), static_cast<LILFlag *>(flagNode.get()));
					if (!rule) {
						std::cerr << "RULE NOT FOUND FAIL!!!!!!!!!\n";
						return nullptr;
					}
					if (outIsId) {
						auto ruleFn = d->llvmModule.getFunction(rule->getFnName().data());
						assert(ruleFn && "Rule function not found.");
						auto llvmTy = this->llvmTypeFromLILType(rule->getType().get());
						d->currentAlloca = d->irBuilder.CreateAlloca(llvmTy, 0, "");
						//hack: using the array index to get to the id field of super
						auto gep = this->_emitGEP(d->currentAlloca, llvmTy, true, 0, "id", true, true, 0);
						d->irBuilder.CreateStore(llvmIr, gep);
						std::vector<llvm::Value *> argsvect;
						argsvect.push_back(d->currentAlloca);
						argsvect.push_back(zeroLitVal);
						return d->irBuilder.CreateCall(ruleFn, argsvect);
						
					} else {
						std::cerr << "UNIMPLEMENTED FAIL!!!!\n";
						return nullptr;
					}
				}
			} else {
				if (outIsId) {
					return llvmIr;
				} else {
					std::cerr << "UNIMPLEMENTED FAIL!!!!\n";
					return nullptr;
				}
			}
		}
		default:
		{
			std::cerr << "UNIMPLEMENTED FUNCTION CALL TYPE FAIL\n";
			break;
		}
	}

	return nullptr;

}

std::shared_ptr<LILRule> LILIREmitter::findRuleForSelChAndFlag(LILSelectorChain * selCh, LILFlag * flag) const
{
	const auto & rootNode = this->getRootNode();
	const auto & firstSimpleSel = selCh->getNodes().front();
	assert((firstSimpleSel->getNodeType() == NodeTypeSimpleSelector) && "Node was not simple selector");
	
	std::vector<std::shared_ptr<LILRule>> currentRules;
	const auto & firstSimpleSelNode = firstSimpleSel->getChildNodes().front();
	switch (firstSimpleSelNode->getSelectorType()) {
		case SelectorTypeRootSelector:
		{
			for (const auto & rule : rootNode->getRules()) {
				auto firstSel = rule->getFirstSelector();
				if (firstSel->getSelectorType() == SelectorTypeRootSelector) {
					for (const auto & childRule : rule->getChildRules()) {
						currentRules.push_back(childRule);
					}
				}
			}
			break;
		}
		case SelectorTypeNameSelector:
		{
			auto selector = std::static_pointer_cast<LILSelector>(firstSimpleSelNode);
			for (const auto & rule : rootNode->getRules()) {
				auto firstSel = rule->getFirstSelector();
				if (firstSel->getSelectorType() == SelectorTypeRootSelector) {
					for (const auto & childRule : rule->getChildRules()) {
						const auto & childFirstSel = childRule->getFirstSelector();
						if (childFirstSel && childFirstSel->getSelectorType() == SelectorTypeNameSelector) {
							auto childSelector = std::static_pointer_cast<LILSelector>(childFirstSel);
							if (childSelector->getName() == selector->getName()) {
								if (selCh->getNodes().size() == 1) {
									for (const auto & selChildRule : childRule->getChildRules()) {
										const auto & selectorChain = selChildRule->getSelectorChain();
										const auto & firstSimpleSelNode = selectorChain->getChildNodes().front();
										assert((firstSimpleSelNode->getNodeType() == NodeTypeSimpleSelector) && "Node was not simple selector");
										const auto & simpleSelNodes = firstSimpleSelNode->getChildNodes();
										const auto & firstSelNode = firstSimpleSelNode->getChildNodes().front();
										
										if (firstSelNode->getSelectorType() == SelectorTypeThisSelector) {
											if (simpleSelNodes.size() > 1) {
												const auto & secondSelNode = simpleSelNodes.at(1);
												if (secondSelNode->getNodeType() == NodeTypeFlag) {
													auto ruleFlag = std::static_pointer_cast<LILFlag>(secondSelNode);
													if (ruleFlag->getName() == flag->getName()) {
														return selChildRule;
													}
												}
											} else {
												continue;
											}
										} else if (firstSelNode->getNodeType() == NodeTypeFlag) {
											auto ruleFlag = std::static_pointer_cast<LILFlag>(firstSelNode);
											if (ruleFlag->getName() == flag->getName()) {
												return selChildRule;
											}
										}
									}
								}
								currentRules.push_back(childRule);
							}
						}
					}
				}
			}
		}
		default:
			std::cerr << "UNKNOWN SELECTOR TYPE TYPE FAIL\n";
			break;
	}
	
	for (long int i = 1, j = selCh->getNodes().size(); i<j; i += 1) {
		std::vector<std::shared_ptr<LILRule>> tmpRules;
		const auto & selNode = selCh->getNodes().at(i);
		assert((selNode->getNodeType() == NodeTypeSimpleSelector) && "Node was not simple selector");
		
		const auto & simpleSelNode = selNode->getChildNodes().front();
		switch (simpleSelNode->getSelectorType()) {
			case SelectorTypeNameSelector:
			{
				auto selector = std::static_pointer_cast<LILSelector>(simpleSelNode);
				for (const auto & rule : currentRules) {
					auto firstSel = rule->getFirstSelector();
					if (firstSel->getSelectorType() == SelectorTypeRootSelector) {
						for (const auto & childRule : rule->getChildRules()) {
							const auto & instr = childRule->getInstruction();
							if (instr && instr->getInstructionType() == InstructionTypeNew) {
								const auto & childFirstSel = childRule->getFirstSelector();
								if (childFirstSel && childFirstSel->getSelectorType() == SelectorTypeNameSelector) {
									auto childSelector = std::static_pointer_cast<LILSelector>(childFirstSel);
									if (childSelector->getName() == selector->getName()) {
										tmpRules.push_back(childRule);
									}
								}
							}
						}
					}
				}
				currentRules = tmpRules;
				break;
			}
			default:
				std::cerr << "UNKNOWN SELECTOR TYPE TYPE FAIL\n";
				break;
		}
	}
	for (const auto & selectedRule : currentRules) {
		for (const auto & selChildRule : selectedRule->getChildRules()) {
			const auto & selectorChain = selChildRule->getSelectorChain();
			const auto & firstSimpleSelNode = selectorChain->getChildNodes().front();
			assert((firstSimpleSelNode->getNodeType() == NodeTypeSimpleSelector) && "Node was not simple selector");
			const auto & simpleSelNodes = firstSimpleSelNode->getChildNodes();
			const auto & firstSelNode = firstSimpleSelNode->getChildNodes().front();
			
			if (firstSelNode->getSelectorType() == SelectorTypeThisSelector) {
				if (simpleSelNodes.size() > 1) {
					const auto & secondSelNode = simpleSelNodes.at(1);
					if (secondSelNode->getNodeType() == NodeTypeFlag) {
						auto ruleFlag = std::static_pointer_cast<LILFlag>(secondSelNode);
						if (ruleFlag->getName() == flag->getName()) {
							return selChildRule;
						}
					}
				} else {
					continue;
				}
			} else if (firstSelNode->getNodeType() == NodeTypeFlag) {
				auto ruleFlag = std::static_pointer_cast<LILFlag>(firstSelNode);
				if (ruleFlag->getName() == flag->getName()) {
					return selChildRule;
				}
			}
		}
	}
	
	return nullptr;
}

llvm::Value * LILIREmitter::_emitFunctionCall(LILFunctionCall * value, LILString name, LILFunctionType * fnTy, llvm::Value * instance, bool skipArgument, size_t skipArgIndex)
{
	bool isMethod = instance != nullptr;
	llvm::Function* fun = d->llvmModule.getFunction(name.data());
	if (!fun) {
		fun = this->_emitFnSignature(name.data(), fnTy);
	}
	auto fcArgs = value->getArguments();
	if (fun) {
		std::vector<llvm::Value *> argsvect;

		auto declArgs = fnTy->getArguments();
		auto fcArgsSize = fcArgs.size();
		auto declArgsSize = declArgs.size();
		
		
		if (isMethod){
			argsvect.push_back(instance);
			declArgsSize -= 1;
		}
		
		size_t j = fcArgsSize > declArgsSize ? fcArgsSize : declArgsSize;
		for (size_t i = 0; i<j; ++i) {
			size_t declIndex = i;
			if (isMethod) {
				declIndex += 1;
			}
			if (skipArgument && declIndex == skipArgIndex) {
				continue;
			}
			std::shared_ptr<LILNode> fcArg;
			if (fcArgsSize <= i) {
				auto vdNode = declArgs[declIndex];
				if (!vdNode->isA(NodeTypeVarDecl)) {
					std::cerr << "DECL ARG IS NOT VAR DECL FAIL!!!!!!!!\n\n";
					return nullptr;
				}
				auto vd = std::static_pointer_cast<LILVarDecl>(vdNode);
				fcArg = vd->getInitVal();
			} else {
				fcArg = fcArgs[i];
			}
			
			llvm::Value * fcArgIr;
			std::shared_ptr<LILNode> fcValue;
			if (fcArg->isA(NodeTypeAssignment)) {
				auto asgmt = std::static_pointer_cast<LILAssignment>(fcArg);
				fcValue = asgmt->getValue();
			} else {
				fcValue = fcArg;
			}

			if (declArgsSize <= i)
			{
				auto fcArgTy = fcArg->getType().get();
				fcArgIr = this->_emitFCArg(fcValue.get(), fcArgTy);
				if (fnTy->getIsVariadic()) {
					auto fcArgTyName = fcArgTy->getName();
					//type promotions for var args
					if (fcArgTyName == "f32") {
						fcArgIr = d->irBuilder.CreateFPExt(fcArgIr, llvm::Type::getDoubleTy(d->llvmContext));
					} else if (fcArgTyName == "i8" || fcArgTyName == "i16") {
						fcArgIr = d->irBuilder.CreateSExt(fcArgIr, llvm::Type::getInt32Ty(d->llvmContext));
					}
				}
			}
			else
			{
				auto declArg = declArgs[declIndex];
				std::shared_ptr<LILType> declArgTy;
				if (declArg->isA(NodeTypeType)) {
					declArgTy = std::static_pointer_cast<LILType>(declArg);
				} else {
					declArgTy = declArg->getType();
				}
				fcArgIr = this->_emitFCArg(fcValue.get(), declArgTy.get());
			}

			if (!fcArgIr) {
				std::cerr << "!!!!!!!!!!ARG CODEGEN FAIL!!!!!!!!!!!!!!!\n";
				return nullptr;
			} else {
				argsvect.push_back(fcArgIr);
			}
		}

		if(fun->getReturnType()->getTypeID() != llvm::Type::VoidTyID){
			LILString callName = name+"_return";
			return d->irBuilder.CreateCall(fun, argsvect, callName.data());
		} else {
			return d->irBuilder.CreateCall(fun, argsvect);
		}
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitFCArg(LILNode * value, LILType * ty)
{
	llvm::Value * ret = nullptr;
	bool needsTempVar = this->_needsTemporaryVariable(value);
	auto allocaBackup = d->currentAlloca;
	bool usesPointerConversion =
		ty
		&& ty->isA(TypeTypePointer)
		&& !value->getType()->isA(TypeTypePointer)
	;
	llvm::Type * allocaTy = nullptr;
	if (needsTempVar) {
		if (usesPointerConversion) {
			auto ptrTy = static_cast<LILPointerType *>(ty);
			allocaTy = this->llvmTypeFromLILType(ptrTy->getArgument().get());
		} else {
			allocaTy = this->llvmTypeFromLILType(ty);
		}
		d->currentAlloca = d->irBuilder.CreateAlloca(allocaTy);
	}

	if (
		ty
		&& ty->getIsNullable()
		) {
		ret = this->emitNullable(value, ty);
	} else {
		if ( usesPointerConversion ) {
			ret = this->emitPointer(value);
		} else {
			ret = this->emit(value);
		}
	}
	if (ret) {
		auto ty2 = value->getType();
		if (ty && ty2) {
			this->_convertLlvmValueIfNeeded(&ret, ty, ty2.get());
		}
	}
	if (needsTempVar) {
		if (usesPointerConversion) {
			ret = d->currentAlloca;
		} else {
			ret = d->irBuilder.CreateLoad(allocaTy, d->currentAlloca);
		}
	}
	if (
		ty
		&& !ty->isA(TypeTypePointer)
		&& value->getType()->isA(TypeTypePointer)
		) {
		ret = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(ty), ret);
	}
	if (needsTempVar) {
		d->currentAlloca = allocaBackup;
	}
	return ret;
}

llvm::Value * LILIREmitter::_emitFunctionCallMT(LILFunctionCall *value, LILString name, std::vector<std::shared_ptr<LILType> > types, LILFunctionType * fnTy, llvm::Value * instance)
{
	llvm::Function* fun = d->llvmModule.getFunction(name.data());
	auto fcArgs = value->getArguments();
	if (!fun) {
		fun = this->_emitFnSignature(name.data(), fnTy);
	}
	if (fun) {
		std::vector<llvm::Value *> argsvect;
		
		if (instance != nullptr){
			argsvect.push_back(instance);
		}
		
		auto declArgs = fnTy->getArguments();
		auto fcArgsSize = fcArgs.size();
		auto declArgsSize = declArgs.size();
		size_t j = fcArgsSize > declArgsSize ? fcArgsSize : declArgsSize;
		for (size_t i = 0; i<j; ++i) {
			std::shared_ptr<LILNode> fcArg;
			if (fcArgsSize <= i) {
				auto vdNode = declArgs[i];;
				if (!vdNode->isA(NodeTypeVarDecl)) {
					std::cerr << "DECL ARG IS NOT VAR DECL FAIL!!!!!!!!\n\n";
					return nullptr;
				}
				auto vd = std::static_pointer_cast<LILVarDecl>(vdNode);
				fcArg = vd->getInitVal();
			} else {
				fcArg = fcArgs[i];
			}
			
			llvm::Value * fcArgIr;
			std::shared_ptr<LILNode> fcValue;
			
			if (declArgsSize <= i)
			{
				fcArgIr = this->emitUnwrappedFromMT(fcArg.get(), types[i].get());
			}
			else
			{
				auto declArg = declArgs[i];
				auto declArgTy = declArg->getType();
				if (declArgTy->getName() == "null") {
					continue;
				}
				
				if (fcArg->isA(NodeTypeAssignment)) {
					auto asgmt = std::static_pointer_cast<LILAssignment>(fcArg);
					fcValue = asgmt->getValue();
				} else {
					fcValue = fcArg;
				}
				if (
					declArgTy
					&& declArgTy->getIsNullable()
					) {
					fcArgIr = this->emitNullable(fcValue.get(), declArgTy.get());
				} else {
					if (
						declArgTy
						&& declArgTy->isA(TypeTypePointer)
						&& !types[i]->isA(TypeTypePointer)
						) {
						//fcArgIr = this->emitPointer(fcValue.get());
						std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
						return nullptr;
					} else {
						fcArgIr = this->emitUnwrappedFromMT(fcArg.get(), types[i].get());
					}
				}
				if (
					declArgTy
					&& !declArgTy->isA(TypeTypePointer)
					&& types[i]->isA(TypeTypePointer)
					) {
					std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
					return nullptr;
//					fcArgIr = d->irBuilder.CreateLoad(fcArgIr);
				}
			}
			
			if (!fcArgIr) {
				std::cerr << "!!!!!!!!!!ARG CODEGEN FAIL!!!!!!!!!!!!!!!!\n";
				return nullptr;
			} else {
				argsvect.push_back(fcArgIr);
			}
		}
		
		if(fun->getReturnType()->getTypeID() != llvm::Type::VoidTyID){
			LILString callName = name+"_return";
			return d->irBuilder.CreateCall(fun, argsvect, callName.data());
		} else {
			return d->irBuilder.CreateCall(fun, argsvect);
		}
	}
	return nullptr;
}
llvm::Value * LILIREmitter::_emitFunctionCallPointer(llvm::Value * fun, LILFunctionCall * value, LILFunctionType * fnTy, llvm::Value * instance)
{
	bool isMethod = instance != nullptr;
	auto fcArgs = value->getArguments();
	
	std::vector<llvm::Value *> argsvect;
	
	auto declArgs = fnTy->getArguments();
	auto fcArgsSize = fcArgs.size();
	auto declArgsSize = declArgs.size();
	
	
	if (isMethod){
		argsvect.push_back(instance);
		declArgsSize -= 1;
	}

	size_t j = fcArgsSize > declArgsSize ? fcArgsSize : declArgsSize;
	for (size_t i = 0; i<j; ++i) {
		size_t declIndex = i;
		if (isMethod) {
			declIndex += 1;
		}
		std::shared_ptr<LILNode> fcArg;
		if (fcArgsSize <= i) {
			auto vdNode = declArgs[declIndex];
			if (!vdNode->isA(NodeTypeVarDecl)) {
				std::cerr << "DECL ARG IS NOT VAR DECL FAIL!!!!!!!!\n\n";
				return nullptr;
			}
			auto vd = std::static_pointer_cast<LILVarDecl>(vdNode);
			fcArg = vd->getInitVal();
		} else {
			fcArg = fcArgs[i];
		}
		
		llvm::Value * fcArgIr;
		std::shared_ptr<LILNode> fcValue;
		if (fcArg->isA(NodeTypeAssignment)) {
			auto asgmt = std::static_pointer_cast<LILAssignment>(fcArg);
			fcValue = asgmt->getValue();
		} else {
			fcValue = fcArg;
		}
		
		if (declArgsSize <= i)
		{
			fcArgIr = this->_emitFCArg(fcValue.get(), fcArg->getType().get());
		}
		else
		{
			auto declArg = declArgs[declIndex];
			std::shared_ptr<LILType> declArgTy;
			if (declArg->isA(NodeTypeType)) {
				declArgTy = std::static_pointer_cast<LILType>(declArg);
			} else {
				declArgTy = declArg->getType();
			}
			fcArgIr = this->_emitFCArg(fcValue.get(), declArgTy.get());
		}
		
		if (!fcArgIr) {
			std::cerr << "!!!!!!!!!!ARG CODEGEN FAIL!!!!!!!!!!!!!!!\n";
			return nullptr;
		} else {
			argsvect.push_back(fcArgIr);
		}
	}
	
	auto llvmTy = this->llvmTypeFromLILType(fnTy);
	if (llvmTy->getTypeID() != llvm::Type::FunctionTyID) {
		std::cerr << "TYPE OF CALL WAS NOT FUNCTION TYPE FAIL!!!!!!!!!!!!!!!\n";
		return nullptr;
	}
	return d->irBuilder.CreateCall(static_cast<llvm::FunctionType *>(llvmTy), fun, argsvect);
}

llvm::Value * LILIREmitter::_emitFlowC(LILFlowControl * value)
{
	switch (value->getFlowControlType()) {
		case FlowControlTypeIf:
		{
			return this->_emitIf(value);
		}
		case FlowControlTypeIfCast:
		{
			return this->_emitIfCast(value);
		}
		case FlowControlTypeFinally:
		{
			for (auto node : value->getThen()) {
				this->emit(node.get());
			}
			return nullptr;
		}
		case FlowControlTypeFor:
		{
			return this->_emitFor(value);
		}
		case FlowControlTypeLoop:
		{
			return this->_emitLoop(value);
		}
		default:
		{
			std::cerr << "UNKNOWN FLOW CONTROL TYPE FAIL!!!!!!!!!!!!!!!!\n\n";
			return nullptr;
		}
	}

	return nullptr;
}

llvm::Value * LILIREmitter::_emitIf(LILFlowControl * value)
{
	const auto & args = value->getArguments();
	if (args.size() == 0) {
		this->_emitEvaluables(value->getThen());
		return nullptr;
	}
	const auto & firstArg = args.front();
	llvm::Value * condition = this->emit(firstArg.get());
	if (!firstArg->isA(NodeTypeExpression)) {
		switch (condition->getType()->getTypeID()) {
			case llvm::Type::IntegerTyID:
			{
				auto bitWidth = condition->getType()->getIntegerBitWidth();
				if (bitWidth == 1){
					condition = d->irBuilder.CreateICmpNE(condition, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(bitWidth, 0, true)), "if.cond");
				} else {
					condition = d->irBuilder.CreateICmpSGT(condition, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(bitWidth, 0, true)), "if.cond");
				}
			}
				break;
			case llvm::Type::FloatTyID:
				condition = d->irBuilder.CreateFCmpONE(condition, llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(0.0)), "if.cond");
				break;

			default:
				break;
		}
	}

	llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
	llvm::BasicBlock * bodyBB = llvm::BasicBlock::Create(d->llvmContext, "if.true", fun);
	llvm::BasicBlock * elseBB = llvm::BasicBlock::Create(d->llvmContext, "if.false");
	llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, "if.end");

	d->irBuilder.CreateCondBr(condition, bodyBB, elseBB);

	//configure the body of the if
	d->irBuilder.SetInsertPoint(bodyBB);

	bool hasReturn = false;
	bool hasBreak = false;
	for (auto & node : value->getThen()) {
		if (node->isA(FlowControlCallTypeReturn)) {
			hasReturn = true;
		}
		if (node->isA(FlowControlCallTypeBreak)) {
			hasBreak = true;
		}
		this->emit(node.get());
		if (hasReturn || hasBreak) {
			break;
		}
	}
	if (hasReturn) {
		d->irBuilder.CreateBr(d->finallyBB);
	} else if (!hasBreak) {
		d->irBuilder.CreateBr(mergeBB);
	}

	fun->getBasicBlockList().push_back(elseBB);
	d->irBuilder.SetInsertPoint(elseBB);

	hasReturn = false;
	hasBreak = false;
	for (auto & node : value->getElse()) {
		if (node->isA(FlowControlCallTypeReturn)) {
			hasReturn = true;
		}
		if (node->isA(FlowControlCallTypeBreak)) {
			hasBreak = true;
		}
		this->emit(node.get());
		if (hasReturn || hasBreak) {
			break;
		}
	}
	if (hasReturn) {
		d->irBuilder.CreateBr(d->finallyBB);
	} else if (!hasBreak) {
		d->irBuilder.CreateBr(mergeBB);
	}

	fun->getBasicBlockList().push_back(mergeBB);
	d->irBuilder.SetInsertPoint(mergeBB);
	return nullptr;
}

llvm::Value * LILIREmitter::_emitIfCast(LILFlowControl * value)
{
	const auto & args = value->getArguments();
	if (args.size() == 0) {
		this->_emitEvaluables(value->getThen());
		return nullptr;
	}
	
	llvm::Value * condition;
	
	const auto & firstArg = args.front();
	const auto & lastArg = args.back();
	if (!lastArg->isA(NodeTypeType)) {
		std::cerr << "LAST ARG NOT TYPE FAIL!!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	
	this->inhibitSearchingForIfCastType = true;
	std::shared_ptr<LILType> ty;
	std::shared_ptr<LILVarDecl> subject;
	if (firstArg->isA(NodeTypeVarName)) {
		auto vn = std::static_pointer_cast<LILVarName>(firstArg);
		ty = vn->getType();
		auto subjNode = this->findNodeForVarName(vn.get());
		if (!subjNode->isA(NodeTypeVarDecl)) {
			std::cerr << "SUBJECT NODE WAS NOT VAR DECL FAIL!!!!!!!!!!!!!!!!\n\n";
			return nullptr;
		}
		subject = std::static_pointer_cast<LILVarDecl>(subjNode);
	} else if (firstArg->isA(NodeTypeValuePath)){
		auto vp = std::static_pointer_cast<LILValuePath>(firstArg);
		ty = vp->getType();
	} else {
		std::cerr << "UNKNOWN FIRST ARG FAIL!!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	if (!ty) {
		std::cerr << "TYPE OF ARGUMENT TO IF CAST WAS NULL!!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	
	auto lastArgTy = std::static_pointer_cast<LILType>(lastArg);
	bool isMultiple = ty->isA(TypeTypeMultiple);

	if (isMultiple)
	{
		condition = this->_emitIfCastConditionForMT(false, lastArgTy.get(), static_cast<LILMultipleType *>(ty.get()), firstArg.get());
	}
	else if (ty->getIsNullable())
	{
		if (lastArgTy->getName() == "null")
		{
			condition = this->_emitIfCastConditionForNullable(true, ty.get(), firstArg.get());
		} else {
			condition = this->_emitIfCastConditionForNullable(false, lastArgTy.get(), firstArg.get());
		}
	} else {
		std::cerr << "!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}

	llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
	llvm::BasicBlock * bodyBB = llvm::BasicBlock::Create(d->llvmContext, "if.true", fun);
	llvm::BasicBlock * elseBB = llvm::BasicBlock::Create(d->llvmContext, "if.false");
	llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, "if.end");

	d->irBuilder.CreateCondBr(condition, bodyBB, elseBB);

	this->inhibitSearchingForIfCastType = false;

	//configure the body of the if
	d->irBuilder.SetInsertPoint(bodyBB);
	
	bool hasReturn = false;
	for (auto & node : value->getThen()) {
		if (node->isA(FlowControlCallTypeReturn)) {
			hasReturn = true;
		}
		this->emit(node.get());
		if (hasReturn) {
			break;
		}
	}
	if (hasReturn) {
		d->irBuilder.CreateBr(d->finallyBB);
	} else {
		d->irBuilder.CreateBr(mergeBB);
	}

	fun->getBasicBlockList().push_back(elseBB);
	d->irBuilder.SetInsertPoint(elseBB);
	
	hasReturn = false;
	for (auto & node : value->getElse()) {
		if (node->isA(FlowControlCallTypeReturn)) {
			hasReturn = true;;
		}
		this->emit(node.get());
		if (hasReturn) {
			break;
		}
	}
	if (hasReturn) {
		d->irBuilder.CreateBr(d->finallyBB);
	} else {
		d->irBuilder.CreateBr(mergeBB);
	}

	fun->getBasicBlockList().push_back(mergeBB);
	d->irBuilder.SetInsertPoint(mergeBB);
	return nullptr;
}

llvm::Value * LILIREmitter::_emitIfCastConditionForMT(bool negated, LILType * ty, LILMultipleType * multiTy, LILNode * val)
{
	size_t theIndex = multiTy->indexOfType(ty);
	auto typeIndex = llvm::ConstantInt::get(
		d->llvmContext,
		llvm::APInt(
			8,
			theIndex,
			false
		)
	);
	
	auto llvmIr = this->emitPointer(val);
	
	std::vector<llvm::Value *> gepIndices2;
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
	auto i8Ty = llvm::Type::getInt8Ty(d->llvmContext);
	auto member2 = d->irBuilder.CreateGEP(this->llvmTypeFromLILType(multiTy), llvmIr, gepIndices2);
	auto member2Value = d->irBuilder.CreateLoad(i8Ty, member2);
	if (negated) {
		return d->irBuilder.CreateICmpNE(typeIndex, member2Value);
	} else {
		return d->irBuilder.CreateICmpEQ(typeIndex, member2Value);
	}
}

llvm::Value * LILIREmitter::_emitIfCastConditionForNullable(bool negated, LILType * ty, LILNode * val)
{
	llvm::Value * ret = nullptr;
	if (ty->getName() == "bool")
	{
		llvm::Value * ir = this->emit(val);
		auto upperBitValue = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 2, false));
		if (negated) {
			ret = d->irBuilder.CreateICmpUGT(ir, upperBitValue, "if.cond");
		} else {
			ret = d->irBuilder.CreateICmpULT(ir, upperBitValue, "if.cond");
		}
	}
	else if (ty->isA(TypeTypePointer))
	{
		llvm::Value * ir = this->emit(val);
		auto temp = d->irBuilder.CreatePtrToInt(ir, llvm::Type::getInt64Ty(d->llvmContext));
		auto nullValue = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 0, false));
		if (negated) {
			ret = d->irBuilder.CreateICmpEQ(temp, nullValue, "if.cond");
		} else {
			ret = d->irBuilder.CreateICmpNE(temp, nullValue, "if.cond");
		}
	}
	else if (
		ty->getName() == "i8"
		|| ty->getName() == "i16"
		|| ty->getName() == "i32"
		|| ty->getName() == "i64"
		|| ty->getName() == "i128"
		|| ty->getName() == "f32"
		|| ty->getName() == "f64"
		|| ty->isA(TypeTypeObject)
		|| ty->getName() == "i8%"
		|| ty->getName() == "i16%"
		|| ty->getName() == "i32%"
		|| ty->getName() == "i64%"
		|| ty->getName() == "i128%"
		|| ty->getName() == "f32%"
		|| ty->getName() == "f64%"
	) {
		auto llvmSubject = this->emitPointer(val);
		std::vector<llvm::Value *> idList;
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
		idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
		auto llvmTy = this->llvmTypeFromLILType(ty);
		auto temp = llvm::GetElementPtrInst::Create(llvmTy, llvmSubject, idList, "hasValue", d->irBuilder.GetInsertBlock());
		auto temp2 = d->irBuilder.CreateLoad(llvmTy, temp);
		auto nullValue = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(1, 0, false));
		if (negated) {
			ret = d->irBuilder.CreateICmpEQ(temp2, nullValue, "if.cond");
		} else {
			ret = d->irBuilder.CreateICmpNE(temp2, nullValue, "if.cond");
		}
	}
	else
	{
		std::cerr << "UNKNOWN TYPE FOR IF CAST FAIL!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	return ret;
}

llvm::Value * LILIREmitter::_emitFor(LILFlowControl * value)
{
	auto arguments = value->getArguments();
	if (arguments.size() > 1) {
		std::map<std::string, llvm::Value *> scope;
		d->hiddenLocals.push_back(scope);
		
		auto initial = arguments.front();
		this->emit(initial.get());
		
		auto condNode = arguments[1];
		if (!condNode) {
			return nullptr;
		}
		
		auto currentBB = d->irBuilder.GetInsertBlock();
		auto fun = currentBB->getParent();
		auto loopBB = llvm::BasicBlock::Create(d->llvmContext, "loop", fun);
		auto afterLoopBBBackup = d->afterLoopBB;
		d->afterLoopBB = llvm::BasicBlock::Create(d->llvmContext, "for.after", fun);
		
		auto condition = this->emit(condNode.get());
		d->irBuilder.CreateCondBr(condition, loopBB, d->afterLoopBB);
		
		d->irBuilder.SetInsertPoint(loopBB);
		this->_emitEvaluables(value->getThen());
		
		if (arguments.size() == 3) {
			auto stepNode = arguments[2];
			if (stepNode) {
				this->emit(stepNode.get());
			}
		}

		auto condition2 = this->emit(condNode.get());
		if (!condNode->isA(NodeTypeExpression)) {
			switch (condition2->getType()->getTypeID()) {
				case llvm::Type::IntegerTyID:
				{
					auto bitWidth = condition2->getType()->getIntegerBitWidth();
					if (bitWidth == 1){
						condition2 = d->irBuilder.CreateICmpNE(condition2, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(bitWidth, 0, true)), "for.cond");
					} else {
						condition2 = d->irBuilder.CreateICmpSGT(condition2, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(bitWidth, 0, true)), "for.cond");
					}
				}
					break;
				case llvm::Type::FloatTyID:
					condition2 = d->irBuilder.CreateFCmpONE(condition2, llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(0.0)), "for.cond");
					break;
					
				default:
					break;
			}
		}

		d->irBuilder.CreateCondBr(condition2, loopBB, d->afterLoopBB);
		
		//restore hidden locals
		const std::map<std::string, llvm::Value *> & hiddenLocals = d->hiddenLocals.back();
		for (auto it = hiddenLocals.begin(); it != hiddenLocals.end(); ++it) {
			d->namedValues[it->first] = it->second;
		}
		d->hiddenLocals.pop_back();

		d->irBuilder.SetInsertPoint(d->afterLoopBB);

		d->afterLoopBB = afterLoopBBBackup;
		
	} else {
		std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	
	return nullptr;
}

llvm::Value * LILIREmitter::_emitLoop(LILFlowControl * value)
{
	auto currentBB = d->irBuilder.GetInsertBlock();
	auto fun = currentBB->getParent();
	auto loopBB = llvm::BasicBlock::Create(d->llvmContext, "loop", fun);
	auto afterLoopBBBackup = d->afterLoopBB;
	d->afterLoopBB = llvm::BasicBlock::Create(d->llvmContext, "for.after", fun);

	d->irBuilder.CreateBr(loopBB);
	d->irBuilder.SetInsertPoint(loopBB);
	
	auto condVd = std::make_shared<LILVarDecl>();
	condVd->setParentNode(value->shared_from_this());
	LILString condName("_lil_loop_repeat");
	condVd->setName(condName);
	value->setLocalVariable(condName, condVd);
	auto boolVal = std::make_shared<LILBoolLiteral>();
	boolVal->setValue(false);
	auto boolTy = std::make_shared<LILType>();
	boolTy->setName("bool");
	boolVal->setType(boolTy);
	condVd->setType(boolTy);
	condVd->setInitVal(boolVal);
	this->emit(condVd.get());
	
	this->_emitEvaluables(value->getThen());
	
	auto exp = std::make_shared<LILExpression>();
	exp->setExpressionType(ExpressionTypeEqualComparison);
	exp->setParentNode(value->shared_from_this());
	exp->setType(boolTy);
	auto leftVn = std::make_shared<LILVarName>();
	leftVn->setName(condName);
	leftVn->setType(boolTy);
	exp->setLeft(leftVn);
	auto rightVal = std::make_shared<LILBoolLiteral>();
	rightVal->setValue(true);
	exp->setRight(rightVal);
	auto condition = this->emit(exp.get());

	d->irBuilder.CreateCondBr(condition, loopBB, d->afterLoopBB);
	d->irBuilder.SetInsertPoint(d->afterLoopBB);

	d->afterLoopBB = afterLoopBBBackup;
	return nullptr;
}

llvm::Value * LILIREmitter::_emitFlowCCall(LILFlowControlCall * value)
{
	switch (value->getFlowControlCallType()) {
		case FlowControlCallTypeReturn:
		{
			return this->_emitReturn(value);
		}
		case FlowControlCallTypeRepeat:
		{
			return this->_emitRepeat(value);
		}
		case FlowControlCallTypeBreak:
		{
			return this->_emitBreak(value);
		}
		default:
		{
			std::cerr << "!!!!!!!!!! UNKNOWN FLOW CONTROL CALL TYPE!!!!!!!!!!!!!!!!\n";
			return nullptr;
		}
	}
	
	return nullptr;
}

llvm::Value * LILIREmitter::_emitReturn(LILFlowControlCall * value)
{
	auto arg = value->getArgument();
	auto allocaBackup = d->currentAlloca;
	if (arg) {
		d->currentAlloca = d->returnAlloca;
		d->needsReturnValue = true;

		auto ty = value->getType();
		llvm::Value * retVal;
		if (ty->isA(TypeTypeMultiple)) {
			retVal = this->emitForMultipleType(arg.get(), std::static_pointer_cast<LILMultipleType>(ty));
		} else if (ty->getIsNullable()) {
			retVal = this->emitNullable(arg.get(), ty.get());
		} else {
			retVal = this->emit(arg.get());
		}
		if (retVal) {
			this->_convertLlvmValueIfNeeded(&retVal, ty.get(), arg->getType().get());
			llvm::Value * theReturn = d->irBuilder.CreateStore(retVal, d->currentAlloca);
			d->currentAlloca = allocaBackup;
			return theReturn;
		}
	}
	d->currentAlloca = allocaBackup;
	return nullptr;
}

llvm::Value * LILIREmitter::_emitRepeat(LILFlowControlCall * value)
{
	auto asgmt = std::make_shared<LILAssignment>();
	auto vn = std::make_shared<LILVarName>();
	vn->setName("_lil_loop_repeat");
	asgmt->setSubject(vn);
	auto boolVal = std::make_shared<LILBoolLiteral>();
	boolVal->setValue(true);
	boolVal->setType(LILType::make("bool"));
	asgmt->setValue(boolVal);
	auto ty = std::make_shared<LILType>();
	ty->setName("bool");
	asgmt->setType(ty);
	return this->emit(asgmt.get());
}

llvm::Value * LILIREmitter::_emitBreak(LILFlowControlCall * value)
{
	return d->irBuilder.CreateBr(d->afterLoopBB);
}

llvm::Value * LILIREmitter::_emitInstr(LILInstruction * value)
{
	return nullptr;
}

llvm::Value * LILIREmitter::_emitForeignLang(LILForeignLang * value)
{
	if (value->getLanguage() == "llvm") {
		llvm::StringRef llvmStr = value->getContent().data().c_str();
		std::unique_ptr<llvm::MemoryBuffer> llvmBuf = llvm::MemoryBuffer::getMemBuffer(llvmStr);
		llvm::SMDiagnostic err;
		llvm::SourceMgr sourceMgr;

		auto parent = value->getParentNode();
		switch (parent->getNodeType()) {
			case NodeTypeFunctionDecl:
			case NodeTypeFlowControl:
			{
				if (this->getVerbose()) {
					std::cerr << "\n=====  PARSING LLVM IR  ====\n\n";
				}

				llvm::LLVMIRParser llParser(llvmBuf->getBuffer(), sourceMgr, err, d->llvmContext, this);
				llParser.run();
				
				if (this->getVerbose()) {
					std::cerr << "\n\n== Finished parsing LLVM IR. ==\n\n";
				}
				break;
			}
			case NodeTypeRoot:
			{
				llvm::ModuleSummaryIndex index(true);
				auto buf = llvmBuf.get();
				sourceMgr.AddNewSourceBuffer(std::move(llvmBuf), llvm::SMLoc());
				llvm::parseAssemblyInto(*buf, &d->llvmModule, &index, err);
				break;
			}
			default:
				break;
		}
	}
	
	return nullptr;
}

llvm::Value * LILIREmitter::_emitValList(LILValueList * value)
{
	if (d->currentAlloca == nullptr) {
		std::cerr << "CURRENT ALLOCA WAS NULL FAIL !!!!!!!!\n\n";
		return nullptr;
	}
	auto allocaBackup = d->currentAlloca;
	auto ty = value->getType();
	auto llvmTy = this->llvmTypeFromLILType(ty.get());
	if (ty->isA(TypeTypeStaticArray)) {
		//FIXME: optimize this into a global and memcpy when profitable
		size_t i = 0;
		for (auto node : value->getValues()) {
			d->currentAlloca = this->_emitGEP(allocaBackup, llvmTy, false, 0, "", true, true, i);
			auto ir = this->emit(node.get());
			if (ir != nullptr) {
				auto gep = this->_emitGEP(allocaBackup, llvmTy, false, 0, "", true, true, i);
				this->_convertLlvmValueIfNeeded(&ir, ty->getType().get(), node->getType().get());
				d->irBuilder.CreateStore(ir, gep);
			}
			i += 1;
		}
	} else if (ty->isA(TypeTypeObject) && ty->getName().substr(0, 9) == "lil_array") {
		auto cd = this->findClassWithName(ty->getName());
		if (!cd) {
			std::cerr << "ARRAY CLASS NOT FOUND FAIL!!!! \n\n";
			return nullptr;
		}
		auto ctor = cd->getMethodNamed("construct");
		if (ctor) {
			if (!ctor->isA(NodeTypeFunctionDecl)) {
				std::cerr << "CONSTRUCTOR NODE WAS NOT FUNCTION DECL FAIL!!!\n\n";
				return nullptr;
			}
			auto fd = std::static_pointer_cast<LILFunctionDecl>(ctor);
			llvm::Function* fun = d->llvmModule.getFunction(fd->getName().data());
			if (fun) {
				std::vector<llvm::Value *> argsvect;
				argsvect.push_back(d->currentAlloca);
				d->irBuilder.CreateCall(fun, argsvect);
			} else {
				std::cerr << "COULD NOT CALL CONSTRUCTOR FAIL!!!!\n\n";
				return nullptr;
			}
		}
		
		const auto & values = value->getValues();
		size_t valuesSize = values.size();
		const auto & fields = cd->getFields();
		size_t index = 0;
		for (auto field : fields) {
			if (field->isA(NodeTypeVarDecl)) {
				auto fldTy = field->getType();
				auto vd = std::static_pointer_cast<LILVarDecl>(field);
				auto fldName = vd->getName();
				if (fldName == "buffer") {
					if (!fldTy->isA(TypeTypeMultiple)) {
						std::cerr << "BUFFER FIELD WAS NOT MULTIPLE TYPE FAIL!!!! \n\n";
						return nullptr;
					}
					auto mtTy = std::static_pointer_cast<LILMultipleType>(fldTy);
					std::shared_ptr<LILType> saTy;
					bool saTyFound = false;
					for (auto mtTyTy : mtTy->getTypes()) {
						if (mtTyTy->isA(TypeTypeStaticArray)) {
							saTy = mtTyTy;
							saTyFound = true;
							break;
						}
					}
					if (!saTyFound) {
						std::cerr << "STATIC ARRAY TYPE NOT FOUND FAIL!!!! \n\n";
						return nullptr;
					}

					if (valuesSize <= LIL_ARRAY_SMALL_BUFFER_SIZE) {
						d->currentAlloca = this->_emitGEP(d->currentAlloca, llvmTy, true, index, fldName, true, false, 0);
						value->setType(saTy);
						this->emitForMultipleType(value, std::static_pointer_cast<LILMultipleType>(fldTy));
						value->setType(ty);
						d->currentAlloca = allocaBackup;
					}
					else
					{
						auto initializeMethod = cd->getMethodNamed("initialize");
						if (initializeMethod) {
							if (!initializeMethod->isA(NodeTypeFunctionDecl)) {
								std::cerr << "NODE WAS NOT FUNCTION DECL FAIL!!!!!!!!!!!!!!!!\n\n";
								return nullptr;
							}
							auto fd = std::static_pointer_cast<LILFunctionDecl>(initializeMethod);
							llvm::Function* fun = d->llvmModule.getFunction(fd->getName().data());
							llvm::Value * pointer;
							if (fun) {
								std::vector<llvm::Value *> argsvect;
								argsvect.push_back(d->currentAlloca);
								size_t capacityNum = valuesSize;
								if (valuesSize < LIL_ARRAY_BIG_BUFFER_MIN_SIZE) {
									capacityNum = LIL_ARRAY_BIG_BUFFER_MIN_SIZE;
								}
								auto capacityVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, capacityNum, false));
								argsvect.push_back(capacityVal);
								pointer = d->irBuilder.CreateCall(fun, argsvect);
							} else {
								std::cerr << "COULD NOT CALL INITIALIZE FUNCTION FAIL!!!!!!!!!!!!!!!!\n\n";
								return nullptr;
							}
							size_t i = 0;
							for (auto node : value->getValues()) {
								d->currentAlloca = this->_emitGEP(pointer, this->llvmTypeFromLILType(saTy.get()), false, 0, "", true, true, i);
								auto ir = this->emit(node.get());
								if (ir) {
									this->_convertLlvmValueIfNeeded(&ir, saTy->getType().get(), node->getType().get());
									d->irBuilder.CreateStore(ir, d->currentAlloca);
								}
								d->currentAlloca = allocaBackup;
								i += 1;
							}
						} else {
							std::cerr << "INITIALIZE METHOD NOT FOUND FAIL!!!!!!!!!!!!!!!!\n\n";
							return nullptr;
						}
					}
				} else if (fldName == "size") {
					auto gep = this->_emitGEP(d->currentAlloca, llvmTy, true, index, fldName, true, false, 0);
					auto sizeVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, valuesSize, false));
					d->irBuilder.CreateStore(sizeVal, gep);
				}
			}
			index += 1;
		}
	} else if (ty->isA(TypeTypeSIMD)) {
		std::vector<llvm::Constant *> values;
		for (auto node : value->getValues()) {
			auto irVal = this->emit(node.get());
			values.push_back(llvm::cast<llvm::Constant>(irVal));
		}
		d->irBuilder.CreateStore(llvm::ConstantVector::get(values), d->currentAlloca);
		
	}
	d->currentAlloca = allocaBackup;
	return nullptr;
}

void LILIREmitter::receiveLLVMIRData(llvm::LLVMIRParserEvent eventType, std::string data)
{
	switch (eventType) {
		case llvm::LLVMIRParserEventGlobalID:
		case llvm::LLVMIRParserEventGlobalVar:
		{
			std::cerr << "@" + data;
			break;
		}
		case llvm::LLVMIRParserEventLocalVar:
		case llvm::LLVMIRParserEventLocalVarID:
		{
			std::cerr << "%" + data;
			break;
		}
		default:
			std::cerr << data;
			break;
	}
}

llvm::Value * LILIREmitter::emitPointer(LILNode * node)
{
	switch (node->getNodeType()) {
		case NodeTypeValuePath:
		{
			return this->_emitPointer(static_cast<LILValuePath *>(node));
		}
		case NodeTypeNumberLiteral:
		{
			//create global variable with the constant as initial value
			//mark variable as being constant
			//set linkage to private
			auto globalDeclaration = new llvm::GlobalVariable(d->llvmModule, this->llvmTypeFromLILType(node->getType().get()), true, llvm::GlobalVariable::PrivateLinkage, nullptr, "num");
			
			auto constValue = this->_emitNum(static_cast<LILNumberLiteral *>(node));
			globalDeclaration->setInitializer(llvm::cast<llvm::Constant>(constValue));
			globalDeclaration->setConstant(true);
			globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
			globalDeclaration->setUnnamedAddr (llvm::GlobalValue::UnnamedAddr::Global);
			return globalDeclaration;
		}
		case NodeTypeVarName:
		{
			auto vn = static_cast<LILVarName *>(node);
			LILString name = vn->getName();
			if (!d->namedValues.count(name.data())) {
				std::cerr << "!!!!!!!!!!UNKNOWN VARIABLE FAIL!!!!!!!!!!!!!!!!\n";
				return nullptr;
			}
			llvm::Value * val = d->namedValues[name.data()];
			return val;
		}
		case NodeTypeStringLiteral:
		{
			return this->_emitStr(static_cast<LILStringLiteral *>(node));
		}
		case NodeTypeFunctionCall:
		{
			auto fc = static_cast<LILFunctionCall *>(node);
			d->currentAlloca = d->irBuilder.CreateAlloca(this->llvmTypeFromLILType(fc->getReturnType().get()));
			auto returnVal = this->_emitFC(fc);
			d->irBuilder.CreateStore(returnVal, d->currentAlloca);
			return d->currentAlloca;
		}
		default:
			std::cerr << "!!!!!!!!!!EMIT POINTER FAIL!!!!!!!!!!!!!!!!\n";
			break;
	}
	return nullptr;
}

llvm::Value * LILIREmitter::_emitPointer(LILValuePath * value)
{
	const auto & childNodes = value->getNodes();
	if (childNodes.size() == 1)
	{
		auto firstNode = childNodes.front();
		switch (firstNode->getNodeType()) {
			case NodeTypeVarName:
			{
				auto vn = std::static_pointer_cast<LILVarName>(firstNode);
				LILString name = vn->getName();
				if (!d->namedValues.count(name.data())) {
					std::cerr << "!!!!!!!!!!UNKNOWN VARIABLE FAIL!!!!!!!!!!!!!!!!\n";
					return nullptr;
				}
				llvm::Value * val = d->namedValues[name.data()];
				return val;
			}

			default:
				std::cerr << "!!!!!!!!!!UNKNOWN NODE TYPE IN VALUE PATH FAIL!!!!!!!!!!!!!!!!\n";
				return nullptr;
		}
	}
	else if (childNodes.size() > 1)
	{
		auto it = childNodes.begin();
		auto firstNode = *it;
		llvm::Value * llvmSubject = nullptr;
		std::shared_ptr<LILType> currentTy;
		LILString instanceName;
		LILString stringRep;
		
		if (firstNode->isA(NodeTypeVarName)) {
			auto vn = std::static_pointer_cast<LILVarName>(firstNode);
			stringRep = vn->getName();
			std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
			if (subjectNode && subjectNode->isA(NodeTypeVarDecl)) {
				auto vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
				instanceName = vd->getName();
				llvmSubject = d->namedValues[instanceName.data()];
				auto vdTy = vd->getType();
				if (!vdTy) {
					std::cerr << "TYPE OF VAR DECL WAS NULL FAIL !!!!!!!!!!!!!!!!\n";
					return nullptr;
				}
				currentTy = vdTy;
			}
		} else {
			//selector
			auto sel = std::static_pointer_cast<LILSelector>(firstNode);
			switch (sel->getSelectorType()) {
				case SelectorTypeSelfSelector:
				{
					llvmSubject = d->namedValues["@self"];
					auto classDecl = this->findAncestorClass(value->shared_from_this());
					currentTy = classDecl->getType();
					stringRep = "@self";
					break;
				}
				default:
					std::cerr << "!!!!!!!!!!UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!\n";
					return nullptr;
			}
		}
		
		++it;
		size_t i = 1;
		
		while (it != childNodes.end()) {
			auto currentNode = *it;
			switch (currentNode->getNodeType()) {
				case NodeTypeFunctionCall:
				{
					std::cerr << "!!!!!!!!!!POINTER TO: FUNCTION CALL IN VALUE PATH IS UNSUPPORTED!!!!!!!!!!!!!!!!\n";
					return nullptr;
				}
					
				case NodeTypePropertyName:
				{
					if (llvmSubject == nullptr) {
						std::cerr << "!!!!!!!!!!SUBJECT OF VALUE PATH WAS NULL!!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto pn = std::static_pointer_cast<LILPropertyName>(currentNode);
					stringRep += "." + pn->getName();
					
					//get index of field into struct
					if (currentTy->isA(TypeTypePointer)) {
						llvmSubject = d->irBuilder.CreateLoad(this->llvmTypeFromLILType(currentTy.get()), llvmSubject);
						auto ptrTy = std::static_pointer_cast<LILPointerType>(currentTy);
						currentTy = ptrTy->getArgument();
					}
					if (!currentTy->isA(TypeTypeObject)) {
						std::cerr << "CURRENT TYPE WAS NOT OBJECT TYPE FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto classDecl = this->findClassWithName(currentTy->getName());
					if (!classDecl) {
						std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
						return nullptr;
					}
					auto field = classDecl->getFieldNamed(pn->getName());
					if (!field) {
						field = classDecl->getMethodNamed(pn->getName());
						if (field && field->getNodeType() == NodeTypeFunctionDecl) {
							auto fd = std::static_pointer_cast<LILFunctionDecl>(field);
							auto fnName = fd->getName().data();
							if (d->namedValues.count(fnName) == 0) {
								std::cerr << "FUNCTION POINTER NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
								return nullptr;
							}
							return d->namedValues.at(fnName);
						}
					}
					auto fieldTy = field->getType();
					bool useIfCastTy = false;
					if (!this->inhibitSearchingForIfCastType && fieldTy->isA(TypeTypeMultiple)) {
						size_t outStartIndex;
						auto ifCastTy = this->findIfCastType(value, outStartIndex);
						if (ifCastTy) {
							useIfCastTy = true;
							if (outStartIndex != i+1) {
								std::cerr << "!!!!!!!!!!WRONG VALUE PATH FAIL !!!!!!!!!!!!!!!!\n\n";
								return nullptr;
							}
							llvmSubject = this->emitUnwrappedPointerFromMT(llvmSubject, ifCastTy.get(), static_cast<LILMultipleType *>(fieldTy.get()));
							currentTy = ifCastTy;
						}
					}
					
					if (!useIfCastTy) {
						bool fieldFound = false;
						size_t theIndex = classDecl->getIndexOfField(field, fieldFound);
						if (!fieldFound) {
							std::cerr << "FIELD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n\n";
							return nullptr;
						}
						
						std::string name = pn->getName().data();
						llvmSubject = this->_emitGEP(llvmSubject, classDecl->getName(), theIndex, stringRep, true);
						currentTy = fieldTy;
					}
					break;
				}
					
				case NodeTypeIndexAccessor:
				{
					auto ia = std::static_pointer_cast<LILIndexAccessor>(currentNode);
					auto arg = ia->getArgument();
					switch (currentTy->getTypeType()) {
						case TypeTypeStaticArray:
						{
							auto saTy = std::static_pointer_cast<LILStaticArrayType>(currentTy);
							
							std::vector<llvm::Value *> idList;
							//step through the pointer
							idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
							//add the index into the array
							auto argIr = this->emit(arg.get());
							if (!argIr) {
								std::cerr << "CODEGEN OF ARGUMENT OF INDEX ACCESSOR FAILED!!!!!!!!!!!!!!!!\n";
								return nullptr;
							}
							idList.push_back(argIr);
							llvmSubject = llvm::GetElementPtrInst::Create(this->llvmTypeFromLILType(currentTy.get()), llvmSubject, idList, "", d->irBuilder.GetInsertBlock());
							currentTy = saTy->getType();
							break;
						}
						default:
							std::cerr << "!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
							return nullptr;
					}
					break;
				}
					
				default:
					break;
			}
			
			++it;
			++i;
		}
		return llvmSubject;
	}
	std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
	return nullptr;
}

void LILIREmitter::printIR(llvm::raw_ostream & file) const
{
	d->llvmModule.print(file, nullptr);
}

llvm::Type * LILIREmitter::llvmTypeFromLILType(LILType * type)
{
	if (type->isA(TypeTypeMultiple)) {
		auto multiTy = static_cast<LILMultipleType *>(type);
		auto mostAlignedType = this->getMostAlignedType(multiTy->getTypes());
		std::vector<llvm::Type *> llvmTypes;
		llvmTypes.push_back(this->llvmTypeFromLILType(mostAlignedType.get()));
		llvmTypes.push_back(llvm::Type::getInt8Ty(d->llvmContext));
		return llvm::StructType::get(d->llvmContext, llvmTypes);
	}
	
	if (type->isA(TypeTypePointer))
	{
		auto ptrTy = static_cast<LILPointerType *>(type);
		auto argTy = ptrTy->getArgument();
		if (!argTy) {
			std::cerr << "!!!!!!!!!!PTR TYPE EMPTY ARG FAIL!!!!!!!!!!!!!!!!\n";
			return nullptr;
		}
		auto llvmType = this->llvmTypeFromLILType(argTy.get());
		return llvmType->getPointerTo();
	}
	else if (type->isA(TypeTypeObject))
	{
		auto className = type->getName().data();
		llvm::StructType * classTy;
		if (d->classTypes.count(className)) {
			return d->classTypes.at(className);
		} else {
			auto classDecl = this->findClassWithName(className);
			if (!classDecl) {
				std::cerr << "CLASS NOT FOUND FAIL!!!!!!!!!!!!!!!!\n";
				return nullptr;
			}
			auto newClassTy = this->extractStructFromClass(classDecl.get());
			classTy = newClassTy;
			
			if (type->getIsNullable()) {
				std::vector<llvm::Type*> types;
				types.push_back(classTy);
				types.push_back(llvm::Type::getInt1Ty(d->llvmContext));
				return llvm::StructType::get(d->llvmContext, types);
			} else {
				return classTy;
			}
		}
	}
	else if (type->isA(TypeTypeStaticArray))
	{
		auto sa = static_cast<LILStaticArrayType *>(type);
		auto arrTy = sa->getType();
		if (!arrTy) {
			std::cerr << "STATIC ARRAY TYPE HAD NO ELEM TYPE FAIL!!!!!!!!!!!!!!!!\n";
			return nullptr;
		}
		auto arrType = this->llvmTypeFromLILType(arrTy.get());
		auto arg = sa->getArgument();
		if (arg) {
			arg = this->recursiveFindNode(arg);
		}
		if (arg && arg->isA(NodeTypeVarDecl)) {
			auto vd = std::static_pointer_cast<LILVarDecl>(arg);
			arg = vd->getInitVal();
		}
		if (arg && arg->isA(NodeTypeExpression)) {
			arg = this->_evaluateLiteralExpression(std::static_pointer_cast<LILExpression>(arg));
		}
		if (!arg || !arg->isA(NodeTypeNumberLiteral)) {
			std::cerr << "STATIC ARRAY ARGUMENT WAS NOT NUMBER FAIL!!!!!!!!!!!!!!!!\n";
			return nullptr;
		}
		auto num = std::static_pointer_cast<LILNumberLiteral>(arg);
		auto numElements = this->extractSizeFromNumberLiteral(num.get());
		return llvm::ArrayType::get(arrType, numElements);
	}
	else if (type->isA(TypeTypeFunction))
	{
		auto fnTy = static_cast<LILFunctionType *>(type);
		return this->_emitLlvmFnType(fnTy);
	}
	else if (type->isA(TypeTypeSIMD))
	{
		auto simdTy = static_cast<LILSIMDType *>(type);
		auto vectTy = llvm::FixedVectorType::get(this->llvmTypeFromLILType(simdTy->getType().get()), simdTy->getWidth());
		return vectTy;
	}

	LILString typestr = type->getName();
	llvm::Type * ret = nullptr;
	if (typestr == "bool") {
		if (type->getIsNullable()) {
			return llvm::IntegerType::get(d->llvmContext, 2);
		} else {
			return llvm::Type::getInt1Ty(d->llvmContext);
		}
	} else if (typestr == "any" || typestr == "i8" || typestr == "i8%"){
		ret = llvm::Type::getInt8Ty(d->llvmContext);
	} else if (typestr == "i16" || typestr == "i16%"){
		ret = llvm::Type::getInt16Ty(d->llvmContext);
	} else if (typestr == "i32" || typestr == "i32%"){
		ret = llvm::Type::getInt32Ty(d->llvmContext);
	} else if (typestr == "i64" || typestr == "i64%"){
		ret = llvm::Type::getInt64Ty(d->llvmContext);
	} else if (typestr == "f32" || typestr == "f32%"){
		ret = llvm::Type::getFloatTy(d->llvmContext);
	} else if (typestr == "f64" || typestr == "f64%"){
		ret = llvm::Type::getDoubleTy(d->llvmContext);
	} else if (typestr == "null") {
		return llvm::Type::getVoidTy(d->llvmContext);
	}
	
	if (!ret) {
		auto enumVal = this->findEnumWithName(typestr);
		if (enumVal) {
			auto enumTy = enumVal->getType();
			if (enumTy) {
				ret = this->llvmTypeFromLILType(enumTy.get());
			}
		}
	}

	if (type->getIsNullable()) {
		std::vector<llvm::Type*> types;
		types.push_back(ret);
		types.push_back(llvm::Type::getInt1Ty(d->llvmContext));
		ret = llvm::StructType::get(d->llvmContext, types);
	}

	if (ret)
		return ret;
	
	std::cerr << "!!!!!!!!!!COULD NOT MAKE LLVM TYPE FAIL!!!!!!!!!!!!!!!!\n";
	return nullptr;
}

std::shared_ptr<LILFunctionDecl> LILIREmitter::chooseFnByType(std::vector<std::shared_ptr<LILFunctionDecl>> funcDecls, std::vector<std::shared_ptr<LILType>> types)
{
	for (auto fd : funcDecls) {
		auto fnTy = fd->getFnType();
		auto fdTys = fnTy->getArguments();
		if (fdTys.size() != types.size()) {
			continue;
		}
		//the sizes are equal at this point, so if there are no types
		//this is the function without arguments
		if (types.size() == 0) {
			return fd;
		}
		std::shared_ptr<LILType> ty;
		bool match = true;
		for (size_t i=0, j=fdTys.size(); i<j; ++i) {
			ty.reset();
			auto arg = fdTys[i];
			if (arg->isA(NodeTypeType)) {
				ty = std::static_pointer_cast<LILType>(arg);
			} else if (arg->isA(NodeTypeVarDecl)) {
				ty = std::static_pointer_cast<LILVarDecl>(arg)->getType();
			}
			if (ty) {
				if (ty->getIsWeakType() || types[i]->getIsWeakType()) {
					ty = LILType::merge(ty, types[i]);
				}
				if (ty->equalTo(types[i])) {
					match = true;
				} else if (ty->isA(TypeTypePointer) && std::static_pointer_cast<LILPointerType>(ty)->getArgument()->equalTo(types[i])) {
					match = true;
				} else if (types[i]->isA(TypeTypePointer) && std::static_pointer_cast<LILPointerType>(types[i])->getArgument()->equalTo(ty)){
					match = true;
				} else {
					match = false;
					break;
				}
			}
		}
		if (match) {
			return fd;
		} else {
			for (size_t i=0, j=fdTys.size(); i<j; i+=1) {
				ty.reset();
				auto arg = fdTys[i];
				if (arg->isA(NodeTypeType)) {
					ty = std::static_pointer_cast<LILType>(arg);
				} else if (arg->isA(NodeTypeVarDecl)) {
					ty = std::static_pointer_cast<LILVarDecl>(arg)->getType();
				}
				if (ty) {
					auto otherTy = types.at(i);
					if (ty->getIsWeakType() || otherTy->getIsWeakType()) {
						ty = LILType::merge(ty, otherTy);
					}
					if (ty->equalTo(otherTy)) {
						match = true;
					} else if (ty->isA(TypeTypePointer) && std::static_pointer_cast<LILPointerType>(ty)->getArgument()->equalTo(otherTy)) {
						match = true;
					} else if (otherTy->isA(TypeTypePointer) && std::static_pointer_cast<LILPointerType>(otherTy)->getArgument()->equalTo(ty)){
						match = true;
					} else if (LILType::isIntegerType(otherTy.get()) && LILType::isIntegerType(ty.get())) {
						match = (ty->getBitWidth() > otherTy->getBitWidth());
					} else {
						match = false;
						break;
					}
				}
			}
		}
		if (match) {
			return fd;
		}
	}
	return nullptr;
}

llvm::AllocaInst * LILIREmitter::createEntryBlockAlloca(llvm::Function * fun, const std::string & name, llvm::Type * llvmType)
{
	llvm::IRBuilder<> tmpBuilder(&fun->getEntryBlock(), fun->getEntryBlock().begin());
	return tmpBuilder.CreateAlloca(llvmType, 0, name.c_str());
}

void LILIREmitter::setDebug(bool value)
{
	this->_debug = value;
}

llvm::StructType * LILIREmitter::extractStructFromClass(LILClassDecl * value)
{
	auto name = value->getName().data();
	if (d->classTypes.count(name)) {
		return d->classTypes[name];
	}
	auto structType = llvm::StructType::create(d->llvmContext, name);
	d->classTypes[name] = structType;
	std::vector<llvm::Type*> types;
	for (auto & fld : value->getFields()) {
		auto vd = std::static_pointer_cast<LILVarDecl>(fld);
		if (!vd->getIsVVar()) {
			llvm::Type * llvmTy = this->llvmTypeFromLILType(fld->getType().get());
			if (llvmTy) {
				types.push_back(llvmTy);
			}
		}
	}
	structType->setBody(types);
	return structType;
}

size_t LILIREmitter::extractSizeFromNumberLiteral(LILNumberLiteral * value) const
{
	auto numStr = value->getValue().data();
	auto numCstr = numStr.c_str();
	char * endPtr;
	size_t numElements = std::strtoull(numCstr, &endPtr, 10);
	return numElements;
}

llvm::Value * LILIREmitter::emitNullable(LILNode * node, LILType * targetTy)
{
	//nullable pointer
	if (targetTy->isA(TypeTypePointer)) {
		if (node->isA(NodeTypeNull)) {
			auto ptrTy = static_cast<LILPointerType *>(targetTy);
			auto argTy = ptrTy->getArgument();
			auto argLlvmTy = this->llvmTypeFromLILType(argTy.get());
			return llvm::ConstantPointerNull::get(argLlvmTy->getPointerTo());
		} else {
			return this->emit(node);
		}
	}

	//nullable bool
	switch (node->getNodeType()) {
		case NodeTypeBoolLiteral:
		{
			auto bl = static_cast<LILBoolLiteral *>(node);
			if (bl->getValue()) {
				return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 0b01, false));
			} else {
				return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 0b00, false));
			}
			break;
		}
		case NodeTypeNull:
		{
			return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 0b11, false));
		}

		default:
			break;
	}

	return nullptr;
}

llvm::Value * LILIREmitter::emitForMultipleType(LIL::LILNode *node, std::shared_ptr<LILMultipleType> multiTy)
{
	auto allocaBackup = d->currentAlloca;
	LILString name;
	if (node->isA(NodeTypeVarName)) {
		name = static_cast<LILVarName *>(node)->getName();
	} else {
		name = "tmp";
	}
	auto ty = node->getType();
	if (!ty) {
		std::cerr << "NODE HAD NO TY FAIL!!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	if (ty->isA(TypeTypeMultiple)) {
		if (ty->equalTo(multiTy)) {
			return this->emit(node);
		} else {
			return this->emitMultiTyToMultiTyConversion(node, multiTy.get(), name);
		}
	} else if (ty->getIsNullable()) {
		return this->emitNullableToMultiTyConversion(node, multiTy.get(), name);
	}

	auto llvmTy = this->llvmTypeFromLILType(ty.get());
	auto llvmMultiTy = this->llvmTypeFromLILType(multiTy.get());
	const auto & mtTypes = multiTy->getTypes();
	std::vector<llvm::Value *> gepIndices1;
	gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	auto member1 = d->irBuilder.CreateGEP(llvmMultiTy, d->currentAlloca, gepIndices1);
	d->currentAlloca = member1;

	if (!node->isA(NodeTypeNull)) {
		llvm::Value * llvmIr = this->emit(node);
		
		auto destLlvmTy = llvmTy->getPointerTo();
		if (member1->getType() != destLlvmTy) {
			d->currentAlloca = d->irBuilder.CreateBitCast(member1, destLlvmTy);
		}
		
		if (llvmIr) {
			d->irBuilder.CreateStore(llvmIr, d->currentAlloca);
		}
	}

	d->currentAlloca = allocaBackup;

	size_t theIndex = 1;
	bool found = false;
	for (auto mtTy : mtTypes) {
		if (mtTy->equalTo(ty)) {
			found = true;
			break;
		}
		theIndex += 1;
	}
	//when it's null
	if (!found) {
		if (!multiTy->getIsNullable()) {
			std::cerr << "TYPE NOT FOUND IN MULTIPLE TYPE FAIL!!!!!!!!!!!!!!!!\n\n";
			return nullptr;
		}
		theIndex = 0;
		
	}
	auto typeIndex = llvm::ConstantInt::get(
		d->llvmContext,
		llvm::APInt(
			8,
			theIndex,
			false
		)
	);
	std::vector<llvm::Value *> gepIndices2;
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
	auto member2 = d->irBuilder.CreateGEP(llvmMultiTy, d->currentAlloca, gepIndices2);
	d->irBuilder.CreateStore(typeIndex, member2);
	
	return nullptr;
}

llvm::Value * LILIREmitter::emitNullableToMultiTyConversion(LILNode * node, LILMultipleType * multiTy, const LILString & name)
{
	auto allocaBackup = d->currentAlloca;
	auto namestr = name.data();
	auto ty = node->getType();
	auto llvmTy = this->llvmTypeFromLILType(ty.get());
	auto llvmMultiTy = this->llvmTypeFromLILType(multiTy);
	llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();

	std::vector<llvm::Value *> gepIndices1;
	gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	auto member1 = d->irBuilder.CreateGEP(llvmMultiTy, d->currentAlloca, gepIndices1);
	d->currentAlloca = member1;
	auto nullableValue = this->emit(node);
	d->currentAlloca = allocaBackup;

	std::vector<llvm::Value *> gepIndices2;
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
	auto member2 = d->irBuilder.CreateGEP(llvmMultiTy, d->currentAlloca, gepIndices2);
	auto types2 = multiTy->getTypes();
	size_t index;
	ty->setIsNullable(false);
	for (size_t i = 0, j = types2.size(); i<=j; ++i) {
		auto ty2 = types2.at(i);
		if (ty->equalTo(ty2)) {
			index = i+1;
			break;
		}
	}
	ty->setIsNullable(true);

	llvm::Value * cond;
	bool isPtr = ty->isA(TypeTypePointer);
	if (isPtr) {
		auto ptrType = llvm::cast<llvm::PointerType>(llvmTy);
		auto zeroVal = llvm::ConstantPointerNull::get(ptrType);
		cond = d->irBuilder.CreateICmpNE(nullableValue, zeroVal, namestr + ".not.null.cond");
	} else {
		auto zeroVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 0, true));
		cond = d->irBuilder.CreateICmpSGE(nullableValue, zeroVal, namestr + ".not.null.cond");
	}

	llvm::BasicBlock * notNullBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".if.not.null", fun);
	llvm::BasicBlock * isNullBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".if.null");
	llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".if.end");
	d->irBuilder.CreateCondBr(cond, notNullBB, isNullBB);
	d->irBuilder.SetInsertPoint(notNullBB);
	if (isPtr) {
		d->irBuilder.CreateStore(nullableValue, member1);
	} else {
		auto castedMember1 = d->irBuilder.CreatePointerCast(member1, llvm::Type::getInt1Ty(d->llvmContext)->getPointerTo());

		auto nullableTrueVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 1, true));
		auto boolCond = d->irBuilder.CreateICmpEQ(nullableValue, nullableTrueVal, namestr + ".bool.val.cond");
		llvm::BasicBlock * ifTrueBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".bool.true", fun);
		llvm::BasicBlock * ifFalseBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".bool.false");
		llvm::BasicBlock * boolMergeBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".bool.val.end");

		d->irBuilder.CreateCondBr(boolCond, ifTrueBB, ifFalseBB);
		d->irBuilder.SetInsertPoint(ifTrueBB);
		auto trueVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(1, 1, true));

		d->irBuilder.CreateStore(trueVal, castedMember1);
		d->irBuilder.CreateBr(boolMergeBB);
		
		fun->getBasicBlockList().push_back(ifFalseBB);
		d->irBuilder.SetInsertPoint(ifFalseBB);

		auto falseVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(1, 0, true));
		d->irBuilder.CreateStore(falseVal, castedMember1);
		d->irBuilder.CreateBr(boolMergeBB);
		
		fun->getBasicBlockList().push_back(boolMergeBB);
		d->irBuilder.SetInsertPoint(boolMergeBB);
	}
	d->irBuilder.CreateStore(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, 1, false)), member2);
	d->irBuilder.CreateBr(mergeBB);
	
	fun->getBasicBlockList().push_back(isNullBB);
	d->irBuilder.SetInsertPoint(isNullBB);
	d->irBuilder.CreateStore(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, 0, false)), member2);
	d->irBuilder.CreateBr(mergeBB);

	fun->getBasicBlockList().push_back(mergeBB);
	d->irBuilder.SetInsertPoint(mergeBB);

	return nullptr;
}

llvm::Value * LILIREmitter::emitMultiTyToMultiTyConversion(LILNode * node, LILMultipleType * multiTy, const LILString & name)
{
	auto namestr = name.data();
	auto ty = node->getType();
	auto llvmTy = this->llvmTypeFromLILType(ty.get());
	auto llvmMultiTy = this->llvmTypeFromLILType(multiTy);
	llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();

	auto i8Ty = llvm::Type::getInt8Ty(d->llvmContext);
	auto indexAlloca = d->irBuilder.CreateAlloca(i8Ty);

	auto mtValue = this->emitPointer(node);
	std::vector<llvm::Value *> mtGepIndices;
	mtGepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	mtGepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
	auto mtValMember2 = d->irBuilder.CreateGEP(llvmMultiTy, mtValue, mtGepIndices);
	auto mtValIndex = d->irBuilder.CreateLoad(i8Ty, mtValMember2, namestr + "_lil_type_index");
	
	llvm::BasicBlock * defaultBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".null");
	llvm::SwitchInst * switchInstr = d->irBuilder.CreateSwitch(mtValIndex, defaultBB);
	llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, namestr + ".merge");
	
	fun->getBasicBlockList().push_back(defaultBB);
	d->irBuilder.SetInsertPoint(defaultBB);
	d->irBuilder.CreateStore(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, 0, false)), indexAlloca);
	
	d->irBuilder.CreateBr(mergeBB);

	auto types1 = std::static_pointer_cast<LILMultipleType>(ty)->getTypes();
	auto types2 = multiTy->getTypes();
	std::vector<size_t> indexes;
	for (size_t i = 0, j = types1.size(); i<j; ++i) {
		auto ty1 = types1[i];
		for (size_t k = i, h = types2.size(); k<=h; ++k) {
			auto ty2 = types2[k];
			if (ty1->equalTo(ty2)) {
				indexes.push_back(k+1);
				break;
			}
		}
	}
	size_t i = 1;
	for (auto ty1 : types1) {
		llvm::BasicBlock * bb = llvm::BasicBlock::Create(d->llvmContext, namestr +"."+ty1->getName().data(), fun);
		switchInstr->addCase(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, i, false)), bb);
		d->irBuilder.SetInsertPoint(bb);

		auto indexVal = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, indexes[i-1], false));
		d->irBuilder.CreateStore(indexVal, indexAlloca);
		
		d->irBuilder.CreateBr(mergeBB);
		i += 1;
	}

	fun->getBasicBlockList().push_back(mergeBB);
	d->irBuilder.SetInsertPoint(mergeBB);

	std::vector<llvm::Value *> gepIndices1;
	gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	auto member1 = d->irBuilder.CreateGEP(llvmTy, d->currentAlloca, gepIndices1);

	auto mtValMember1 = d->irBuilder.CreateGEP(llvmMultiTy, mtValue, gepIndices1);
	auto mtValMember1Val = d->irBuilder.CreateLoad(llvmTy, mtValMember1);
	d->irBuilder.CreateStore(mtValMember1Val, member1);

	std::vector<llvm::Value *> gepIndices2;
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
	auto member2 = d->irBuilder.CreateGEP(llvmMultiTy, d->currentAlloca, gepIndices2);
	auto typeIndex = d->irBuilder.CreateLoad(i8Ty, indexAlloca);
	d->irBuilder.CreateStore(typeIndex, member2);
	return nullptr;
}

llvm::Value * LILIREmitter::emitUnwrappedFromMT(LILNode *node, LILType *targetTy)
{
	auto wrappedVal = this->emitPointer(node);
	std::vector<llvm::Value *> gepIndices;
	auto llvmTy = this->llvmTypeFromLILType(targetTy);
	gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	auto gep = d->irBuilder.CreateGEP(llvmTy, wrappedVal, gepIndices);
	auto castedPtr = d->irBuilder.CreateBitCast(gep, llvmTy->getPointerTo());
	auto innerVal = d->irBuilder.CreateLoad(llvmTy, castedPtr);
	return innerVal;
}

llvm::Value * LILIREmitter::emitUnwrappedPointerFromMT(llvm::Value * val, LILType *targetTy, LILMultipleType * multiTy)
{
	auto llvmTy = this->llvmTypeFromLILType(targetTy);
	auto llvmMultiTy = this->llvmTypeFromLILType(multiTy);
	std::vector<llvm::Value *> gepIndices;
	gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
	auto gep = d->irBuilder.CreateGEP(llvmMultiTy, val, gepIndices);
	auto castedPtr = d->irBuilder.CreateBitCast(gep, llvmTy->getPointerTo());
	return castedPtr;
}

std::shared_ptr<LILType> LILIREmitter::getMostAlignedType(const std::vector<std::shared_ptr<LILType>> & types) const
{
	std::vector<std::shared_ptr<LILType>> biggestTys;
	size_t biggestSize = 0;
	for (auto ty : types) {
		size_t tySize = this->getSizeOfType(ty);
		if (tySize > biggestSize) {
			biggestSize = tySize;
			biggestTys.clear();
			biggestTys.push_back(ty);
		} else if (tySize == biggestSize) {
			biggestTys.push_back(ty);
		}
	}
	std::sort(biggestTys.begin(), biggestTys.end(), LILType::sortTyAlphabeticallyCompare);
	return biggestTys.front();
}

//returns size in bits
size_t LILIREmitter::getSizeOfType(std::shared_ptr<LILType> ty) const
{
	size_t ret = 0;
	switch (ty->getTypeType()) {
		case TypeTypeSingle:
		{
			return ty->getBitWidth();
		}

		case TypeTypePointer:
		{
			//FIXME: what about 32 bit systems?
			ret = 64;
			break;
		}

		case TypeTypeFunction:
		{
			//FIXME: what about 32 bit systems?
			ret = 64;
			break;
		}

		case TypeTypeObject:
		{
			auto objTy = std::static_pointer_cast<LILObjectType>(ty);
			auto classDecl = this->findClassWithName(objTy->getName());
			size_t total = 0;
			for (auto field : classDecl->getFields()) {
				if (field->isA(NodeTypeVarDecl) && std::static_pointer_cast<LILVarDecl>(field)->getIsVVar()) {
					continue;
				}
				total += this->getSizeOfType(field->getType());
			}
			ret = total;
			break;
		}
			
		case TypeTypeStaticArray:
		{
			auto saTy = std::static_pointer_cast<LILStaticArrayType>(ty);
			auto baseSize = this->getSizeOfType(saTy->getType());
			auto arg = this->recursiveFindNode(saTy->getArgument());
			if (!arg) {
				std::cerr << "ARGUMENT OF STATIC ARRAY TYPE NOT FOUND FAIL!!!!!!!!!!!!!!!!\n\n";
				return 0;
			}
			if (arg->isA(NodeTypeVarDecl)) {
				auto vd = std::static_pointer_cast<LILVarDecl>(arg);
				arg = vd->getInitVal();
			}
			if (!arg || !arg->isA(NodeTypeNumberLiteral)) {
				std::cerr << "ARGUMENT OF STATIC ARRAY TYPE NOT FOUND FAIL!!!!!!!!!!!!!!!!\n\n";
			}
			auto num = std::static_pointer_cast<LILNumberLiteral>(arg);
			size_t argNum = this->extractSizeFromNumberLiteral(num.get());
			ret = baseSize * argNum;
			break;
		}
		case TypeTypeSIMD:
		{
			auto simdTy = std::static_pointer_cast<LILSIMDType>(ty);
			auto baseSize = this->getSizeOfType(simdTy->getType());
			ret = baseSize * simdTy->getWidth();
			break;
		}

		default:
			std::cerr << "GET SIZE OF TYPE FAIL!!!!!!!!!!!!!!!!\n\n";
			return 0;
	}
	//align the size to multiple of 4
	if (ret / 4 > 0) {
		ret = ((ret + 3) / 4) * 4;
	}
	
	return ret;
}

const std::shared_ptr<LILElement> & LILIREmitter::getDOM() const
{
	return d->dom;
}

void LILIREmitter::setDOM(const std::shared_ptr<LILElement> & dom)
{
	d->dom = dom;
}

bool LILIREmitter::_needsTemporaryVariable(LILNode * node)
{
	switch (node->getNodeType()) {
		case NodeTypeStringLiteral:
		{
			auto string = static_cast<LILStringLiteral *>(node);
			return !string->getIsCString();
		}
		case NodeTypeObjectDefinition:
		case NodeTypeValueList:
		{
			return true;
		}

		default:
			return false;
	}
}

std::shared_ptr<LILNode> LILIREmitter::_evaluateLiteralExpression(std::shared_ptr<LILExpression> exp) const
{
	std::shared_ptr<LILNode> left = exp->getLeft();
	if (left->getNodeType() == NodeTypeExpression) {
		left = this->_evaluateLiteralExpression(std::static_pointer_cast<LILExpression>(left));
		if (!left) {
			std::cerr << "COULD NOT EVALUATE LEFT FAIL!!!!!!!!!!!!!!!!\n\n";
			return nullptr;
		}
	}
	if (left->getNodeType() != NodeTypeNumberLiteral) {
		std::cerr << "LEFT NODE WAS NOT NUMBER LITERAL FAIL!!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	
	std::shared_ptr<LILNode> right = exp->getRight();
	if (right->getNodeType() == NodeTypeExpression) {
		right = this->_evaluateLiteralExpression(std::static_pointer_cast<LILExpression>(right));
		if (!right) {
			std::cerr << "COULD NOT EVALUATE RIGHT FAIL!!!!!!!!!!!!!!!!\n\n";
			return nullptr;
		}
	}
	if (right->getNodeType() != NodeTypeNumberLiteral) {
		std::cerr << "RIGHT NODE WAS NOT NUMBER LITERAL FAIL!!!!!!!!!!!!!!!!\n\n";
		return nullptr;
	}
	
	auto leftNum = std::static_pointer_cast<LILNumberLiteral>(left);
	auto rightNum = std::static_pointer_cast<LILNumberLiteral>(right);
	
	std::shared_ptr<LILNumberLiteral> ret = std::make_shared<LILNumberLiteral>();
	long result = 0;
	switch (exp->getExpressionType()) {
		case ExpressionTypeSum:
		{
			result = leftNum->getValue().toLong() + rightNum->getValue().toLong();
			break;
		}
		case ExpressionTypeSubtraction:
		{
			result = leftNum->getValue().toLong() - rightNum->getValue().toLong();
			break;
		}
		case ExpressionTypeMultiplication:
		{
			result = leftNum->getValue().toLong() * rightNum->getValue().toLong();
			break;
		}
		case ExpressionTypeDivision:
		{
			result = leftNum->getValue().toLong() / rightNum->getValue().toLong();
			break;
		}
		case ExpressionTypeMod:
		{
			result = leftNum->getValue().toLong() % rightNum->getValue().toLong();
			break;
		}
			
		default:
			std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n\n";
			return nullptr;
	}
	ret->setValue(LILString::number((LILUnitI64) result));
	return ret;
}
