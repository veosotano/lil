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
 *      This file converts the ast into IR representation
 *
 ********************************************************************/


#include "LILIREmitter.h"

#include "LILFunctionType.h"
#include "LILMultipleType.h"
#include "LILPointerType.h"
#include "LILRootNode.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"



#define LILIREMITTEROPTIMIZE
#define LIL_GEP_INDEX_SIZE 32


using namespace LIL;

namespace LIL
{
    class LILIREmitterPrivate
    {
        friend class LILIREmitter;

        LILIREmitterPrivate()
        : llvmContext()
        , irBuilder(llvmContext)
        , needsReturnValue(false)
        , currentAlloca(nullptr)
        , returnAlloca(nullptr)
        {
        }
        llvm::LLVMContext llvmContext;
        llvm::IRBuilder<> irBuilder;
        std::unique_ptr<llvm::Module> llvmModule;
        std::unique_ptr<llvm::legacy::FunctionPassManager> functionPassManager;
        std::map<std::string, llvm::Value*> namedValues;
        std::vector<std::map<std::string, llvm::Value*>> hiddenLocals;
        std::map<std::string, llvm::StructType *> classTypes;
        bool needsReturnValue;
        llvm::Value * currentAlloca;
        llvm::Value * returnAlloca;
        llvm::BasicBlock * finallyBB;
    };
}

LILIREmitter::LILIREmitter(LILString name)
: d(new LILIREmitterPrivate)
, _debug(false)
{
    d->llvmModule = std::make_unique<llvm::Module>(name.data(), d->llvmContext);
    d->functionPassManager = std::make_unique<llvm::legacy::FunctionPassManager>(d->llvmModule.get());
    d->functionPassManager->add(llvm::createSROAPass());
    d->functionPassManager->add(llvm::createLICMPass());
    d->functionPassManager->add(llvm::createInstructionCombiningPass());
    d->functionPassManager->add(llvm::createDeadCodeEliminationPass());
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
    return d->llvmModule.get();
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

void LILIREmitter::visit(LILNode *node)
{
    this->emit(node);
}

llvm::Value * LILIREmitter::emit(LILNode * node)
{
    if (this->_debug) {
        std::cerr << "## emitting " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }

    switch (node->getNodeType()) {
        case NodeTypeBoolLiteral:
        {
            LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
            return this->_emit(value);
        }
        case NodeTypeNumberLiteral:
        {
            LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
            return this->_emit(value);
        }
        case NodeTypePercentage:
        {
            LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
            return this->_emit(value);
        }
        case NodeTypeExpression:
        {
            LILExpression * value = static_cast<LILExpression *>(node);
            return this->_emit(value);
        }
        case NodeTypeStringLiteral:
        {
            LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
            return this->_emit(value);
        }
        case NodeTypeStringFunction:
        {
            LILStringFunction * value = static_cast<LILStringFunction *>(node);
            return this->_emit(value);
        }
        case NodeTypeNull:
        {
            LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
            return this->_emit(value);
        }
        case NodeTypeVarDecl:
        {
            LILVarDecl * value = static_cast<LILVarDecl *>(node);
            return this->_emit(value);
        }
        case NodeTypeClassDecl:
        {
            LILClassDecl * value = static_cast<LILClassDecl *>(node);
            return this->_emit(value);
        }
        case NodeTypeObjectDefinition:
        {
            LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
            return this->_emit(value);
        }
        case NodeTypeAssignment:
        {
            LILAssignment * value = static_cast<LILAssignment *>(node);
            return this->_emit(value);
        }
        case NodeTypeValuePath:
        {
            LILValuePath * value = static_cast<LILValuePath *>(node);
            return this->_emit(value);
        }
        case NodeTypePropertyName:
        {
            LILPropertyName * value = static_cast<LILPropertyName *>(node);
            return this->_emit(value);
        }
        case NodeTypeVarName:
        {
            LILVarName * value = static_cast<LILVarName *>(node);
            return this->_emit(value);
        }
        case NodeTypeRule:
        {
            LILRule * value = static_cast<LILRule *>(node);
            return this->_emit(value);
        }
        case NodeTypeSimpleSelector:
        {
            LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
            return this->_emit(value);
        }
        case NodeTypeSelectorChain:
        {
            LILSelectorChain * value = static_cast<LILSelectorChain *>(node);
            return this->_emit(value);
        }
        case NodeTypeSelector:
        {
            LILSelector * value = static_cast<LILSelector *>(node);
            return this->_emit(value);
        }
        case NodeTypeCombinator:
        {
            LILCombinator * value = static_cast<LILCombinator *>(node);
            return this->_emit(value);
        }
        case NodeTypeFilter:
        {
            LILFilter * value = static_cast<LILFilter *>(node);
            return this->_emit(value);
        }
        case NodeTypeFlag:
        {
            LILFlag * value = static_cast<LILFlag *>(node);
            return this->_emit(value);
        }
        case NodeTypeFunctionDecl:
        {
            LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
            return this->_emit(value);
        }
        case NodeTypeFunctionCall:
        {
            LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
            return this->_emit(value);
        }
        case NodeTypeFlowControl:
        {
            LILFlowControl * value = static_cast<LILFlowControl *>(node);
            return this->_emit(value);
        }
        case NodeTypeFlowControlCall:
        {
            LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
            return this->_emit(value);
        }
        case NodeTypeInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            return this->_emit(value);
        }

        default:
        {
            std::cerr << "Error: unkonwn node type to emit\n";
            return nullptr;
        }
    }
}

llvm::Value * LILIREmitter::_emit(LILBoolLiteral * value)
{
    return llvm::ConstantInt::get(d->llvmContext, llvm::APInt(1, value->getValue(), false));
}

llvm::Value * LILIREmitter::_emit(LILNumberLiteral * value)
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

llvm::Value * LILIREmitter::_emit(LILPercentageLiteral * value)
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

