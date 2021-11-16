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
#include "LILAssignment.h"
#include "LILClassDecl.h"
#include "LILConversionDecl.h"
#include "LILErrorMessage.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILFunctionType.h"
#include "LILMultipleType.h"
#include "LILNodeToString.h"
#include "LILObjectType.h"
#include "LILPointerType.h"
#include "LILPropertyName.h"
#include "LILTypeDecl.h"
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
        std::cerr << "====  TYPE VALIDATION   ====\n";
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
        std::cerr << "## validating " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeFunctionCall:
        {
            auto fc = std::static_pointer_cast<LILFunctionCall>(node);
            this->_validate(fc);
            break;
        }
            
        case NodeTypeObjectDefinition:
        {
            auto od = std::static_pointer_cast<LILObjectDefinition>(node);
            this->_validate(od);
            break;
        }
            
        case NodeTypeVarDecl:
        {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            this->_validate(vd);
            break;
        }
            
        default:
            break;
    }
}

void LILTypeValidator::_validate(std::shared_ptr<LILFunctionCall> fc)
{
    if (fc->isA(FunctionCallTypeValuePath)) {
        auto vp = fc->getSubject();
        bool isMethod = false;
        const auto & nodes = vp->getNodes();
        auto firstNode = nodes.front();
        std::shared_ptr<LILNode> remoteNode;
        if (nodes.size() == 1 && firstNode->isA(SelectorTypeSelfSelector)) {
            auto classDecl = this->findAncestorClass(vp);
            remoteNode = classDecl->getMethodNamed(fc->getName());
            isMethod = true;
        } else {
            remoteNode = this->findNodeForValuePath(vp.get());
        }
        if (!remoteNode || (!remoteNode->isA(NodeTypeFunctionDecl) && !remoteNode->isA(NodeTypeVarDecl))) {
            LILErrorMessage ei;
            ei.message =  "Function "+LILNodeToString::stringify(vp.get())+"."+fc->getName()+"() not found";
            LILNode::SourceLocation sl = fc->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
            return;
        }
        
        std::shared_ptr<LILType> fieldTy;
        if (remoteNode->isA(NodeTypeFunctionDecl)) {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(remoteNode);
            fieldTy = fd->getFnType();
        } else if (remoteNode->isTypedNode()) {
            auto tyNode = std::static_pointer_cast<LILTypedNode>(remoteNode);
            fieldTy = tyNode->getType();
        }

        if (!this->inhibitSearchingForIfCastType && fieldTy->isA(TypeTypeMultiple)) {
            size_t outVpSize = 0;
            auto ifCastTy = this->findIfCastType(vp.get(), outVpSize);
            if (ifCastTy) {
                fieldTy = ifCastTy;
            } else {
                std::cerr << "!!!!!!! UNIMPLEMENTED FAIL !!!!!!!\n";
                return;
            }
        }
        
        std::shared_ptr<LILType> ty;
        if (fieldTy->isA(TypeTypePointer)) {
            auto ptrTy = std::static_pointer_cast<LILPointerType>(fieldTy);
            fieldTy = ptrTy->getArgument();
        }
        if (fieldTy->isA(TypeTypeObject)) {
            auto classDecl = this->findClassWithName(fieldTy->getName());
            if (!classDecl) {
                std::cerr << "!!!!!!! CLASS NOT FOUND FAIL !!!!!!!\n";
                return;
            }
            auto methodNode = classDecl->getMethodNamed(fc->getName());
            if (!methodNode) {
                LILErrorMessage ei;
                ei.message =  "The class "+fieldTy->getName()+" does not contain a field named "+fc->getName();
                LILNode::SourceLocation sl = fc->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
                return;
            }
            auto method = std::static_pointer_cast<LILFunctionDecl>(methodNode);
            ty = method->getFnType();
            isMethod = true;
        }
        else if (fieldTy->isA(TypeTypeFunction))
        {
            ty = fieldTy;
        }

        if (!ty || !ty->isA(TypeTypeFunction)) {
            LILErrorMessage ei;
            ei.message =  "The path "+LILNodeToString::stringify(vp.get())+" does not point to a function";
            LILNode::SourceLocation sl = fc->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
            return;
        }
        auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
        auto fnTyArgs = fnTy->getArguments();
        auto args = fc->getArguments();
        size_t argNum = args.size();
        if (isMethod) {
            argNum += 1;
        }
        if (fnTyArgs.size() != argNum) {
            LILErrorMessage ei;
            if (args.size() == 0) {
                if (fnTyArgs.size() > 1) {
                    ei.message =  "Missing argument in call: "+LILNodeToString::stringify(vp.get())+"."+fc->getName()+" needs "+LILString::number((LILUnitI64)fnTyArgs.size()) + " arguments";
                } else {
                    ei.message =  "Missing argument in call: "+LILNodeToString::stringify(vp.get())+"."+fc->getName()+" needs one argument";
                }
            } else {
                ei.message =  "Mismatch of number of arguments: "+LILNodeToString::stringify(vp.get())+"."+fc->getName()+" needs "+LILString::number((LILUnitI64)fnTyArgs.size()-(isMethod?1:0)) + " arguments and was given " + LILString::number((LILUnitI64)argNum-(isMethod?1:0));
            }
            LILNode::SourceLocation sl = fc->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
            return;
        }
        for (size_t i=0,j=args.size(); i<j; ++i) {
            auto declArg = fnTyArgs[i];
            auto callArg = args[i];
            //FIXME
        }
    }
    else if ( fc->isA(FunctionCallTypeNone))
    {
        auto localNode = this->findNodeForName(fc->getName(), fc->getParentNode().get());
        if (!localNode) {
            LILErrorMessage ei;
            ei.message =  "Function "+fc->getName()+" not found.";
            LILNode::SourceLocation sl = fc->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
            return;
        }
        
        auto ty = localNode->getType();
        bool pass = true;
        if (! (localNode->isA(NodeTypeVarDecl) || localNode->isA(NodeTypeFunctionDecl) )) {
            pass = false;
        }
        if (!ty) {
            pass = false;
        } else if (ty->isA(TypeTypePointer)) {
            auto pTy = std::static_pointer_cast<LILPointerType>(ty);
            const auto & arg = pTy->getArgument();
            if (!arg) {
                pass = false;
            } else if (!arg->isA(TypeTypeFunction)){
                pass = false;
            }
        } else if (!ty->isA(TypeTypeFunction)) {
            pass = false;
        }
        
        if ( !pass ) {
            LILErrorMessage ei;
            ei.message =  fc->getName()+" is not a function.";
            LILNode::SourceLocation sl = fc->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        }
        std::shared_ptr<LILFunctionType> fnTy;
        if (ty->isA(TypeTypeFunction)) {
                fnTy = std::static_pointer_cast<LILFunctionType>(ty);
        } else if (ty->isA(TypeTypePointer)){
            auto ptrTy = std::static_pointer_cast<LILPointerType>(ty);
            auto argTy = ptrTy->getArgument();
            fnTy = std::static_pointer_cast<LILFunctionType>(argTy);
        }
        if (!fnTy) {
            LILErrorMessage ei;
            ei.message =  "Unknown type of function call "+fc->getName();
            LILNode::SourceLocation sl = fc->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
            return;
        }
        
        auto fnTyArgs = fnTy->getArguments();
        auto fcArgTys = fc->getArgumentTypes();
        auto fcArgs = fc->getArguments();
        if (fcArgs.size() != fcArgTys.size()) {
            std::cerr << "!!!!!!! SIZE OF FUNCTION CALL ARGS AND ARGUMENT TYPES WAS NOT THE SAME FAIL !!!!!!!\n";
            return;
        }
        if (!fnTy->getIsVariadic() && fnTyArgs.size() != fcArgTys.size()) {
            LILErrorMessage ei;
            ei.message =  "Function "+fc->getName()+" requires " + LILString::number((LILUnitI64)fnTyArgs.size()) + " arguments and " + LILString::number((LILUnitI64)fcArgTys.size()) + " were given.";
            LILNode::SourceLocation sl = fc->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        }
        
        auto conversions = this->getRootNode()->getConversions();
        
        size_t i = 0;
        for (auto fnTyArg : fnTyArgs) {
            if (fcArgTys.size() > i) {
                auto fcArgTy = fcArgTys[i];

                std::shared_ptr<LILType> argTy;
                LILString argName;
                if (fnTyArg->isA(NodeTypeType))
                {
                    argTy = std::static_pointer_cast<LILType>(fnTyArg);
                    argName = LILString::number((LILUnitI64)i+1);
                }
                else if (fnTyArg->isA(NodeTypeVarDecl))
                {
                    argTy = fnTyArg->getType();
                    auto vd = std::static_pointer_cast<LILVarDecl>(fnTyArg);
                    argName = vd->getName();
                }
                if (argTy->isA(TypeTypeMultiple)) {
                    auto mtTy = std::static_pointer_cast<LILMultipleType>(argTy);
                    bool found = false;
                    if (fcArgTy->isA(TypeTypeMultiple)) {
                        auto fcArgMultipleTy = std::static_pointer_cast<LILMultipleType>(fcArgTy);
                        bool allFound = true;
                        for (auto fcArgMtTy : fcArgMultipleTy->getTypes()) {
                            found = false;
                            for (auto mtArgTy : mtTy->getTypes()) {
                                if (fcArgMtTy->equalTo(mtArgTy) || this->_isDefinitionOf(fcArgMtTy, mtArgTy)) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                allFound = false;
                                break;
                            }
                        }
                        found = allFound;
                    } else {
                        bool wasNullable = false;
                        if (fcArgTy->getIsNullable() && mtTy->getIsNullable()) {
                            wasNullable = true;
                            fcArgTy->setIsNullable(false);
                        }
                        for (auto mtArgTy : mtTy->getTypes()) {
                            if (mtArgTy->equalTo(fcArgTy) || this->_isDefinitionOf(fcArgTy, mtArgTy)) {
                                i += 1;
                                found = true;
                                break;
                            } else {
                                LILString conversionName = LILNodeToString::stringify(fcArgTy.get());
                                conversionName += "_to_";
                                conversionName += LILNodeToString::stringify(mtArgTy.get());
                                if (conversions.count(conversionName)) {
                                    i += 1;
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (wasNullable) {
                            fcArgTy->setIsNullable(true);
                        }
                    }
                    if (found) {
                        continue;
                    }
                }
                else if (fcArgTy->isA(TypeTypeMultiple))
                {
                    auto mtTy = std::static_pointer_cast<LILMultipleType>(fcArgTy);
                    bool found = false;
                    for (auto mtArgTy : mtTy->getTypes()) {
                        if (argTy->equalTo(mtArgTy) || this->_isDefinitionOf(mtArgTy, argTy)) {
                            i += 1;
                            found = true;
                            break;
                        } else {
                            LILString conversionName = LILNodeToString::stringify(mtArgTy.get());
                            conversionName += "_to_";
                            conversionName += LILNodeToString::stringify(argTy.get());
                            if (conversions.count(conversionName)) {
                                i += 1;
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found) {
                        continue;
                    }
                }
                else
                {
                    if (argTy->equalTo(fcArgTy) || this->_isDefinitionOf(fcArgTy, argTy)) {
                        i += 1;
                        continue;
                    } else {
                        LILString conversionName = fcArgTy->getStrongTypeName();
                        if (conversionName.length() == 0) {
                            conversionName = LILNodeToString::stringify(fcArgTy.get());
                        }
                        conversionName += "_to_";
                        auto targetTyName = argTy->getStrongTypeName();
                        if (targetTyName.length() > 0) {
                            conversionName += targetTyName;
                        } else {
                            conversionName += LILNodeToString::stringify(argTy.get());
                        }
                        if (conversions.count(conversionName)) {
                            i += 1;
                            continue;
                        }
                    }
                }

                LILErrorMessage ei;
                ei.message =  "Type mismatch while calling " + fc->getName() + ": argument " + argName + " needs type "+LILNodeToString::stringify(argTy.get())+" but was given "+LILNodeToString::stringify(fcArgTy.get());
                LILNode::SourceLocation sl = fcArgs[i]->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            }

            i += 1;
        }
    }
}

bool LILTypeValidator::_isDefinitionOf(std::shared_ptr<LILType> nativeTy, std::shared_ptr<LILType> customTy)
{
    auto rootNode = this->getRootNode();
    auto typeDecls = rootNode->getTypes();
    for (auto typeDecl : typeDecls) {
        auto srcTy = typeDecl->getSrcType();
        if (srcTy->equalTo(customTy)) {
            return typeDecl->getDstType()->equalTo(nativeTy);
        }
    }
    return false;
}

void LILTypeValidator::_validate(std::shared_ptr<LILObjectDefinition> od)
{
    auto ty = od->getType();
    auto classValue = this->findClassWithName(ty->getName());
    if (!classValue) {
        LILErrorMessage ei;
        ei.message =  "Class "+ty->getName()+" not found";
        LILNode::SourceLocation sl = od->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
        return;
    }

    auto clFields = classValue->getFields();
    auto odFields = od->getNodes();
    for (auto odField : odFields) {
        if (!odField->isA(NodeTypeAssignment)){
            std::cerr << "!!!!!!! NODE IS NOT ASSIGNMENT FAIL !!!!!!!\n";
            continue;
        }
        auto as = std::static_pointer_cast<LILAssignment>(odField);
        auto asSubject = as->getSubject();
        if (!asSubject) {
            std::cerr << "!!!!!!! SUBJECT OF ASSIGNMENT WAS NULL FAIL !!!!!!!\n";
            continue;
        }
        LILString pnName;
        if (asSubject->isA(NodeTypeValuePath)) {
            auto vp = std::static_pointer_cast<LILValuePath>(asSubject);
            auto firstNode = vp->getNodes().front();
            if (!firstNode || !firstNode->isA(NodeTypePropertyName)) {
                std::cerr << "!!!!!!! FIRST NODE IS NOT PROPERTY NAME FAIL !!!!!!!\n";
                continue;
            }
            auto pn = std::static_pointer_cast<LILPropertyName>(firstNode);
            pnName = pn->getName();
        } else if (asSubject->isA(NodeTypeVarName)){
            pnName = std::static_pointer_cast<LILVarName>(asSubject)->getName();
        } else if (asSubject->isA(NodeTypePropertyName)){
            pnName = std::static_pointer_cast<LILPropertyName>(asSubject)->getName();
        }

        bool fieldFound = false;
        for (auto clField : clFields) {
            if (!clField->isA(NodeTypeVarDecl)) {
                std::cerr << "!!!!!!! FIELD IN CLASS IS NOT VAR DECL FAIL !!!!!!!\n";
                continue;
            }
            auto vd = std::static_pointer_cast<LILVarDecl>(clField);
            if (pnName == vd->getName()) {
                fieldFound = true;
                auto vdTy = vd->getType();
                auto asTy = as->getType();
                if (!vdTy->equalTo(asTy)) {
                    LILErrorMessage ei;
                    ei.message =  "The field "+pnName+" needs to be of type "+LILNodeToString::stringify(vdTy.get())+", "+LILNodeToString::stringify(asTy.get())+" was given instead";
                    LILNode::SourceLocation sl = as->getSourceLocation();
                    ei.file = sl.file;
                    ei.line = sl.line;
                    ei.column = sl.column;
                    this->errors.push_back(ei);
                }
            }
        }
        if (!fieldFound) {
            LILErrorMessage ei;
            ei.message =  "The field "+pnName+" was not found on class @"+classValue->getName();
            LILNode::SourceLocation sl = as->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        }
    }
}

void LILTypeValidator::_validate(std::shared_ptr<LILVarDecl> vd)
{
    auto ty = vd->getType();
    auto initVal = vd->getInitVal();
    if (ty && !ty->isA(TypeTypeFunction) && initVal) {
        auto ivTy = initVal->getType();
        if (!ivTy) {
            LILErrorMessage ei;
            ei.message =  "FATAL ERROR: type of value of " + vd->getName() + " was null";
            LILNode::SourceLocation sl = initVal->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
            return;
        }
        
        bool found = false;
        if (ty->isA(TypeTypeMultiple)) {
            auto multiTy = std::static_pointer_cast<LILMultipleType>(ty);
            if (ivTy->isA(TypeTypeMultiple)) {
                bool allFound = true;
                auto ivMtTy = std::static_pointer_cast<LILMultipleType>(ivTy);
                for (auto ivMtTy : ivMtTy->getTypes()) {
                    found = false;
                    for (auto mtTy : multiTy->getTypes()) {
                        if (mtTy->equalTo(ivMtTy)) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        allFound = false;
                        break;
                    }
                }
                found = allFound;
            } else if (ivTy->getIsNullable()) {
                ivTy->setIsNullable(false);
                for (auto mtTy : multiTy->getTypes()) {
                    if (mtTy->equalTo(ivTy)) {
                        found = true;
                        break;
                    }
                }
                ivTy->setIsNullable(true);

            } else {
                if (ty->getIsNullable() && ivTy->getName() == "null") {
                    found = true;
                } else {
                    for (auto mtTy : multiTy->getTypes()) {
                        if (mtTy->equalTo(ivTy)) {
                            found = true;
                            break;
                        }
                    }
                }
            }
        } else if (ty->getIsNullable() && !ivTy->getIsNullable()) {
            ty->setIsNullable(false);
            found = ty->equalTo(ivTy);
            ty->setIsNullable(true);
        } else {
            found = this->typesCompatible(ty, ivTy);
        }
        if (!found) {
            LILErrorMessage ei;
            ei.message =  "Type mismatch: cannot assign type "+LILNodeToString::stringify(ivTy.get())+" to var."+LILNodeToString::stringify(ty.get()) + " " + vd->getName();
            LILNode::SourceLocation sl = initVal->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
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

bool LILTypeValidator::typesCompatible(const std::shared_ptr<LILType> & ty1, const std::shared_ptr<LILType> & ty2)
{
    switch (ty1->getTypeType())
    {
        case TypeTypePointer:
        {
            if (!ty2->isA(TypeTypePointer)) {
                return false;
            }
            auto ty1p = std::static_pointer_cast<LILPointerType>(ty1);
            auto arg = ty1p->getArgument();
            if (arg) {
                if (arg->getName() == "any") {
                    return true;
                } else {
                    auto ty2p = std::static_pointer_cast<LILPointerType>(ty2);
                    auto arg2 = ty2p->getArgument();
                    if (arg2->getName() == "any") {
                        return true;
                    }

                    return ty1->equalTo(ty2);
                }
            }
            break;
        }
            
        default:
            return ty1->equalTo(ty2);
    }
    return false;
}
