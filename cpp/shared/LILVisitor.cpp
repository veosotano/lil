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
 *      This file defines the base class for visitors
 *
 ********************************************************************/

#include "LILVisitor.h"
#include "LILClassDecl.h"
#include "LILErrorMessage.h"
#include "LILFlowControl.h"
#include "LILFunctionCall.h"
#include "LILVarName.h"
#include "LILVarNode.h"
#include "LILRootNode.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILMultipleType.h"
#include "LILNumberLiteral.h"
#include "LILObjectType.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILRule.h"
#include "LILStaticArrayType.h"
#include "LILStringLiteral.h"
#include "LILValuePath.h"
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

std::shared_ptr<LILNode> LILVisitor::findNodeForValuePath(LILValuePath * vp) const
{
    auto nodes = vp->getNodes();
    std::shared_ptr<LILNode> firstNode;
    if (nodes.size() == 1) {
        firstNode = nodes.front();
        if (firstNode->isA(NodeTypeVarName)) {
            return this->recursiveFindNode(firstNode);
        }
    } else if (nodes.size() > 1){
        firstNode = nodes.front();
        std::shared_ptr<LILClassDecl> classDecl;
        
        if (firstNode->isA(NodeTypeVarName)) {
            auto localNode = this->recursiveFindNode(firstNode);
            if (localNode) {
                auto subjTy = localNode->getType();
                if (!subjTy) {
                    std::cerr << "CANNOT TRAVERSE VALUE PATH IF SUBJECT TYPE IS NULL FAIL !!!!!!!\n\n";
                    return nullptr;
                }
                if (!this->inhibitSearchingForIfCastType && subjTy->isA(TypeTypeMultiple)) {
                    size_t outStartIndex = 0;
                    auto ifCastTy = this->findIfCastType(vp, outStartIndex);
                    if (ifCastTy) {
                        subjTy = ifCastTy;
                    }
                }
                if (subjTy->isA(TypeTypeObject)) {
                    auto objTy = std::static_pointer_cast<LILObjectType>(subjTy);
                    classDecl = this->findClassWithName(objTy->getName().data());
                }
            }
        }
        else if (firstNode->isA(SelectorTypeSelfSelector)) {
            classDecl = this->findAncestorClass(firstNode);
        }
        if (!classDecl) {
            return nullptr;
        }
        for (size_t i=1, j=nodes.size(); i<j; ++i) {
            auto node = nodes[i];
            switch (node->getNodeType()) {
                case NodeTypeFunctionCall:
                {
                    auto fc = std::static_pointer_cast<LILFunctionCall>(node);
                    auto method = classDecl->getMethodNamed(fc->getName());
                    if (!method->isA(NodeTypeFunctionDecl)) {
                        std::cerr << "!!!!!!!!!!NODE IS NOT FUNCTION DECL FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto fd = std::static_pointer_cast<LILFunctionDecl>(method);
                    auto fnTy = fd->getFnType();
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
                    auto pn = std::static_pointer_cast<LILPropertyName>(node);
                    auto pnName = pn->getName();
                    auto field = classDecl->getFieldNamed(pnName);
                    if (i==j-1) {
                        return field;
                    } else {
                        auto fieldTy = field->getType();
                        if (fieldTy && fieldTy->isA(TypeTypeObject)) {
                            auto fieldObjTy = std::static_pointer_cast<LILObjectType>(fieldTy);
                            classDecl = this->findClassWithName(fieldObjTy->getName().data());
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
    for (auto classVal : this->getRootNode()->getClasses()) {
        if (classVal->getName() == name) {
            return classVal;
        }
    }
    return nullptr;
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
                if (firstArg->isA(NodeTypeVarName) && nodes.size() == 1) {
                    if (firstArg->equalTo(nodes.front())) {
                        auto ifCastTy = args.back();
                        return std::static_pointer_cast<LILType>(ifCastTy);
                    }
                }
            }
        }
        parent = parent->getParentNode();
    }
    return ret;
}