llvm::Value * LILIREmitter::_emit(LILExpression * value)
{
    std::shared_ptr<LILNode> left = value->getLeft();
    std::shared_ptr<LILNode> right = value->getRight();

    llvm::Value * leftV = this->emit(left.get());
    llvm::Value * rightV = this->emit(right.get());
    if (!leftV || !rightV) {
        std::cerr << "!!!!!!!!!!LEFT OR RIGHT EMIT FAIL!!!!!!!!!!!!!!!!\n";
        return nullptr;
    }
    if (leftV->getType() != rightV->getType()) {
        std::cerr << "!!!!!!!!!!LEFT AND RIGHT TYPE DONT MATCH FAIL!!!!!!!!!!!!!!!!\n";
        return nullptr;
    }
    switch (value->getExpressionType()) {
        case ExpressionTypeSum:
        {
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            switch (leftV->getType()->getTypeID()) {
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
            
        default:
            std::cerr << "!!!!!!!!!!UNKNOWN EXPRESSION TYPE FAIL!!!!!!!!!!!!!!!!\n";
            return nullptr;
    }

    std::cerr << "!!!!!!!!!!EMIT EXPRESSION FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILStringLiteral * value)
{
    if (value->getIsCString()) {
        LILString stringLiteral = value->getValue();
        LILString stringWithoutQuotes = stringLiteral.stripQuotes();
        LILString stringEscaped = stringWithoutQuotes.replaceEscapes();
        auto str = stringEscaped.data();
        auto charType = llvm::IntegerType::get(d->llvmContext, 8);

        std::vector<llvm::Constant *> chars(str.length());
        for(unsigned int i = 0; i < str.size(); i++) {
            chars[i] = llvm::ConstantInt::get(charType, str[i]);
        }

        //add \0
        chars.push_back(llvm::ConstantInt::get(charType, 0));

        auto stringType = llvm::ArrayType::get(charType, chars.size());

        auto globalDeclaration = new llvm::GlobalVariable(*(d->llvmModule.get()), stringType, true, llvm::GlobalVariable::ExternalLinkage, nullptr, ".str");

        globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, chars));
        globalDeclaration->setConstant(true);
        globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
        globalDeclaration->setUnnamedAddr (llvm::GlobalValue::UnnamedAddr::Global);

        return llvm::ConstantExpr::getBitCast(globalDeclaration, charType->getPointerTo());
    }
    std::cerr << "!!!!!!!!!!EMIT STRING LITERAL FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILStringFunction * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILNullLiteral * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILVarDecl * value)
{
    auto name = value->getName().data();
    if (value->getIsExtern()) {
        return this->_emitFnSignature(name, std::static_pointer_cast<LILFunctionType>(value->getType()));
    } else {
        auto ty = value->getType();
        if (!ty->isA(TypeTypeFunction)) {
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
            d->currentAlloca = this->createEntryBlockAlloca(fun, name, this->llvmTypeFromLILType(ty.get()));
            d->namedValues[name] = d->currentAlloca;
        }

        auto initVals = value->getInitVals();
        for (auto initVal : initVals) {
            if (ty->isA(TypeTypeMultiple))
            {
                auto multiTy = std::static_pointer_cast<LILMultipleType>(ty);
                auto llvmValue = this->emitForMultipleType(initVal.get(), multiTy);
                if (llvmValue) {
                    d->irBuilder.CreateStore(llvmValue, d->currentAlloca);
                }
            }
            else
            {
                llvm::Value * llvmValue = this->emit(initVal.get());
                if (llvmValue) {
                    if (
                        !ty->isA(TypeTypePointer)
                        && initVal->getType()->isA(TypeTypePointer)
                        ) {
                        llvmValue = d->irBuilder.CreateLoad(llvmValue);
                    }
                    d->irBuilder.CreateStore(llvmValue, d->currentAlloca);
                }
            }
        }
    }
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILClassDecl * value)
{
    std::string name = value->getName().data();

    d->classTypes[name] = this->extractStructFromClass(value);

    for (auto methodVar : value->getMethods()) {
        auto vd = std::static_pointer_cast<LILVarDecl>(methodVar);
        for (auto method : vd->getInitVals()) {
            std::shared_ptr<LILFunctionDecl> fd = std::static_pointer_cast<LILFunctionDecl>(method);
            this->_emitMethod(fd.get(), value);
        }
    }
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILObjectDefinition * value)
{
    if (d->currentAlloca == nullptr) {
        std::cerr << "CURRENT ALLOCA WAS NULL FAIL !!!!!!!!\n\n";
        return nullptr;
    }
    LILString classNameStr = value->getType()->getName();
    std::string className = classNameStr.data();

    auto classValue = this->findClassWithName(classNameStr);
    size_t theIndex = 0;
    std::vector<std::shared_ptr<LILVarDecl>> callVars;
    std::vector<llvm::Value *> callValues;
    llvm::Value * alloca = d->currentAlloca;
    for (auto field : classValue->getFields()) {
        auto vd = std::static_pointer_cast<LILVarDecl>(field);
        
        bool needsCall = vd->getIsIVar() || vd->getIsVVar();
        if (needsCall) {
            callVars.push_back(vd);
        }
        
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
                    }

                    if (pn && pn->getName() == varName) {
                        theVal = asgmt->getValue();
                        break;
                    }
                }
            }
        }
        
        if (!theVal) {
            theVal = vd->getInitVal();
        }

        if (theVal) {
            if (needsCall) {
                callValues.push_back(this->emit(theVal.get()));
            } else {
                auto gep = this->_emitGEP(alloca, classValue->getName(), theIndex, varName, 0);
                if (vd->getType()->isA(TypeTypeObject)) {
                    d->currentAlloca = gep;
                    this->emit(theVal.get());
                } else {
                    llvm::Value * llvmValue = this->emit(theVal.get());
                    d->irBuilder.CreateStore(llvmValue, gep);
                }
            }
        }
        ++theIndex;
    }
    
    theIndex = 0;
    for (auto vd : callVars) {
        LILString name = vd->getName();
        LILString methodName = "set" + name.toUpperFirstCase();
        auto methodVd = classValue->getMethodNamed(methodName);
        
        LILString newName = this->decorate("", classValue->getName(), methodName, methodVd->getType());
        llvm::Function* fun = d->llvmModule->getFunction(newName.data());
        std::vector<llvm::Value *> argsvect;
        
        argsvect.push_back(d->currentAlloca);
        argsvect.push_back(callValues[theIndex]);
        
        d->irBuilder.CreateCall(fun, argsvect);
        
        ++theIndex;
    }

    //call the constructor
    if (classValue->getMethodNamed("construct")) {
        LILString decoratedName = this->decorate("", className, "construct", nullptr);
        llvm::Function* fun = d->llvmModule->getFunction(decoratedName.data());
        if (fun) {
            std::vector<llvm::Value *> argsvect;
            argsvect.push_back(alloca);
            d->irBuilder.CreateCall(fun, argsvect);
        }
    }

    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILAssignment * value)
{
    auto theValue = value->getValue();
    if (theValue) {
        llvm::Value * llvmValue = this->emit(value->getValue().get());
        auto subjectVal = value->getSubject();
        if (subjectVal->isA(NodeTypeVarName)) {
            auto vd = std::static_pointer_cast<LILVarName>(subjectVal);
            llvm::Value * llvmSubject = d->namedValues[vd->getName().data()];
            
            auto ty = value->getType();
            if (
                ty->isA(TypeTypePointer)
                && llvmValue->getType()->getTypeID() != llvm::Type::PointerTyID
                ){
                llvmSubject = d->irBuilder.CreateLoad(llvmSubject);
                return d->irBuilder.CreateStore(llvmValue, llvmSubject);
            } else {
                return d->irBuilder.CreateStore(llvmValue, llvmSubject);
            }
            
            return d->irBuilder.CreateStore(llvmValue, llvmSubject);
        }

        if (!subjectVal->isA(NodeTypeValuePath)) {
            std::cerr << "!!!!!!!!!!SUBJECT OF ASSIGNMENT WAS NOT VALUE PATH OR VAR NAME FAIL !!!!!!!!!!!!!!!!\n";
            return nullptr;
        }

        auto vp = std::static_pointer_cast<LILValuePath>(subjectVal);
        
        const auto & childNodes = vp->getNodes();
        auto it = childNodes.begin();
        auto firstNode = *it;
        bool isExtern = false;
        llvm::Value * llvmSubject = nullptr;
        std::shared_ptr<LILClassDecl> classDecl;
        LILString className;
        LILString stringRep;
        
        if (firstNode->isA(NodeTypeVarName)) {
            auto vn = std::static_pointer_cast<LILVarName>(firstNode);
            std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
            if (subjectNode && subjectNode->isA(NodeTypeVarDecl)) {
                auto vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
                className = vd->getName();
                llvmSubject = d->namedValues[className.data()];
                if (vd->getIsExtern()) {
                    isExtern = true;
                }
                stringRep = vn->getName();
                auto vdTy = vd->getType();
                if (!vdTy->isA(TypeTypeObject)) {
                    std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                    return nullptr;
                }
                classDecl = this->findClassWithName(vdTy->getName());
                if (!classDecl) {
                    std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                    return nullptr;
                }
            }
        } else {
            //selector
            auto sel = std::static_pointer_cast<LILSelector>(firstNode);
            switch (sel->getSelectorType()) {
                case SelectorTypeSelfSelector:
                {
                    llvmSubject = d->namedValues["@self"];
                    classDecl = this->findAncestorClass(value->shared_from_this());
                    stringRep = "@self";
                    break;
                }
                default:
                    std::cerr << "!!!!!!!!!!UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!\n";
                    return nullptr;
            }
        }
        
        ++it;
        
        while (it != childNodes.end()) {
            auto currentNode = *it;
            ++it;
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
                    
                    auto method = classDecl->getMethodNamed(fc->getName());
                    if (!method->isA(NodeTypeVarDecl)) {
                        std::cerr << "!!!!!!!!!!NODE IS NOT VAR DECL FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto vd = std::static_pointer_cast<LILVarDecl>(method);

                    auto fcTypes = fc->getArgumentTypes();
                    std::shared_ptr<LILFunctionDecl> targetFn;
                    if (vd->getInitVals().size() > 1 && fcTypes.size() > 0) {
                        targetFn = this->chooseFnByType(vd, fcTypes);
                    } else {
                        auto initVal = vd->getInitVal();
                        if (initVal && initVal->isA(NodeTypeFunctionDecl)) {
                            targetFn = std::static_pointer_cast<LILFunctionDecl>(vd->getInitVal());
                        }
                    }
                    auto ty = vd->getType();
                    if (!ty->isA(TypeTypeFunction)) {
                        std::cerr << "!!!!!!!!!!TYPE IS NOT FUNCTION TYPE FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                    if (targetFn) {
                        llvmSubject = this->_emitFunctionCall(fc.get(), targetFn->getName(), fnTy.get(), nullptr);
                        stringRep += "()";
                    }

                    auto retTy = fnTy->getReturnType();
                    if (!retTy->isA(TypeTypeObject)) {
                        std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    classDecl = this->findClassWithName(retTy->getName());
                    if (!classDecl) {
                        std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    break;
                }
                    
                case NodeTypePropertyName:
                {
                    if (llvmSubject == nullptr) {
                        std::cerr << "!!!!!!!!!!SUBJECT OF VALUE PATH WAS NULL!!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto pn = std::static_pointer_cast<LILPropertyName>(currentNode);
                    const auto & pnName = pn->getName();
                    stringRep += "." + pnName;
                    
                    
                    auto field = classDecl->getFieldNamed(pnName);
                    if (!field->isA(NodeTypeVarDecl)) {
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
                        auto methodVd = classDecl->getMethodNamed(methodName);
                        
                        LILString newName = this->decorate("", classDecl->getName(), methodName, methodVd->getType());
                        llvm::Function* fun = d->llvmModule->getFunction(newName.data());
                        std::vector<llvm::Value *> argsvect;
                        argsvect.push_back(llvmSubject);
                        if (isLastNode) {
                            argsvect.push_back(llvmValue);
                        }
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
                                if (!retTy->isA(TypeTypeObject)) {
                                    std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                                    return nullptr;
                                }
                                classDecl = this->findClassWithName(retTy->getName());
                                if (!classDecl) {
                                    std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                                    return nullptr;
                                }
                            }
                        } else {
                            std::cerr << "!!!!!!!!!!COULD NOT FIND METHOD FAIL!!!!!!!!!!!!!!!!\n";
                        }
                        
                    } else {
                        
                        if (isLastNode) {
                            auto ty = value->getType();
                            if (
                                ty->isA(TypeTypePointer)
                                && llvmValue->getType()->getTypeID() != llvm::Type::PointerTyID
                                ){
                                llvmSubject = d->irBuilder.CreateLoad(llvmSubject);
                                return d->irBuilder.CreateStore(llvmValue, llvmSubject);
                            } else {
                                return d->irBuilder.CreateStore(llvmValue, llvmSubject);
                            }
                        } else {
                            
                            //get index of field into struct
                            auto fields = classDecl->getFields();
                            size_t theIndex = 0;
                            std::shared_ptr<LILVarDecl> fieldVd;
                            for (size_t i=0, j=fields.size(); i<j; ++i) {
                                fieldVd = std::static_pointer_cast<LILVarDecl>(fields[i]);
                                if (fieldVd->getName() == pn->getName()) {
                                    theIndex = i;
                                    break;
                                }
                            }
                            if (!fieldVd) {
                                std::cerr << "!!!!!!!!!!FIELD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                                return nullptr;
                            }
                            
                            std::string name = pn->getName().data();
                            llvmSubject = this->_emitGEP(llvmSubject, className, theIndex, stringRep, 0);
                            auto ty = fieldVd->getType();
                            if (!ty->isA(TypeTypeFunction)) {
                                std::cerr << "!!!!!!!!!!TYPE IS NOT FUNCTION TYPE FAIL !!!!!!!!!!!!!!!!\n";
                                return nullptr;
                            }
                            auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                            auto retTy = fnTy->getReturnType();
                            if (!retTy->isA(TypeTypeObject)) {
                                std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                                return nullptr;
                            }
                            classDecl = this->findClassWithName(retTy->getName());
                            if (!classDecl) {
                                std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                                return nullptr;
                            }
                        }
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
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILValuePath * value)
{
    const auto & childNodes = value->getNodes();
    if (childNodes.size() == 1)
    {
        return this->emit(childNodes[0].get());
    }
    else if (childNodes.size() > 1)
    {
        auto it = childNodes.begin();
        auto firstNode = *it;
        bool isExtern = false;
        llvm::Value * llvmSubject = nullptr;
        std::shared_ptr<LILClassDecl> classDecl;
        LILString className;
        LILString instanceName;
        LILString stringRep;

        if (firstNode->isA(NodeTypeVarName)) {
            auto vn = std::static_pointer_cast<LILVarName>(firstNode);
            std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
            if (subjectNode && subjectNode->isA(NodeTypeVarDecl)) {
                auto vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
                instanceName = vd->getName();
                llvmSubject = d->namedValues[instanceName.data()];
                if (vd->getIsExtern()) {
                    isExtern = true;
                }
                stringRep = vn->getName();
                auto vdTy = vd->getType();
                if (!vdTy->isA(TypeTypeObject)) {
                    std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                    return nullptr;
                }
                className = vdTy->getName();
                classDecl = this->findClassWithName(className);
                if (!classDecl) {
                    std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                    return nullptr;
                }
            }
        } else {
            //selector
            auto sel = std::static_pointer_cast<LILSelector>(firstNode);
            switch (sel->getSelectorType()) {
                case SelectorTypeSelfSelector:
                {
                    auto ptrToSelf = d->namedValues["@self"];
                    llvmSubject = d->irBuilder.CreateLoad(ptrToSelf);
                    classDecl = this->findAncestorClass(value->shared_from_this());
                    className = classDecl->getName();
                    stringRep = "@self";
                    break;
                }
                default:
                    std::cerr << "!!!!!!!!!!UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!\n";
                    return nullptr;
            }
        }

        ++it;

        while (it != childNodes.end()) {
            auto currentNode = *it;
            ++it;
            bool isLastNode = it == childNodes.end();
            switch (currentNode->getNodeType()) {
                case NodeTypeFunctionCall:
                {
                    auto fc = std::static_pointer_cast<LILFunctionCall>(currentNode);

                    auto method = classDecl->getMethodNamed(fc->getName());
                    if (!method->isA(NodeTypeVarDecl)) {
                        std::cerr << "!!!!!!!!!!NODE IS NOT VAR DECL FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto vd = std::static_pointer_cast<LILVarDecl>(method);

                    auto fcTypes = fc->getArgumentTypes();
                    std::shared_ptr<LILFunctionDecl> targetFn;
                    if (vd->getInitVals().size() > 1 && fcTypes.size() > 0) {
                        targetFn = this->chooseFnByType(vd, fcTypes);
                    } else {
                        auto initVal = vd->getInitVal();
                        if (initVal && initVal->isA(NodeTypeFunctionDecl)) {
                            targetFn = std::static_pointer_cast<LILFunctionDecl>(vd->getInitVal());
                        }
                    }
                    auto ty = vd->getType();
                    if (!ty->isA(TypeTypeFunction)) {
                        std::cerr << "!!!!!!!!!!TYPE IS NOT FUNCTION TYPE FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                    if (targetFn) {
                        
                        llvmSubject = this->_emitFunctionCall(fc.get(), targetFn->getName(), fnTy.get(), llvmSubject);
                        stringRep += "()";
                    }
                    
                    if (isLastNode) {
                        return llvmSubject;
                    } else {
                        auto retTy = fnTy->getReturnType();
                        if (!retTy->isA(TypeTypeObject)) {
                            std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                            return nullptr;
                        }
                        className = retTy->getName();
                        classDecl = this->findClassWithName(className);
                        if (!classDecl) {
                            std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                            return nullptr;
                        }
                    }
                    break;
                }

                case NodeTypePropertyName:
                {
                    if (llvmSubject == nullptr) {
                        std::cerr << "!!!!!!!!!!SUBJECT OF VALUE PATH WAS NULL!!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto pn = std::static_pointer_cast<LILPropertyName>(currentNode);
                    const auto & pnName = pn->getName();
                    stringRep += "." + pnName;

                    auto field = classDecl->getFieldNamed(pnName);
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

                        LILString newName = this->decorate("", classDecl->getName(), methodName, methodVd->getType());
                        llvm::Function* fun = d->llvmModule->getFunction(newName.data());
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
                                if (!retTy->isA(TypeTypeObject)) {
                                    std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                                    return nullptr;
                                }
                                className = retTy->getName();
                                classDecl = this->findClassWithName(className);
                                if (!classDecl) {
                                    std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                                    return nullptr;
                                }
                            }
                        } else {
                            std::cerr << "!!!!!!!!!!COULD NOT FIND GETTER FAIL!!!!!!!!!!!!!!!!\n";
                        }

                    } else {
                        
                        //get index of field into struct
                        auto fields = classDecl->getFields();
                        size_t theIndex = 0;
                        std::shared_ptr<LILVarDecl> fieldVd;
                        for (size_t i=0, j=fields.size(); i<j; ++i) {
                            fieldVd = std::static_pointer_cast<LILVarDecl>(fields[i]);
                            if (fieldVd->getName() == pn->getName()) {
                                theIndex = i;
                                break;
                            }
                        }
                        if (!fieldVd) {
                            std::cerr << "!!!!!!!!!!FIELD NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                            return nullptr;
                        }

                        std::string name = pn->getName().data();
                        llvmSubject = this->_emitGEP(llvmSubject, className, theIndex, stringRep, 0);
                        if (isLastNode) {
                            return d->irBuilder.CreateLoad(llvmSubject, name.data());
                        } else {
                            auto ty = fieldVd->getType();
                            if (!ty->isA(TypeTypeFunction)) {
                                std::cerr << "!!!!!!!!!!TYPE IS NOT FUNCTION TYPE FAIL !!!!!!!!!!!!!!!!\n";
                                return nullptr;
                            }
                            auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                            auto retTy = fnTy->getReturnType();
                            if (!retTy->isA(TypeTypeObject)) {
                                std::cerr << "!!!!!!!!!!NODE DOES NOT POINT TO OBJECT FAIL !!!!!!!!!!!!!!!!\n";
                                return nullptr;
                            }
                            className = retTy->getName();
                            classDecl = this->findClassWithName(className);
                            if (!classDecl) {
                                std::cerr << "!!!!!!!!!!CLASS NOT FOUND FAIL !!!!!!!!!!!!!!!!\n";
                                return nullptr;
                            }
                        }
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

llvm::Value * LILIREmitter::_emitGEP(llvm::Value * llvmValue, LILString className, LILUnitI32 fieldIndex, LILString fieldName, LILUnitI32 arrayIndex)
{
    auto name = className.data();
    if (d->classTypes.count(name)) {
        return this->_emitGEP(llvmValue, d->classTypes[name], fieldIndex, fieldName, arrayIndex);
    }
    return nullptr;
}

llvm::Value * LILIREmitter::_emitGEP(llvm::Value * llvmValue, llvm::Type * llvmType, LILUnitI32 fieldIndex, LILString fieldName, LILUnitI32 arrayIndex)
{
    std::vector<llvm::Value *> idList;
    idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
    idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, fieldIndex, false)));
    return llvm::GetElementPtrInst::Create(llvmType, llvmValue, idList, fieldName.data(), d->irBuilder.GetInsertBlock());
}

llvm::Value * LILIREmitter::_emit(LILPropertyName * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILRule * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILSimpleSelector * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILSelectorChain * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILSelector * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILCombinator * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILFilter * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILFlag * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILVarName * value)
{
    LILString name = value->getName();
    auto namestr = name.data();
    llvm::Value * val = d->namedValues[namestr];
    if (!val) {
        std::cerr << "!!!!!!!!!!UNKNOWN VARIABLE OMG ALKJDLFJA FAIL FAIL FAIL!!!!!!!!!!!!!!!!\n";
        return nullptr;
    }
    return d->irBuilder.CreateLoad(val, namestr);
}

llvm::Function * LILIREmitter::_emit(LILFunctionDecl * value)
{
    switch (value->getFunctionDeclType()) {
        case FunctionDeclTypeFn:
        {
            return this->_emitFn(value);
        }

        default:
            std::cerr << "!!!!!!!!!!UNKNOWN FUNCTION DECL TYPE FAIL!!!!!!!!!!!!!!!!\n";
            break;
    }
    return nullptr;
}

llvm::Function * LILIREmitter::_emitFnSignature(std::string name, const std::shared_ptr<LILFunctionType> fnTy)
{
    std::vector<llvm::Type*> types;
    auto arguments = fnTy->getArguments();
    for (auto & arg : arguments) {
        llvm::Type * llvmTy;
        std::shared_ptr<LILType> ty;
        if (arg->isA(NodeTypeType)) {
            ty = std::static_pointer_cast<LILType>(arg);
        } else if (arg->isA(NodeTypeVarDecl)){
            ty = std::static_pointer_cast<LILVarDecl>(arg)->getType();
        }
        if (ty) {
            llvmTy = this->llvmTypeFromLILType(ty.get());
        }
        if (llvmTy) {
            types.push_back(llvmTy);
        } else {
            std::cerr << "!!!!!!!!!!EMIT FN SIGNATURE FAIL!!!!!!!!!!!!!!!!\n";
        }
    }
    return this->_emitFnSignature(name, types, fnTy);
}

llvm::Function * LILIREmitter::_emitFnSignature(std::string name, std::vector<llvm::Type*> types, const std::shared_ptr<LILFunctionType> fnTy)
{
    std::shared_ptr<LILType> retTy = fnTy->getReturnType();
    llvm::Type * returnType;
    if (retTy) {
        returnType = this->llvmTypeFromLILType(retTy.get());
    }
    if (!returnType) {
        returnType = llvm::Type::getVoidTy(d->llvmContext);
    }

    llvm::FunctionType * ft = llvm::FunctionType::get(returnType, types, fnTy->getIsVariadic());

    llvm::Function * fun = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, d->llvmModule.get());
    return fun;
}

llvm::Function * LILIREmitter::_emitFn(LILFunctionDecl * value)
{
    auto fnTy = std::static_pointer_cast<LILFunctionType>(value->getType());
    auto arguments = fnTy->getArguments();
    llvm::Function * fun = this->_emitFnSignature(value->getName().data(), fnTy);

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

    for (llvm::Value & arg : fun->args()) {
        llvm::AllocaInst * alloca = this->createEntryBlockAlloca(fun, arg.getName(), arg.getType());
        d->irBuilder.CreateStore(&arg, alloca);
        auto name = arg.getName();
        if (d->namedValues.count(name)) {
            d->hiddenLocals.back()[name] = d->namedValues[name];
        }
        d->namedValues[name] = alloca;
    }

    this->_emitFnBody(fun, value);

    //clear args from local values
    for (llvm::Value & arg : fun->args()) {
        d->namedValues.erase(arg.getName());
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
    this->_emitEvaluables(body);

    d->finallyBB = llvm::BasicBlock::Create(d->llvmContext, "finally", fun);
    d->irBuilder.CreateBr(d->finallyBB);
    d->irBuilder.SetInsertPoint(d->finallyBB);

    auto finally = value->getFinally();
    if (finally) {
        this->emit(finally.get());
    }

    if (d->needsReturnValue) {
        llvm::Value * loadInstr = d->irBuilder.CreateLoad(d->returnAlloca);
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

llvm::Function * LILIREmitter::_emitMethod(LILFunctionDecl * value, LILClassDecl * classValue)
{
    auto fnTy = std::static_pointer_cast<LILFunctionType>(value->getType());
    auto arguments = fnTy->getArguments();

    std::vector<llvm::Type*> types;

    std::string className = classValue->getName().data();
    if (d->classTypes.count(className) == 0) {
        return nullptr;
    }
    auto classType = d->classTypes[className];
    llvm::Type * classPtrType = llvm::PointerType::get(classType, 0);

    types.push_back(classPtrType);

    llvm::Type * llvmTy;
    std::shared_ptr<LILType> ty;
    for (auto & arg : arguments) {
        llvmTy = nullptr;
        ty.reset();
        if (arg->isA(NodeTypeType)) {
            ty = std::static_pointer_cast<LILType>(arg);
        } else if (arg->isA(NodeTypeVarDecl)){
            ty = std::static_pointer_cast<LILVarDecl>(arg)->getType();
        }
        if (ty) {
            llvmTy = this->llvmTypeFromLILType(ty.get());
        }
        if (llvmTy) {
            types.push_back(llvmTy);
        } else {
            std::cerr << "!!!!!!!!!!EMIT METHOD FAIL!!!!!!!!!!!!!!!!\n";
        }
    }
    llvm::Function * fun = this->_emitFnSignature(value->getName().data(), types, fnTy);

    size_t argIndex = 0;
    for (auto & llvmArg : fun->args()) {
        if (argIndex == 0){
            llvmArg.setName("@self");

        } else {
            auto arg = arguments[argIndex-1];
            if(!arg || !arg->isA(NodeTypeVarDecl)){
                std::cerr << "!!!!!!!!!!NODE WAS NOT VAR DECL FAIL!!!!!!!!!!!!!!!!\n";
                return nullptr;
            }

            std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(arg);
            llvmArg.setName(vd->getName().data());
        }

        ++argIndex;
    }

    llvm::BasicBlock * bb = llvm::BasicBlock::Create(d->llvmContext, "entry", fun);
    d->irBuilder.SetInsertPoint(bb);

    std::map<std::string, llvm::Value *> scope;
    d->hiddenLocals.push_back(scope);

    for (llvm::Value & arg : fun->args()) {
        if (arg.getType()->getTypeID() == llvm::Type::TypeID::PointerTyID) {
            d->namedValues[arg.getName()] = &arg;
        } else {
            llvm::AllocaInst * alloca = this->createEntryBlockAlloca(fun, arg.getName(), arg.getType());
            d->irBuilder.CreateStore(&arg, alloca);
            auto name = arg.getName();
            if (d->namedValues.count(name)) {
                d->hiddenLocals.back()[name] = d->namedValues[name];
            }
            d->namedValues[name] = alloca;
        }
    }

//    if (value->getIsConstructor()) {
//        size_t theIndex = 0;
//        for (auto field : classValue->getFields()) {
//            auto vd = std::static_pointer_cast<LILVarDecl>(field);
//            auto initVal = vd->getInitVal();
//            if (initVal) {
//                auto varName = vd->getName();
//                auto subject = d->namedValues["@self"];
//                auto gep = this->_emitGEP(subject, classValue->getName(), theIndex, varName, 0);
//                llvm::Value * llvmValue = this->emit(initVal.get());
//
//                d->irBuilder.CreateStore(llvmValue, gep);
//            }
//            ++theIndex;
//        }
//    }

    this->_emitFnBody(fun, value);

    const std::map<std::string, llvm::Value *> & hiddenLocals = d->hiddenLocals.back();
    for (auto it = hiddenLocals.begin(); it != hiddenLocals.end(); ++it) {
        d->namedValues[it->first] = it->second;
    }
    d->hiddenLocals.pop_back();
    return fun;
}

llvm::Value * LILIREmitter::_emit(LILFunctionCall * value)
{
    switch (value->getFunctionCallType()) {
        case FunctionCallTypeNone:
        {
            LILString name = value->getName();
            auto node = this->findNodeForName(name, value->getParentNode().get());
            if (!node || !node->isA(NodeTypeVarDecl)) {
                std::cerr << "!!!!!!!!!!!!!!!!FUNCTION NOT FOUND FAIL\n\n";
                break;
            }
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            auto ty = node->getType();
            if (!ty) {
                std::cerr << "!!!!!!!!!!!!!!!!NODE HAD NO TYPE FAIL\n\n";
                break;
            }
            if (!ty->isA(TypeTypeFunction)) {
                std::cerr << "TYPE IS NOT FUNCTION TYPE FAIL !!!!!!!!!!!!!!!!\n\n";
                return nullptr;
            }
            auto initVals = vd->getInitVals();
            auto fcTypes = value->getArgumentTypes();
            if (initVals.size() > 1) {
                llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
                size_t i = 0;
                bool hasMultipleType = false;
                for (auto fcTy : fcTypes) {
                    if (fcTy->isA(TypeTypeMultiple)) {
                        hasMultipleType = true;
                        break;
                    }
                }
                if (hasMultipleType) {
                    for (auto fcTy : fcTypes) {
                        if (fcTy->isA(TypeTypeMultiple)) {
                            auto multiTy = std::static_pointer_cast<LILMultipleType>(fcTy);
                            auto arguments = value->getArguments();
                            auto argument = arguments[i];
                            
                            auto llvmIr = this->emitPointer(argument.get());
                            
                            std::vector<llvm::Value *> gepIndices1;
                            gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
                            gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
                            auto gep = d->irBuilder.CreateGEP(llvmIr, gepIndices1);
                            
                            auto argVal = d->irBuilder.CreateLoad(gep, "_lil_type_index");
                            
                            llvm::BasicBlock * defaultBB = llvm::BasicBlock::Create(d->llvmContext, "case.null");
                            llvm::SwitchInst * switchInstr = d->irBuilder.CreateSwitch(argVal, defaultBB);
                            llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, "switch.merge");
                            
                            fun->getBasicBlockList().push_back(defaultBB);
                            d->irBuilder.SetInsertPoint(defaultBB);
                            d->irBuilder.CreateBr(mergeBB);
                            
                            auto mfcTys = multiTy->getTypes();
                            size_t j = 1;
                            for (auto mfcTy : mfcTys) {
                                llvm::BasicBlock * bb = llvm::BasicBlock::Create(d->llvmContext, "case."+mfcTy->getName().data(), fun);
                                switchInstr->addCase(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, j, false)), bb);
                                std::vector<std::shared_ptr<LILType>> types;
                                for (size_t k=0; k<fcTypes.size(); ++k) {
                                    if (k == i) {
                                        types.push_back(mfcTy);
                                    } else {
                                        types.push_back(fcTy);
                                    }
                                }
                                auto fd = this->chooseFnByType(vd, types);
                                d->irBuilder.SetInsertPoint(bb);
                                this->_emitFunctionCallMT(value, fd->getName(), types, fd->getFnType().get(), nullptr);
                                d->irBuilder.CreateBr(mergeBB);
                                j += 1;
                            }
                            
                            fun->getBasicBlockList().push_back(mergeBB);
                            d->irBuilder.SetInsertPoint(mergeBB);
                        }
                        i += 1;
                    }
                } else {
                    auto fd = this->chooseFnByType(vd, fcTypes);
                    auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                    return this->_emitFunctionCall(value, fd->getName(), fnTy.get(), nullptr);
                    
                }
            } else {
                auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                return this->_emitFunctionCall(value, value->getName(), fnTy.get(), nullptr);
            }
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
            return d->irBuilder.CreateLoad(llvmValue);
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
        default:
        {
            std::cerr << "UNIMPLEMENTED FUNCTION CALL TYPE FAIL\n";
            break;
        }
    }

    return nullptr;

}

llvm::Value * LILIREmitter::_emitFunctionCall(LILFunctionCall * value, LILString name, LILFunctionType * fnTy, llvm::Value * instance)
{
    llvm::Function* fun = d->llvmModule->getFunction(name.data());
    auto fcArgs = value->getArguments();
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
                fcArgIr = this->emit(fcArg.get());
            }
            else
            {
                auto declArg = declArgs[i];
                auto declArgTy = declArg->getType();
                
                
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
                        && !fcValue->getType()->isA(TypeTypePointer)
                        ) {
                        fcArgIr = this->emitPointer(fcValue.get());
                    } else {
                        fcArgIr = this->emit(fcValue.get());
                    }
                }
                if (
                    declArgTy
                    && !declArgTy->isA(TypeTypePointer)
                    && fcValue->getType()->isA(TypeTypePointer)
                    ) {
                    fcArgIr = d->irBuilder.CreateLoad(fcArgIr);
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

llvm::Value * LILIREmitter::_emitFunctionCallMT(LILFunctionCall *value, LILString name, std::vector<std::shared_ptr<LILType> > types, LILFunctionType * fnTy, llvm::Value * instance)
{
    llvm::Function* fun = d->llvmModule->getFunction(name.data());
    auto fcArgs = value->getArguments();
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
//                    fcArgIr = d->irBuilder.CreateLoad(fcArgIr);
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

llvm::Value * LILIREmitter::_emit(LILFlowControl * value)
{
    switch (value->getFlowControlType()) {
        case FlowControlTypeIf:
        {
            return this->_emitIf(value);
        }
        case FlowControlTypeIfIs:
        {
            return this->_emitIfIs(value);
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

    for (auto & node : value->getThen()) {
        bool breakAfter = false;
        if (node->isA(FlowControlCallTypeReturn)) {
            breakAfter = true;;
        }
        this->emit(node.get());
        if (breakAfter) {
            break;
        }
    }

    d->irBuilder.CreateBr(mergeBB);

    fun->getBasicBlockList().push_back(elseBB);
    d->irBuilder.SetInsertPoint(elseBB);

    for (auto & node : value->getElse()) {
        bool breakAfter = false;
        if (node->isA(FlowControlCallTypeReturn)) {
            breakAfter = true;;
        }
        this->emit(node.get());
        if (breakAfter) {
            break;
        }
    }

    d->irBuilder.CreateBr(mergeBB);

    fun->getBasicBlockList().push_back(mergeBB);
    d->irBuilder.SetInsertPoint(mergeBB);
    return nullptr;
}

llvm::Value * LILIREmitter::_emitIfIs(LILFlowControl * value)
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

    std::shared_ptr<LILType> ty;
    std::shared_ptr<LILVarDecl> subject;
    if (firstArg->isA(NodeTypeVarName)) {
        auto vn = std::static_pointer_cast<LILVarName>(firstArg);
        auto subjectNode = this->findNodeForVarName(vn.get());
        if (!subjectNode || !subjectNode->isA(NodeTypeVarDecl)) {
            std::cerr << "NODE NOT FOUND FAIL!!!!!!!!!!!!!!!!\n\n";
            return nullptr;
        }
        subject = std::static_pointer_cast<LILVarDecl>(subjectNode);
        ty = subject->getType();
        if (ty->isA(TypeTypeMultiple)) {
            std::cerr << "UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n\n";
            return nullptr;
        }
        
    } else {
        std::cerr << "UNKNOWN FIRST ARG FAIL!!!!!!!!!!!!!!!!\n\n";
        return nullptr;
    }

    if (ty->getIsNullable()) {
        auto lastArgTy = std::static_pointer_cast<LILType>(lastArg);
        
        if (lastArgTy->getName() == "bool")
        {
            llvm::Value * firstArgIr = this->emit(firstArg.get());
            condition = d->irBuilder.CreateICmpULT(firstArgIr, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(2, 2, false)), "if.cond");
        }
        else if (lastArgTy->getName() == "cstr")
        {
            llvm::Value * firstArgIr = this->emit(firstArg.get());
            auto temp = d->irBuilder.CreatePtrToInt(firstArgIr, llvm::Type::getInt64Ty(d->llvmContext));
            condition = d->irBuilder.CreateICmpNE(temp, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 0, false)), "if.cond");
        }
        else if (
            lastArgTy->getName() == "i8"
            || lastArgTy->getName() == "i16"
            || lastArgTy->getName() == "i32"
            || lastArgTy->getName() == "i64"
            || lastArgTy->getName() == "i128"
            || lastArgTy->getName() == "f32"
            || lastArgTy->getName() == "f64"
        )
        {
            auto llvmSubject = this->emitPointer(firstArg.get());
            std::vector<llvm::Value *> idList;
            idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
            idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
            auto temp = llvm::GetElementPtrInst::Create(this->llvmTypeFromLILType(ty.get()), llvmSubject, idList, "isNull", d->irBuilder.GetInsertBlock());
            auto temp2 = d->irBuilder.CreateLoad(temp);
            condition = d->irBuilder.CreateICmpNE(temp2, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(1, 1, false)), "if.cond");
        }
        else if (lastArgTy->isA(TypeTypePointer))
        {
            llvm::Value * firstArgIr = this->emit(firstArg.get());
            auto temp = d->irBuilder.CreatePtrToInt(firstArgIr, llvm::Type::getInt64Ty(d->llvmContext));
            condition = d->irBuilder.CreateICmpNE(temp, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 0, false)), "if.cond");
        }
        else if (lastArgTy->isA(TypeTypeObject))
        {
            auto llvmSubject = this->emitPointer(firstArg.get());
            std::vector<llvm::Value *> idList;
            idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
            idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
            auto temp = llvm::GetElementPtrInst::Create(this->llvmTypeFromLILType(ty.get()), llvmSubject, idList, "isNull", d->irBuilder.GetInsertBlock());
            auto temp2 = d->irBuilder.CreateLoad(temp);
            condition = d->irBuilder.CreateICmpNE(temp2, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(1, 1, false)), "if.cond");
        }
        else
        {
            std::cerr << "UNKNOWN LAST ARG FAIL!!!!!!!!!!!!!!!!\n\n";
            return nullptr;
        }
    }

    llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
    llvm::BasicBlock * bodyBB = llvm::BasicBlock::Create(d->llvmContext, "if.true", fun);
    llvm::BasicBlock * elseBB = llvm::BasicBlock::Create(d->llvmContext, "if.false");
    llvm::BasicBlock * mergeBB = llvm::BasicBlock::Create(d->llvmContext, "if.end");
    
    d->irBuilder.CreateCondBr(condition, bodyBB, elseBB);
    
    //hide the nullable var
    auto newTy = subject->getType()->clone();
    newTy->setIsNullable(false);
    auto newVar = subject->clone();
    newVar->setType(newTy);
    auto name = subject->getName();
    auto nameData = name.data();
    value->setLocalVariable(name, newVar);

    auto currentLlvmValue = d->namedValues[nameData];
    
    std::vector<llvm::Value *> gepIndices;
    gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
    gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
    d->namedValues[nameData] = llvm::GetElementPtrInst::Create(this->llvmTypeFromLILType(ty.get()), currentLlvmValue, gepIndices, nameData, d->irBuilder.GetInsertBlock());

    //configure the body of the if
    d->irBuilder.SetInsertPoint(bodyBB);
    
    for (auto & node : value->getThen()) {
        bool breakAfter = false;
        if (node->isA(FlowControlCallTypeReturn)) {
            breakAfter = true;;
        }
        this->emit(node.get());
        if (breakAfter) {
            break;
        }
    }
    
    d->irBuilder.CreateBr(mergeBB);
    
    d->namedValues[nameData] = currentLlvmValue;
    value->unsetLocalVariable(name);
    
    fun->getBasicBlockList().push_back(elseBB);
    d->irBuilder.SetInsertPoint(elseBB);
    
    for (auto & node : value->getElse()) {
        bool breakAfter = false;
        if (node->isA(FlowControlCallTypeReturn)) {
            breakAfter = true;;
        }
        this->emit(node.get());
        if (breakAfter) {
            break;
        }
    }
    
    d->irBuilder.CreateBr(mergeBB);
    
    fun->getBasicBlockList().push_back(mergeBB);
    d->irBuilder.SetInsertPoint(mergeBB);
    return nullptr;
}

llvm::Value * LILIREmitter::_emitFor(LILFlowControl * value)
{
    auto arguments = value->getArguments();
    auto initial = arguments.front();
    this->emit(initial.get());
    
    auto currentBB = d->irBuilder.GetInsertBlock();
    auto fun = currentBB->getParent();
    auto loopBB = llvm::BasicBlock::Create(d->llvmContext, "loop", fun);
    d->irBuilder.CreateBr(loopBB);
    d->irBuilder.SetInsertPoint(loopBB);
    this->_emitEvaluables(value->getThen());
    
    auto stepNode = arguments[2];
    if (!stepNode) {
        return nullptr;
    }
    this->emit(stepNode.get());
    
    auto condNode = arguments[1];
    if (!condNode) {
        return nullptr;
    }
    auto condition = this->emit(condNode.get());
    if (!condNode->isA(NodeTypeExpression)) {
        switch (condition->getType()->getTypeID()) {
            case llvm::Type::IntegerTyID:
            {
                auto bitWidth = condition->getType()->getIntegerBitWidth();
                if (bitWidth == 1){
                    condition = d->irBuilder.CreateICmpNE(condition, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(bitWidth, 0, true)), "for.cond");
                } else {
                    condition = d->irBuilder.CreateICmpSGT(condition, llvm::ConstantInt::get(d->llvmContext, llvm::APInt(bitWidth, 0, true)), "for.cond");
                }
            }
                break;
            case llvm::Type::FloatTyID:
                condition = d->irBuilder.CreateFCmpONE(condition, llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(0.0)), "for.cond");
                break;
                
            default:
                break;
        }
    }
    
    auto afterBB = llvm::BasicBlock::Create(d->llvmContext, "for.after", fun);
    d->irBuilder.CreateCondBr(condition, loopBB, afterBB);
    
    d->irBuilder.SetInsertPoint(afterBB);

    return nullptr;
}

