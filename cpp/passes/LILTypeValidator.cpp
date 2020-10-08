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
 *      This file checks if function calls match their prototypes
 *
 ********************************************************************/

#include "LILTypeValidator.h"
#include "LILClassDecl.h"
#include "LILErrorMessage.h"
#include "LILFunctionCall.h"
#include "LILFunctionType.h"
#include "LILObjectType.h"
#include "LILPropertyName.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

using namespace LIL;

LILTypeValidator::LILTypeValidator()
{
}

LILTypeValidator::~LILTypeValidator()
{
}

void LILTypeValidator::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "===  FUNCTION VALIDATION  ==\n";
        std::cerr << "============================\n\n";
    }
}

void LILTypeValidator::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    for (const auto & node : nodes) {
        this->validate(node);
    }
    if (this->getVerbose() && !this->hasErrors()) {
        std::cerr << "All OK\n\n";
    }
}

void LILTypeValidator::validate(std::shared_ptr<LILNode> node)
{
    if (LILNode::isContainerNode(node->getNodeType())) {
        this->validateChildren(node->getChildNodes());
    }
    if (this->getDebug()) {
        std::cerr << "## validating " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    if (node->isA(NodeTypeFunctionCall)) {
        auto fc = std::static_pointer_cast<LILFunctionCall>(node);
        switch (fc->getFunctionCallType()) {
            case FunctionCallTypeValuePath:
            {
                auto vp = fc->getSubject();
                auto remoteNode = this->_findNodeForValuePath(vp);
                if (!remoteNode || !remoteNode->isA(NodeTypeVarDecl)) {
                    LILErrorMessage ei;
                    ei.message =  "Function "+vp->stringRep()+"() not found";
                    LILNode::SourceLocation sl = node->getSourceLocation();
                    ei.file = sl.file;
                    ei.line = sl.line;
                    ei.column = sl.column;
                    this->errors.push_back(ei);
                    return;
                }
                
                auto vd = std::static_pointer_cast<LILVarDecl>(remoteNode);
                auto ty = vd->getType();
                if (!ty || !ty->isA(TypeTypeFunction)) {
                    LILErrorMessage ei;
                    ei.message =  "The path "+vp->stringRep()+" does not point to a function";
                    LILNode::SourceLocation sl = node->getSourceLocation();
                    ei.file = sl.file;
                    ei.line = sl.line;
                    ei.column = sl.column;
                    this->errors.push_back(ei);
                    return;
                }
                auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                auto fnTyArgs = fnTy->getArguments();
                auto args = fc->getArguments();
                if (fnTyArgs.size() != args.size()) {
                    LILErrorMessage ei;
                    if (args.size() == 0) {
                        if (fnTyArgs.size() > 1) {
                            ei.message =  "Missing argument in call: "+vp->stringRep()+" needs "+LILString::number((LILUnitI64)fnTyArgs.size()) + " arguments";
                        } else {
                            ei.message =  "Missing argument in call: "+vp->stringRep()+" needs one argument";
                        }
                    } else {
                        ei.message =  "Mismatch of numberof arguments: "+vp->stringRep()+" needs "+LILString::number((LILUnitI64)fnTyArgs.size()) + " arguments and was given " + LILString::number((LILUnitI64)args.size());
                    }
                    LILNode::SourceLocation sl = node->getSourceLocation();
                    ei.file = sl.file;
                    ei.line = sl.line;
                    ei.column = sl.column;
                    this->errors.push_back(ei);
                    return;
                }
                for (size_t i=0,j=args.size(); i<j; ++i) {
                    auto declArg = fnTyArgs[i];
                    auto callArg = args[i];
                    
                }
                break;
            }
                
            default:
                break;
        }
    }
}

void LILTypeValidator::validateChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->validate((*it));
    };
}

std::shared_ptr<LILNode> LILTypeValidator::_findNodeForValuePath(std::shared_ptr<LILValuePath> vp) const
{
    auto nodes = vp->getNodes();
    std::shared_ptr<LILNode> firstNode;
    if (nodes.size() == 1) {
        firstNode = nodes.front();
        if (firstNode->isA(NodeTypeVarName)) {
            return this->findNodeForVarName(static_cast<LILVarName *>(firstNode.get()));
        }
    } else if (nodes.size() > 1){
        firstNode = nodes.front();
        std::shared_ptr<LILClassDecl> classDecl;
        
        if (firstNode->isA(NodeTypeVarName)) {
            auto subjTy = firstNode->getType();
            if (subjTy && subjTy->isA(TypeTypeObject)) {
                auto objTy = std::static_pointer_cast<LILObjectType>(subjTy);
                classDecl = this->findClassWithName(objTy->getName().data());
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
                case NodeTypePropertyName:
                {
                    auto pn = std::static_pointer_cast<LILPropertyName>(node);
                    auto field = classDecl->getFieldNamed(pn->getName());
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
