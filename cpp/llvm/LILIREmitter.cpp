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
        std::vector<LILClassDecl *> selfContext;
        bool needsReturnValue;
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
        case NodeTypeBool:
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
        std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILExpression * value)
{
    std::shared_ptr<LILNode> left = value->getLeft();
    std::shared_ptr<LILNode> right = value->getRight();

    llvm::Value * leftV = this->deref(left.get());
    llvm::Value * rightV = this->deref(right.get());
    if (!leftV || !rightV) {
        std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
        return nullptr;
    }
    if (leftV->getType() != rightV->getType()) {
        std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
                    break;
            }
        }
        default:
            std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
            return nullptr;
    }

    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILStringLiteral * value)
{
    if (value->getIsCString()) {
        auto str = value->getValue().stripQuotes().data();
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
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILStringFunction * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILNullLiteral * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILVarDecl * value)
{
    auto name = value->getName().data();
    if (value->getIsExtern()) {
        return this->_emitFnSignature(name, std::static_pointer_cast<LILFunctionType>(value->getType()));
    } else {
        auto initVals = value->getInitVals();
        if (initVals.size() == 0) {
            d->namedValues[name] = d->irBuilder.CreateAlloca(this->llvmTypeFromLILType(value->getType()), 0, name);
        } else {
            for (auto initVal : initVals) {
                this->emit(initVal.get());
            }
        }
    }
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILClassDecl * value)
{
    d->selfContext.push_back(value);

    std::string name = value->getName().data();

    d->classTypes[name] = this->extractStructFromClass(value);

    for (auto methodVar : value->getMethods()) {
        auto vd = std::static_pointer_cast<LILVarDecl>(methodVar);
        for (auto method : vd->getInitVals()) {
            std::shared_ptr<LILFunctionDecl> fd = std::static_pointer_cast<LILFunctionDecl>(method);
            this->_emitMethod(fd.get(), value);
        }
    }

    d->selfContext.pop_back();
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILObjectDefinition * value)
{
    auto parent = value->getParentNode();
    if (!parent->isA(NodeTypeVarDecl)) {
        std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
        return nullptr;
    }
    auto vd = std::static_pointer_cast<LILVarDecl>(parent);
    LILString classNameStr = value->getType()->getName();
    std::string className = classNameStr.data();
    std::string instanceName = vd->getName().data();
    llvm::AllocaInst * alloca = d->irBuilder.CreateAlloca(
        d->classTypes[className],
        0,
        instanceName.c_str()
    );

    auto classValue = this->findClassWithName(classNameStr);
    size_t theIndex = 0;
    for (auto field : classValue->getFields()) {
        auto vd = std::static_pointer_cast<LILVarDecl>(field);
        auto varName = vd->getName();
        
        std::shared_ptr<LILNode> initVal = nullptr;
        for (auto node : value->getNodes()) {
            if (node->isA(NodeTypeAssignment)) {
                auto asgmt = std::static_pointer_cast<LILAssignment>(node);
                auto subj = asgmt->getSubject();
                if (subj) {
                    auto vp = std::static_pointer_cast<LILValuePath>(subj);
                    std::shared_ptr<LILNode> firstNode = vp->getNodes().front();
                    if (firstNode->isA(NodeTypePropertyName)) {
                        auto pn = std::static_pointer_cast<LILPropertyName>(firstNode);
                        if (pn->getName() == varName) {
                            initVal = asgmt->getValue();
                            break;
                        }
                    }
                }
            }
        }
        
        if (!initVal) {
            initVal = vd->getInitVal();
        }

        if (initVal) {
            llvm::Value * llvmValue = this->deref(initVal.get());
            auto gep = this->_emitGEP(alloca, classValue->getName(), theIndex, instanceName + "." + varName, 0);
            d->irBuilder.CreateStore(llvmValue, gep);
        }
        ++theIndex;
    }

    //call the constructor
    LILString decoratedName = this->decorate("", className, "construct", nullptr);
    llvm::Function* fun = d->llvmModule->getFunction(decoratedName.data());
    if (fun) {
        std::vector<llvm::Value *> argsvect;
        argsvect.push_back(alloca);
        d->irBuilder.CreateCall(fun, argsvect);
    }

    return alloca;

}

llvm::Value * LILIREmitter::_emit(LILAssignment * value)
{
    auto theValue = value->getValue();
    if (theValue) {
        llvm::Value * llvmSubject = this->emit(value->getSubject().get());
        llvm::Value * llvmValue = this->emit(value->getValue().get());
        return d->irBuilder.CreateStore(llvmValue, llvmSubject);
    } else {
        std::cerr << "!!!!!!!!!!ASSIGNMENT HAD NO VALUE !!!!!!!!!!!!!!!!\n";
        return nullptr;
    }
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
        std::shared_ptr<LILVarDecl> vd;
        LILNode * subject;
        llvm::Value * llvmSubject = nullptr;
        LILString className;

        if (firstNode->isA(NodeTypeVarName)) {
            auto vn = std::static_pointer_cast<LILVarName>(firstNode);
            std::shared_ptr<LILNode> subjectNode = this->findNodeForVarName(vn.get());
            subject = subjectNode.get();
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
                    subject = d->selfContext.back();
                    break;
                }
                default:
                    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
                    return nullptr;
            }
        }

        ++it;

        while (it != childNodes.end()) {
            auto currentNode = *it;
            switch (currentNode->getNodeType()) {
                case NodeTypeFunctionCall:
                {
                    auto fc = std::static_pointer_cast<LILFunctionCall>(currentNode);
                    if (vd->getIsExtern()) {
                        return this->_emit(fc.get(), vd->getName());
                    }

                    auto fcTypes = fc->getTypes();
                    std::shared_ptr<LILFunctionDecl> targetFn;
                    if (vd->getInitVals().size() > 1 && fcTypes.size() > 0) {
                        targetFn = this->chooseFnByType(vd, fcTypes);
                    } else {
                        auto initVal = vd->getInitVal();
                        if (initVal && initVal->isA(NodeTypeFunctionDecl)) {
                            targetFn = std::static_pointer_cast<LILFunctionDecl>(vd->getInitVal());
                        }
                    }
                    if (targetFn) {

                        llvmSubject = this->_emit(fc.get(), targetFn->getName());
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

                    //get index of field into struct
                    auto ty = subject->getType();
                    if (!ty->isA(TypeTypeObject)) {
                        continue;
                    }
                    auto objTy = std::static_pointer_cast<LILObjectType>(ty);
                    auto classValue = this->findClassWithName(objTy->getName());
                    auto fields = classValue->getFields();
                    size_t theIndex = 0;
                    for (size_t i=0, j=fields.size(); i<j; ++i) {
                        auto vd = std::static_pointer_cast<LILVarDecl>(fields[i]);
                        if (vd->getName() == pn->getName()) {
                            theIndex = i;
                            break;
                        }
                    }

                    std::string name = pn->getName().data();
                    std::string tmp_name = "_lil_"+name+"_tmp";
                    auto gep = this->_emitGEP(llvmSubject, className, theIndex, tmp_name, 0);
                    llvmSubject = d->irBuilder.CreateLoad(gep, name.data());
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

llvm::Value * LILIREmitter::_emitGEP(llvm::Value * llvmValue, LILString className, LILUnitI32 fieldIndex, LILString fieldName, LILUnitI32 arrayIndex)
{
    std::vector<llvm::Value *> idList;
    idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(32, 0, false)));
    idList.push_back(llvm::ConstantInt::get(d->llvmContext, llvm::APInt(32, fieldIndex, false)));
    return llvm::GetElementPtrInst::Create(d->classTypes[className.data()], llvmValue, idList, fieldName.data(), d->irBuilder.GetInsertBlock());
}

llvm::Value * LILIREmitter::_emit(LILPropertyName * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILRule * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILSimpleSelector * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILSelectorChain * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;

}

llvm::Value * LILIREmitter::_emit(LILSelector * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILCombinator * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILFilter * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILFlag * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
    return val;
}

llvm::Function * LILIREmitter::_emit(LILFunctionDecl * value)
{
    switch (value->getFunctionDeclType()) {
        case FunctionDeclTypeFn:
        {
            return this->_emitFn(value);
        }

        default:
            std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
            break;
    }
    return nullptr;
}

llvm::Function * LILIREmitter::_emitFnSignature(std::string name, const std::shared_ptr<LILFunctionType> fnTy)
{
    std::vector<llvm::Type*> types;
    auto arguments = fnTy->getArguments();
    for (auto & arg : arguments) {
        llvm::Type * llvmTy = this->llvmTypeFromLILType(arg);
        if (llvmTy) {
            types.push_back(llvmTy);
        }
    }
    return this->_emitFnSignature(name, types, fnTy);
}

llvm::Function * LILIREmitter::_emitFnSignature(std::string name, std::vector<llvm::Type*> types, const std::shared_ptr<LILFunctionType> fnTy)
{
    std::shared_ptr<LILType> retTy = fnTy->getReturnType();
    llvm::Type * returnType;
    if (retTy) {
        returnType = this->llvmTypeFromLILType(retTy);
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
    auto arguments = value->getArguments();

    auto fnTy = std::static_pointer_cast<LILFunctionType>(value->getType());
    llvm::Function * fun = this->_emitFnSignature(value->getName().data(), fnTy);

    size_t argIndex = 0;
    for (auto & llvmArg : fun->args()) {
        auto arg = arguments[argIndex];
        if(!arg || !arg->isA(NodeTypeVarDecl)){
            std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
            return nullptr;
        }

        std::shared_ptr<LILVarDecl> vd = std::static_pointer_cast<LILVarDecl>(arg);
        llvmArg.setName(vd->getName().data());
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

    const std::map<std::string, llvm::Value *> & hiddenLocals = d->hiddenLocals.back();
    for (auto it = hiddenLocals.begin(); it != hiddenLocals.end(); ++it) {
        d->namedValues[it->first] = it->second;
    }
    d->hiddenLocals.pop_back();
    return fun;
}

llvm::Function * LILIREmitter::_emitFnBody(llvm::Function * fun, LILFunctionDecl * value)
{
    auto body = value->getBody();

    auto ty = value->getReturnType();
    if (ty->getName() != "null") {
        d->returnAlloca = d->irBuilder.CreateAlloca(fun->getReturnType(), 0, "return");
    }
    for (auto & node : body) {
        bool breakAfter = false;
        if (node->isA(FunctionCallTypeReturn)) {
            breakAfter = true;;
        }

        if (node->isA(NodeTypeVarDecl)) {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            auto initVals = vd->getInitVals();
            if (initVals.size() == 0) {
                this->emit(vd.get());
            } else {
                std::string name = vd->getName().data();
                for (auto initVal : initVals){
                    switch (initVal->getNodeType()) {
                        case NodeTypeFunctionDecl:
                        {
                            //this is a lambda
                            this->emit(initVal.get());
                            break;
                        }
                        case NodeTypeObjectDefinition:
                        {
                            llvm::Value * llvmValue = this->emit(initVal.get());
                            d->hiddenLocals.back()[name] = d->namedValues[name];
                            d->namedValues[name] = llvmValue;
                            break;
                        }
                            
                        case NodeTypeValuePath:
                        {
                            llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
                            llvm::Value * llvmValue = this->emit(initVal.get());
                            if (llvmValue) {
                                llvm::AllocaInst * alloca = this->createEntryBlockAlloca(fun, name, llvmValue->getType());
                                d->irBuilder.CreateStore(llvmValue, alloca);
                                d->hiddenLocals.back()[name] = d->namedValues[name];
                                d->namedValues[name] = alloca;
                            }
                            
                            break;
                        }
                            
                        default:
                            //local var
                            llvm::Function * fun = d->irBuilder.GetInsertBlock()->getParent();
                            llvm::Value * llvmValue = this->emit(initVal.get());
                            if (llvmValue) {
                                llvm::AllocaInst * alloca = this->createEntryBlockAlloca(fun, name, llvmValue->getType());
                                d->irBuilder.CreateStore(llvmValue, alloca);
                                d->hiddenLocals.back()[name] = d->namedValues[name];
                                d->namedValues[name] = alloca;
                            }
                            break;
                    }
                }
            }
        } else {
            this->emit(node.get());
        }

        if (breakAfter) {
            break;
        }
    }

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

llvm::Function * LILIREmitter::_emitMethod(LILFunctionDecl * value, LILClassDecl * classValue)
{
    auto arguments = value->getArguments();

    auto fnTy = std::static_pointer_cast<LILFunctionType>(value->getType());

    std::vector<llvm::Type*> types;

    std::string className = classValue->getName().data();
    auto classType = d->classTypes[className];
    llvm::Type * classPtrType = llvm::PointerType::get(classType, 0);

    types.push_back(classPtrType);

    auto tyArgs = fnTy->getArguments();
    for (auto & arg : tyArgs) {
        llvm::Type * llvmTy = this->llvmTypeFromLILType(arg);
        if (llvmTy) {
            types.push_back(llvmTy);
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
                std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
            std::cerr << "fcalltype none\n";
            break;
        }
        case FunctionCallTypeReturn:
        {
            return this->_emitReturn(value);
        }
        case FunctionCallTypePrint:
        {

            break;
        }
    }

    return nullptr;

}

llvm::Value * LILIREmitter::_emitReturn(LILFunctionCall * value)
{
    if (value->getArguments().size() > 0) {
        llvm::Value * retVal = this->deref(value->getArguments().front().get());
        if (retVal) {
            llvm::Value * theReturn = d->irBuilder.CreateStore(retVal, d->returnAlloca);
            d->needsReturnValue = true;

            return theReturn;
        }
    }
    return nullptr;
}

llvm::Value * LILIREmitter::_emit(LILFunctionCall * value, LILString name)
{
    llvm::Function* fun = d->llvmModule->getFunction(name.data());
    auto fcArgs = value->getArguments();
    if (fun) {
        std::vector<llvm::Value *> argsvect;
        llvm::Value * fcArgIr;
        for (auto fcArg : fcArgs) {
            if (fcArg->isA(NodeTypeAssignment)) {
                auto asgmt = std::static_pointer_cast<LILAssignment>(fcArg);
                auto asgmtVal = asgmt->getValue();
                fcArgIr = this->deref(asgmtVal.get());
            } else {
                fcArgIr = this->deref(fcArg.get());
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
        case FlowControlTypeNone:
        {
            std::cerr << "flowcontrol type none\n";
            break;
        }
        case FlowControlTypeIf:
        {
            return this->_emitIf(value);
        }
        case FlowControlTypeFinally:
        {
            for (auto node : value->getThen()) {
                this->emit(node.get());
            }
            return nullptr;
        }
        default:
        {
            std::cerr << "!!!!!!!!!! UNKNOWN FLOW CONTROL TYPE!!!!!!!!!!!!!!!!\n";
            return nullptr;
        }
    }

    return nullptr;
}

llvm::Value * LILIREmitter::_emitIf(LILFlowControl * value)
{
    const auto & args = value->getArguments();
    if (args.size() == 0) {
        for (auto & node : value->getThen()) {
            this->emit(node.get());
        }
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
        if (node->isA(FunctionCallTypeReturn)) {
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
        if (node->isA(FunctionCallTypeReturn)) {
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

llvm::Value * LILIREmitter::_emit(LILInstruction * value)
{
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
    return nullptr;
}

llvm::Value * LILIREmitter::deref(LILNode * node)
{
    if(node->isA(NodeTypeValuePath)){
        auto vp = static_cast<LILValuePath *>(node);

        auto ptr = this->emit(vp);
        auto firstNode = vp->getNodes().front();
        if (!firstNode->isA(NodeTypeVarName)) {
            std::cerr << "!!!!!!!!!!ARG CODEGEN FAIL!!!!!!!!!!!!!!!!\n";
            return nullptr;
        }
        auto vn = std::static_pointer_cast<LILVarName>(firstNode);
        auto name = vn->getName();
        return d->irBuilder.CreateLoad(ptr, name.data());
    } else {
        return this->emit(node);
    }
}

void LILIREmitter::printIR(llvm::raw_ostream & file) const
{
    d->llvmModule->print(file, nullptr);
}

llvm::Type * LILIREmitter::llvmTypeFromLILType(std::shared_ptr<LILType> type)
{
    if (type->isA(TypeTypePointer))
    {
        auto ptrTy = std::static_pointer_cast<LILPointerType>(type);
        auto argTy = ptrTy->getArgument();
        if (!argTy) {
            std::cerr << "!!!!!!!!!!PTR TYPE EMPTY ARG FAIL!!!!!!!!!!!!!!!!\n";
            return nullptr;
        }
        auto llvmType = this->llvmTypeFromLILType(argTy);
        return llvmType->getPointerTo();
    }
    else if (type->isA(TypeTypeObject))
    {
        auto objTy = std::static_pointer_cast<LILObjectType>(type);
        std::vector<llvm::Type *> structTypes;
        for (auto member : objTy->getFields()) {
            auto memTy = this->llvmTypeFromLILType(member);
            if (memTy) {
                structTypes.push_back(memTy);
            } else {
                std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
                return nullptr;
            }
        }
        return llvm::StructType::get(d->llvmContext, structTypes);
    }
    LILString typestr = type->getName();
    if (typestr == "bool") {
        return llvm::Type::getInt1Ty(d->llvmContext);
    } else if (typestr == "i8"){
        return llvm::Type::getInt8Ty(d->llvmContext);
    } else if (typestr == "i16"){
        return llvm::Type::getInt16Ty(d->llvmContext);
    } else if (typestr == "i32"){
        return llvm::Type::getInt32Ty(d->llvmContext);
    } else if (typestr == "i64"){
        return llvm::Type::getInt64Ty(d->llvmContext);
    } else if (typestr == "f32"){
        return llvm::Type::getFloatTy(d->llvmContext);
    } else if (typestr == "f64"){
        return llvm::Type::getDoubleTy(d->llvmContext);
    } else if (typestr == "cstr"){
        auto charType = llvm::IntegerType::get(d->llvmContext, 8);
        return charType->getPointerTo();
    } else if (typestr == "null") {
        return nullptr;
    }
    std::cerr << "!!!!!!!!!!FAIL!!!!!!!!!!!!!!!!\n";
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
            for (size_t i=0, j=fdTys.size(); i<j; ++i) {
                auto ty = fdTys[i];
                auto resultTy = LILType::merge(ty, types[i]);
                if (resultTy && resultTy->equalTo(ty)) {
                    return fd;
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
        llvm::Type * llvmTy = this->llvmTypeFromLILType(fld->getType());
        if (llvmTy) {
            types.push_back(llvmTy);
        }
    }
    return llvm::StructType::create(d->llvmContext, types, value->getName().data());
}