llvm::Value * LILIREmitter::_emitLoop(LILFlowControl * value)
{
    auto currentBB = d->irBuilder.GetInsertBlock();
    auto fun = currentBB->getParent();
    auto loopBB = llvm::BasicBlock::Create(d->llvmContext, "loop", fun);
    d->irBuilder.CreateBr(loopBB);
    d->irBuilder.SetInsertPoint(loopBB);
    
    auto condVd = std::make_shared<LILVarDecl>();
    LILString condName("_lil_loop_repeat");
    condVd->setName(condName);
    auto boolVal = std::make_shared<LILBoolLiteral>();
    boolVal->setValue(false);
    auto boolTy = std::make_shared<LILType>();
    boolTy->setName("bool");
    condVd->setType(boolTy);
    condVd->setInitVal(boolVal);
    this->emit(condVd.get());
    
    this->_emitEvaluables(value->getThen());
    
    auto exp = std::make_shared<LILExpression>();
    exp->setExpressionType(ExpressionTypeEqualComparison);
    auto leftVn = std::make_shared<LILVarName>();
    leftVn->setName(condName);
    exp->setLeft(leftVn);
    auto rightVal = std::make_shared<LILBoolLiteral>();
    rightVal->setValue(true);
    exp->setRight(rightVal);
    auto condition = this->emit(exp.get());
    
    auto afterBB = llvm::BasicBlock::Create(d->llvmContext, "for.after", fun);
    d->irBuilder.CreateCondBr(condition, loopBB, afterBB);
    d->irBuilder.SetInsertPoint(afterBB);
    
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILFlowControlCall * value)
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
    if (arg) {
        d->currentAlloca = d->returnAlloca;
        d->needsReturnValue = true;

        auto ty = value->getType();
        llvm::Value * retVal;
        if (ty->isA(TypeTypeMultiple)) {
            retVal = this->emitForMultipleType(arg.get(), std::static_pointer_cast<LILMultipleType>(ty));
            d->currentAlloca = nullptr;
        } else {
            retVal = this->emit(arg.get());
        }
        if (retVal) {
            llvm::Value * theReturn = d->irBuilder.CreateStore(retVal, d->returnAlloca);
            return theReturn;
        }
    }
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
    asgmt->setValue(boolVal);
    auto ty = std::make_shared<LILType>();
    ty->setName("bool");
    asgmt->setType(ty);
    return this->emit(asgmt.get());
}

