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
#include "LILFunctionType.h"
#include "LILObjectType.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"

using namespace LIL;

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

void LILVisitor::printErrors(const LILString & code) const
{
    if (this->errors.size() == 0) {
        return;
    }

    std::vector<std::string> lines = this->splitString(code.chardata(), "\n");

    std::cerr << "\nFound ";
    std::cerr << errors.size();
    std::cerr << " errors in your code:\n";
    for (auto it=errors.begin(); it!=errors.end(); ++it) {
        LILErrorMessage ei = *it;
        std::cerr << ei.message.chardata();
        if (ei.line > 0) {
            std::cerr << " on line ";
            std::cerr << ei.line;
            std::cerr << " column ";
            std::cerr << ei.column;
            std::cerr << "\n\n";

            if (ei.line > 2)
            {
                std::cerr << ei.line - 1;
                std::cerr << ": ";
                std::cerr << lines[ei.line-2];
                std::cerr << "\n";
            }

            std::cerr << ei.line;
            std::cerr << ": ";
            std::cerr << lines[ei.line-1];
            std::cerr << "\n";
            std::string indicator = "   ";
            if (ei.column > 2) {
                for (unsigned i=0; i<ei.column-3; ++i) {
                    indicator += " ";
                }
            }
            if (ei.column == 0) {
                indicator+= "^__\n";
            } else if (ei.column == 1) {
                indicator+= "_^__\n";
            } else {
                indicator+= "__^__\n";
            }
            std::cerr << indicator;

            if (ei.line < lines.size()-1)
            {
                std::cerr << ei.line+1;
                std::cerr << ": ";
                std::cerr << lines[ei.line];
                std::cerr << "\n";
            }
        } else {
            std::cerr << "\n";
        }
    }
    std::cerr << "\n";
}

std::vector<std::string> LILVisitor::splitString(std::string phrase, std::string delimiter) const
{
    std::vector<std::string> list;
    std::string s = phrase;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
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
                if (!this->inhibitSearchingForIfCastType && subjTy->isA(TypeTypeMultiple)) {
                    size_t outStartIndex = 0;
                    auto ifCastTy = this->findIfCastType(vp, outStartIndex);
                    if (ifCastTy) {
                        subjTy = ifCastTy;
                    }
                }
                if (subjTy && subjTy->isA(TypeTypeObject)) {
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
                    if (!method->isA(NodeTypeVarDecl)) {
                        std::cerr << "!!!!!!!!!!NODE IS NOT VAR DECL FAIL !!!!!!!!!!!!!!!!\n";
                        return nullptr;
                    }
                    auto vd = std::static_pointer_cast<LILVarDecl>(method);
                    auto ty = vd->getType();
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
        ret += LILString("c") + LILString::number((LILUnitF64)className.length()) + "_" + className + "_";
    }
    ret += LILString("f") + LILString::number((LILUnitF64)name.length()) + "_" + name;
    if(type){
        ret += this->typeToString(type);
    }

    return ret;
}

LILString LILVisitor::typeToString(std::shared_ptr<LILType> type) const
{
    LILString ret("");
    auto ft = std::dynamic_pointer_cast<LILFunctionType>(type);
    if (ft) {
        const auto & args = ft->getArguments();
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
    } else {
        ret = type->getName();
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