llvm::Value * LILIREmitter::_emit(LILInstruction * value)
{
    std::cerr << "!!!!!!!!!!UNIMPLEMENTED FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
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
            break;
        }
        case NodeTypeVarName:
        {
            auto vn = static_cast<LILVarName *>(node);
            LILString name = vn->getName();
            llvm::Value * val = d->namedValues[name.data()];
            if (!val) {
                std::cerr << "!!!!!!!!!!UNKNOWN VARIABLE FAIL!!!!!!!!!!!!!!!!\n";
                return nullptr;
            }
            return val;
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
                llvm::Value * val = d->namedValues[name.data()];
                if (!val) {
                    std::cerr << "!!!!!!!!!!UNKNOWN VARIABLE FAIL!!!!!!!!!!!!!!!!\n";
                    return nullptr;
                }
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
        bool isExtern = false;
        std::shared_ptr<LILVarDecl> vd;
        llvm::Value * llvmSubject = nullptr;
        LILString className;
        LILString stringRep;
        
        if (firstNode->isA(NodeTypeVarName)) {
            auto vn = std::static_pointer_cast<LILVarName>(firstNode);
            stringRep = vn->getName();
            std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
            if (subjectNode && subjectNode->isA(NodeTypeVarDecl)) {
                vd = std::static_pointer_cast<LILVarDecl>(subjectNode);
                className = vd->getName();
                llvmSubject = d->namedValues[className.data()];
                if (vd->getIsExtern()) {
                    isExtern = true;
                }
            }
        } else {
            //selector
            auto sel = std::static_pointer_cast<LILSelector>(firstNode);
            switch (sel->getSelectorType()) {
                case SelectorTypeSelfSelector:
                {
                    llvmSubject = d->namedValues["@self"];
                    stringRep = "@self";
                    break;
                }
                default:
                    std::cerr << "!!!!!!!!!!UNKNOWN SELECTOR TYPE FAIL!!!!!!!!!!!!!!!!\n";
                    return nullptr;
            }
        }
        
        ++it;
        
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
                    auto ty = vd->getType();
                    if (!ty->isA(TypeTypeObject)) {
                        continue;
                    }
                    auto objTy = std::static_pointer_cast<LILObjectType>(ty);
                    auto classValue = this->findClassWithName(objTy->getName());
                    auto fields = classValue->getFields();
                    size_t theIndex = 0;
                    for (size_t i=0, j=fields.size(); i<j; ++i) {
                        auto fieldVd = std::static_pointer_cast<LILVarDecl>(fields[i]);
                        if (fieldVd->getName() == pn->getName()) {
                            theIndex = i;
                            vd = fieldVd;
                            break;
                        }
                    }
                    
                    std::string name = pn->getName().data();
                    llvmSubject = this->_emitGEP(llvmSubject, className, theIndex, stringRep, 0);
                    break;
                }
                    
                default:
                    break;
            }
            
            ++it;
        }
        return llvmSubject;
    }
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

void LILIREmitter::printIR(llvm::raw_ostream & file) const
{
    d->llvmModule->print(file, nullptr);
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
        auto classTy = d->classTypes[className];
        if (!classTy) {
            auto classDecl = this->findClassWithName(className);
            if (!classDecl) {
                std::cerr << "CLASS NOT FOUND FAIL!!!!!!!!!!!!!!!!\n";
                return nullptr;
            }
            auto newClassTy = this->extractStructFromClass(classDecl.get());
            d->classTypes[className] = newClassTy;
            classTy = newClassTy;
        }
        if (type->getIsNullable()) {
            std::vector<llvm::Type*> types;
            types.push_back(classTy);
            types.push_back(llvm::Type::getInt1Ty(d->llvmContext));
            return llvm::StructType::get(d->llvmContext, types);
        } else {
            return classTy;
        }
    }
    LILString typestr = type->getName();
    llvm::Type * ret = nullptr;
    if (typestr == "bool") {
        if (type->getIsNullable()) {
            return llvm::IntegerType::get(d->llvmContext, 2);
        } else {
            return llvm::Type::getInt1Ty(d->llvmContext);
        }
    } else if (typestr == "i8" || typestr == "i8%"){
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
    } else if (typestr == "cstr"){
        auto charType = llvm::IntegerType::get(d->llvmContext, 8);
        return charType->getPointerTo();
    } else if (typestr == "null") {
        return llvm::Type::getVoidTy(d->llvmContext);
    }
    //if we get here it's a number type
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

std::shared_ptr<LILFunctionDecl> LILIREmitter::chooseFnByType(std::shared_ptr<LILVarDecl> vd, std::vector<std::shared_ptr<LILType>> types)
{
    for (auto fdNode : vd->getInitVals()) {
        if (fdNode->isA(NodeTypeFunctionDecl)) {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(fdNode);
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
            for (size_t i=0, j=fdTys.size(); i<j; ++i) {
                ty.reset();
                auto arg = fdTys[i];
                if (arg->isA(NodeTypeType)) {
                    ty = std::static_pointer_cast<LILType>(arg);
                } else if (arg->isA(NodeTypeVarDecl)) {
                    ty = std::static_pointer_cast<LILVarDecl>(arg)->getType();
                }
                if (ty) {
                    auto resultTy = LILType::merge(ty, types[i]);
                    if (resultTy && resultTy->equalTo(ty)) {
                        return fd;
                    }
                }
            }
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
    return llvm::StructType::create(d->llvmContext, types, value->getName().data());
}

llvm::Value * LILIREmitter::emitNullable(LILNode * node, LILType * targetTy)
{
    if (targetTy->isA(TypeTypePointer)) {
        switch (node->getNodeType()) {
            case NodeTypeVarName:
            case NodeTypeValuePath:
            case NodeTypeFunctionCall:
            {
                return this->emit(node);
            }
            case NodeTypeNull:
            {
                auto ptrTy = static_cast<LILPointerType *>(targetTy);
                auto argTy = ptrTy->getArgument();
                auto argLlvmTy = this->llvmTypeFromLILType(argTy.get());
                return llvm::ConstantPointerNull::get(argLlvmTy->getPointerTo());
            }
            default:
                std::cerr << "UNKNOWN NODE TYPE FAIL!!!!!!!!!!!!!!!!\n\n";
                return nullptr;
        }
    }
    if (targetTy->isA(TypeTypeObject)) {
        llvm::Value * ir = nullptr;
        //nullable numbers are structs with two members
        auto nullableObjAlloca = d->irBuilder.CreateAlloca(this->llvmTypeFromLILType(targetTy));

        std::vector<llvm::Value *> gepIndices1;
        gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
        gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
        d->currentAlloca = d->irBuilder.CreateGEP(nullableObjAlloca, gepIndices1);
        
        //if it is null we just ignore the first member
        if (!node->isA(NodeTypeNull)) {
            ir = this->emit(node);
        }

        //if we need to store something
        if (ir) {
            d->irBuilder.CreateStore(ir, d->currentAlloca);
        }
        //we're done
        d->currentAlloca = nullptr;

        //is null member
        auto nullBit = llvm::ConstantInt::get(
                                              d->llvmContext,
                                              llvm::APInt(
                                                          1,
                                                          (node->isA(NodeTypeNull) ? 1 : 0),
                                                          false
                                                          )
                                              );
        std::vector<llvm::Value *> gepIndices2;
        gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
        gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
        auto member2 = d->irBuilder.CreateGEP(nullableObjAlloca, gepIndices2);
        d->irBuilder.CreateStore(nullBit, member2);

        auto loadInstr = d->irBuilder.CreateLoad(nullableObjAlloca);
        return loadInstr;
        
    }
    LILString name = targetTy->getName();
    if (name == "bool") {
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
    } else if (name == "cstr"){
        switch (node->getNodeType()) {
            case NodeTypeStringLiteral:
            {
                return this->emit(node);
            }
            case NodeTypeNull:
            {
                auto charType = llvm::IntegerType::get(d->llvmContext, 8);
                return llvm::ConstantPointerNull::get(charType->getPointerTo());
            }
                
            default:
                break;
        }
        
    }
    else if (
        name == "i8"
        || name == "i16"
        || name == "i32"
        || name == "i64"
        || name == "i128"
        || name == "f32"
        || name == "f64"
    ){
        llvm::Value * ir;
        if (node->isA(NodeTypeNull)) {
            if (name == "i8") {
                ir = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(8, 0, false));
            } else if (name == "i16") {
                ir = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(16, 0, false));
            } else if (name == "i32") {
                ir = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(32, 0, false));
            } else if (name == "i64") {
                ir = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(64, 0, false));
            } else if (name == "i128") {
                ir = llvm::ConstantInt::get(d->llvmContext, llvm::APInt(128, 0, false));
            } else if (name == "f32") {
                ir = llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(0.0f));
            } else if (name == "f64") {
                ir = llvm::ConstantFP::get(d->llvmContext, llvm::APFloat(0.0));
            }
        } else {
            ir = this->emit(node);
        }
        if (!ir) {
            std::cerr << "EMIT NODE FAIL!!!!!!!!!!!!!!!!\n\n";
            return nullptr;
        }
        //nullable numbers are structs with two members
        auto alloca = d->irBuilder.CreateAlloca(this->llvmTypeFromLILType(targetTy));
        
        //number member
        std::vector<llvm::Value *> gepIndices1;
        gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
        gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
        auto member1 = d->irBuilder.CreateGEP(alloca, gepIndices1);
        d->irBuilder.CreateStore(ir, member1);
        
        //is null member
        auto nullBit = llvm::ConstantInt::get(
            d->llvmContext,
            llvm::APInt(
                1,
                (node->isA(NodeTypeNull) ? 1 : 0),
                false
            )
        );
        std::vector<llvm::Value *> gepIndices2;
        gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
        gepIndices2.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 1, false)));
        auto member2 = d->irBuilder.CreateGEP(alloca, gepIndices2);
        d->irBuilder.CreateStore(nullBit, member2);
        
        return d->irBuilder.CreateLoad(alloca);
        
    } else {
        std::cerr << "UNKNOWN NULLABLE TARGET TY FAIL!!!!!!!!!!!!!!!!\n\n";
        return nullptr;
    }
    
    return nullptr;
}

llvm::Value * LILIREmitter::emitForMultipleType(LIL::LILNode *node, std::shared_ptr<LILMultipleType> multiTy)
{
    if (node->getType()->equalTo(multiTy)) {
        return this->emit(node);
    }
    
    auto mtAlloca = d->currentAlloca;

    const auto & types = multiTy->getTypes();
    auto mostAlignedTy = this->getMostAlignedType(types)->clone();
    std::vector<llvm::Value *> gepIndices1;
    gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
    gepIndices1.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
    auto member1 = d->irBuilder.CreateGEP(d->currentAlloca, gepIndices1);

    auto destTy = this->llvmTypeFromLILType(node->getType().get())->getPointerTo();
    if (member1->getType() != destTy) {
        d->currentAlloca = d->irBuilder.CreateBitCast(member1, destTy);
    } else {
        d->currentAlloca = member1;
    }
    
    llvm::Value * llvmIr = this->emit(node);

    if (llvmIr) {
        auto nodeType = this->llvmTypeFromLILType(node->getType().get())->getPointerTo();
        if (member1->getType() != nodeType) {
            auto castedPtr = d->irBuilder.CreateBitCast(member1, nodeType);
            d->irBuilder.CreateStore(llvmIr, castedPtr);
        } else {
            d->irBuilder.CreateStore(llvmIr, member1);
        }
    }

    d->currentAlloca = mtAlloca;

    size_t theIndex = 1;
    auto nodeTy = node->getType();
    bool found = false;
    for (auto ty : types) {
        if (ty->equalTo(nodeTy)) {
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
    auto member2 = d->irBuilder.CreateGEP(d->currentAlloca, gepIndices2);
    d->irBuilder.CreateStore(typeIndex, member2);
    
    return nullptr;
}

llvm::Value * LILIREmitter::emitUnwrappedFromMT(LILNode *node, LILType *targetTy)
{
    auto wrappedVal = this->emitPointer(node);
    std::vector<llvm::Value *> gepIndices;
    gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
    gepIndices.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(LIL_GEP_INDEX_SIZE, 0, false)));
    auto gep = d->irBuilder.CreateGEP(wrappedVal, gepIndices);
    auto castedPtr = d->irBuilder.CreateBitCast(gep, this->llvmTypeFromLILType(targetTy)->getPointerTo());
    auto innerVal = d->irBuilder.CreateLoad(castedPtr);
    return innerVal;
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

size_t LILIREmitter::getSizeOfType(std::shared_ptr<LILType> ty) const
{
    size_t ret = 0;
    switch (ty->getTypeType()) {
        case TypeTypeSingle:
        {
            auto name = ty->getName();
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
            } else if (name == "cstr"){
                //FIXME: what about 32 bit systems?
                ret = 64;
            }
            break;
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
                total += this->getSizeOfType(field->getType());
            }
            ret = total;
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


