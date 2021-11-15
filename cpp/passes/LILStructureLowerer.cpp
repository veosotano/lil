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
 *      This file disambiguates nodes in the AST with multiple types
 *
 ********************************************************************/

#include "LILStructureLowerer.h"
#include "LILNodeToString.h"
#include "LILVarNode.h"

using namespace LIL;

LILStructureLowerer::LILStructureLowerer()
{
}

LILStructureLowerer::~LILStructureLowerer()
{
}

void LILStructureLowerer::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "===  STRUCTURE LOWERING  ===\n";
        std::cerr << "============================\n\n";
    }
}

void LILStructureLowerer::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);

    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();

    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->process(node);
        if (this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            //out with the old
            switch (node->getNodeType()) {
                case NodeTypeVarDecl:
                {
                    auto vd = std::static_pointer_cast<LILVarDecl>(node);
                    rootNode->unsetLocalVariable(vd->getName());
                    break;
                }
                case NodeTypeFunctionDecl:
                {
                    auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
                    rootNode->unsetLocalVariable(fd->getName());
                    break;
                }
                default:
                    break;
            }
            rootNode->removeNode(node);
            //in with the new
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
                switch (newNode->getNodeType()) {
                    case NodeTypeVarDecl:
                    {
                        auto vd = std::static_pointer_cast<LILVarDecl>(newNode);
                        rootNode->setLocalVariable(vd->getName(), vd);
                        break;
                    }
                    case NodeTypeFunctionDecl:
                    {
                        auto fd = std::static_pointer_cast<LILFunctionDecl>(newNode);
                        rootNode->setLocalVariable(fd->getName(), fd);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        this->_nodeBuffer.pop_back();
    }
    rootNode->setChildNodes(std::move(resultNodes));
}

void LILStructureLowerer::visit(LILNode *node)
{
    this->process(node->shared_from_this());
}

void LILStructureLowerer::process(std::shared_ptr<LILNode> node)
{
    if (this->getDebug()) {
        std::cerr << "## lowering structure " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBoolLiteral:
        {
            std::shared_ptr<LILBoolLiteral> value = std::static_pointer_cast<LILBoolLiteral>(node);
            return this->_process(value);
        }
        case NodeTypeNumberLiteral:
        {
            std::shared_ptr<LILNumberLiteral> value = std::static_pointer_cast<LILNumberLiteral>(node);
            return this->_process(value);
        }
        case NodeTypePercentage:
        {
            std::shared_ptr<LILPercentageLiteral> value = std::static_pointer_cast<LILPercentageLiteral>(node);
            return this->_process(value);
        }
        case NodeTypeExpression:
        {
            std::shared_ptr<LILExpression> value = std::static_pointer_cast<LILExpression>(node);
            return this->_process(value);
        }
        case NodeTypeUnaryExpression:
        {
            std::shared_ptr<LILUnaryExpression> value = std::static_pointer_cast<LILUnaryExpression>(node);
            return this->_process(value);
        }
        case NodeTypeStringLiteral:
        {
            std::shared_ptr<LILStringLiteral> value = std::static_pointer_cast<LILStringLiteral>(node);
            return this->_process(value);
        }
        case NodeTypeStringFunction:
        {
            std::shared_ptr<LILStringFunction> value = std::static_pointer_cast<LILStringFunction>(node);
            return this->_process(value);
        }
        case NodeTypeNull:
        {
            std::shared_ptr<LILNullLiteral> value = std::static_pointer_cast<LILNullLiteral>(node);
            return this->_process(value);
        }
        case NodeTypeVarDecl:
        {
            std::shared_ptr<LILVarDecl> value = std::static_pointer_cast<LILVarDecl>(node);
            return this->_process(value);
        }
        case NodeTypeAliasDecl:
        case NodeTypeTypeDecl:
        case NodeTypeConversionDecl:
        {
            //do nothing
            break;
        }
        case NodeTypeClassDecl:
        {
            std::shared_ptr<LILClassDecl> value = std::static_pointer_cast<LILClassDecl>(node);
            return this->_process(value);
        }
        case NodeTypeObjectDefinition:
        {
            std::shared_ptr<LILObjectDefinition> value = std::static_pointer_cast<LILObjectDefinition>(node);
            return this->_process(value);
        }
        case NodeTypeAssignment:
        {
            std::shared_ptr<LILAssignment> value = std::static_pointer_cast<LILAssignment>(node);
            return this->_process(value);
        }
        case NodeTypeValuePath:
        {
            std::shared_ptr<LILValuePath> value = std::static_pointer_cast<LILValuePath>(node);
            return this->_process(value);
        }
        case NodeTypePropertyName:
        {
            std::shared_ptr<LILPropertyName> value = std::static_pointer_cast<LILPropertyName>(node);
            return this->_process(value);
        }
        case NodeTypeVarName:
        {
            std::shared_ptr<LILVarName> value = std::static_pointer_cast<LILVarName>(node);
            return this->_process(value);
        }
        case NodeTypeRule:
        {
            std::shared_ptr<LILRule> value = std::static_pointer_cast<LILRule>(node);
            return this->_process(value);
        }
        case NodeTypeSimpleSelector:
        {
            std::shared_ptr<LILSimpleSelector> value = std::static_pointer_cast<LILSimpleSelector>(node);
            return this->_process(value);
        }
        case NodeTypeSelectorChain:
        {
            std::shared_ptr<LILSelectorChain> value = std::static_pointer_cast<LILSelectorChain>(node);
            return this->_process(value);
        }
        case NodeTypeSelector:
        {
            std::shared_ptr<LILSelector> value = std::static_pointer_cast<LILSelector>(node);
            return this->_process(value);
        }
        case NodeTypeCombinator:
        {
            std::shared_ptr<LILCombinator> value = std::static_pointer_cast<LILCombinator>(node);
            return this->_process(value);
        }
        case NodeTypeFilter:
        {
            std::shared_ptr<LILFilter> value = std::static_pointer_cast<LILFilter>(node);
            return this->_process(value);
        }
        case NodeTypeFlag:
        {
            std::shared_ptr<LILFlag> value = std::static_pointer_cast<LILFlag>(node);
            return this->_process(value);
        }
        case NodeTypeFunctionDecl:
        {
            std::shared_ptr<LILFunctionDecl> value = std::static_pointer_cast<LILFunctionDecl>(node);
            return this->_process(value);
        }
        case NodeTypeFunctionCall:
        {
            std::shared_ptr<LILFunctionCall> value = std::static_pointer_cast<LILFunctionCall>(node);
            return this->_process(value);
        }
        case NodeTypeFlowControl:
        {
            std::shared_ptr<LILFlowControl> value = std::static_pointer_cast<LILFlowControl>(node);
            return this->_process(value);
        }
        case NodeTypeFlowControlCall:
        {
            std::shared_ptr<LILFlowControlCall> value = std::static_pointer_cast<LILFlowControlCall>(node);
            return this->_process(value);
        }
        case NodeTypeInstruction:
        {
            std::shared_ptr<LILInstruction> value = std::static_pointer_cast<LILInstruction>(node);
            return this->_process(value);
        }
        case NodeTypeIfInstruction:
        {
            std::shared_ptr<LILIfInstruction> value = std::static_pointer_cast<LILIfInstruction>(node);
            return this->_process(value);
        }
        case NodeTypeValueList:
        {
            std::shared_ptr<LILValueList> value = std::static_pointer_cast<LILValueList>(node);
            return this->_process(value);
        }
        case NodeTypeDocumentation:
        {
            std::shared_ptr<LILDocumentation> value = std::static_pointer_cast<LILDocumentation>(node);
            return this->_process(value);
        }
        case NodeTypeIndexAccessor:
        {
            std::shared_ptr<LILIndexAccessor> value = std::static_pointer_cast<LILIndexAccessor>(node);
            return this->_process(value);
        }

        default:
            std::cerr << "Error: unkonwn node type to process\n";
            break;
    }

}

void LILStructureLowerer::_process(std::shared_ptr<LILBoolLiteral> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILNumberLiteral> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILPercentageLiteral> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILExpression> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILUnaryExpression> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILStringLiteral> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILStringFunction> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILNullLiteral> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILVarDecl> value)
{
    auto initVal = value->getInitVal();
    if (initVal) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->process(initVal);
        if (this->_nodeBuffer.back().size() > 0) {
            value->setChildNodes(std::move(this->_nodeBuffer.back()));
        }
        this->_nodeBuffer.pop_back();
    }
}

void LILStructureLowerer::_process(std::shared_ptr<LILClassDecl> value)
{
    if (value->getIsExtern()) {
        return;
    }

    for (const auto & methodPair : value->getMethods()) {
        auto methodName = methodPair.first;
        this->_nodeBuffer.emplace_back();
        this->process(methodPair.second);
        if (this->_nodeBuffer.back().size() > 0) {
            auto newMethod = this->_nodeBuffer.back().back();
            value->addMethod(methodName, newMethod);
        }
        this->_nodeBuffer.pop_back();
    }
}

void LILStructureLowerer::_process(std::shared_ptr<LILObjectDefinition> value)
{
    this->processChildren(value->getNodes());
}

void LILStructureLowerer::_process(std::shared_ptr<LILAssignment> value)
{
    this->processChildren(value->getNodes());
}

void LILStructureLowerer::_process(std::shared_ptr<LILValuePath> value)
{
    this->processChildren(value->getNodes());
}

void LILStructureLowerer::_process(std::shared_ptr<LILPropertyName> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILRule> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILSimpleSelector> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILSelectorChain> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILSelector> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILCombinator> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILFilter> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILFlag> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILVarName> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILFunctionDecl> value)
{
    switch (value->getFunctionDeclType()) {
        case FunctionDeclTypeFn:
        {
            auto ty = value->getType();

            if (ty && ty->getTypeType() == TypeTypeFunction) {
                auto fnTy = std::static_pointer_cast<LILFunctionType>(ty);
                for (auto arg : fnTy->getArguments()) {
                    std::shared_ptr<LILType> tyArg;
                    if (arg->isA(NodeTypeType)) {
                        tyArg = std::static_pointer_cast<LILType>(arg);
                    } else if (arg->isA(NodeTypeVarDecl)){
                        tyArg = arg->getType();
                    }

                    if ((tyArg->getTypeType() == TypeTypeMultiple) && !tyArg->getIsWeakType()) {
                        
                        auto newFd = std::make_shared<LILFunctionDecl>();
                        newFd->setFunctionDeclType(FunctionDeclTypeFn);
                        newFd->setIsExtern(value->getIsExtern());
                        newFd->setIsExported(value->getIsExported());

                        auto newFnType = fnTy->clone();
                        newFd->setType(newFnType);
                        
                        newFd->setHasMultipleImpls(true);
                        newFd->setName(value->getName());

                        this->_nodeBuffer.back().push_back(newFd);
                        auto tyArgTypes = std::static_pointer_cast<LILMultipleType>(tyArg)->getTypes();
                        if (tyArg->getIsNullable()) {
                            tyArgTypes.push_back(LILType::make("null"));
                        }
                        for (auto argChild : tyArgTypes) {
                            auto newChildFd = std::make_shared<LILFunctionDecl>();
                            newChildFd->setFunctionDeclType(FunctionDeclTypeFn);
                            newChildFd->setIsExtern(value->getIsExtern());
                            newChildFd->setIsExported(value->getIsExported());

                            auto newChildFnType = std::make_shared<LILFunctionType>();
                            auto returnTy = fnTy->getReturnType();
                            if (returnTy) {
                                newChildFnType->setReturnType(returnTy);
                            }
                            newChildFd->setType(newChildFnType);
                            
                            std::vector<std::shared_ptr<LILNode>> newArgs;
                            for (auto funArg : fnTy->getArguments()){
                                if (!funArg->isA(NodeTypeVarDecl)) {
                                    continue;
                                }
                                //disambiguate argument types
                                auto argClone = std::static_pointer_cast<LILVarDecl>(funArg)->clone();
                                auto funArgTy = funArg->getType();
                                if(funArgTy && funArgTy->getTypeType() == TypeTypeMultiple){
                                    argClone->setType(argChild);
                                    newArgs.push_back(argClone);
                                }
                                
                                //resolve "if cast" blocks
                                std::vector<std::shared_ptr<LILNode>> newBody;
                                for (auto node : value->getBody()) {
                                    auto newChildNodes = this->reduceIfCastBlocks(node, argClone->getName(), argChild);
                                    for (auto child : newChildNodes) {
                                        newBody.push_back(child);
                                    }
                                }
                                newChildFd->setBody(newBody);
                            }
                            
                            for (auto newArg : newArgs) {
                                newChildFnType->addArgument(newArg);
                                if (newArg->isA(NodeTypeVarDecl)) {
                                    auto newArgVd = std::static_pointer_cast<LILVarDecl>(newArg);
                                    newChildFd->setLocalVariable(newArgVd->getName(), newArgVd);
                                }
                                
                            }

                            newChildFd->setName(value->getName());
                            
                            newFd->addImpl(newChildFd);
                        } //end for
                    } //end if tyArg is multiple or not weak
                } //end for
            } //end if ty isa function type
            break;
        }

        default:
            break;
    }
    
    this->processChildren(value->getBody());
}
std::vector<std::shared_ptr<LILNode>> LILStructureLowerer::reduceIfCastBlocks(std::shared_ptr<LILNode> node, LILString argName, std::shared_ptr<LILType> ty)
{
    switch (node->getNodeType()) {
        case NodeTypeFlowControl:
        {
            bool doReduce = false;
            auto fc = std::static_pointer_cast<LILFlowControl>(node);
            if (fc->getFlowControlType() == FlowControlTypeIfCast) {
                auto fcArgName = fc->getArguments().front();
                if (fcArgName->isA(NodeTypeVarName)) {
                    doReduce = std::static_pointer_cast<LILVarName>(fcArgName)->getName() == argName;
                }
            }

            if (doReduce) {
                auto fcTyName = fc->getArguments().back();
                bool tyMatches = false;
                if (fcTyName->isA(NodeTypeType)) {
                    tyMatches = std::static_pointer_cast<LILType>(fcTyName)->equalTo(ty);
                }
                if (tyMatches) {
                    std::vector<std::shared_ptr<LILNode>> returnNodes;
                    for (auto thenNode : fc->getThen()) {
                        std::vector<std::shared_ptr<LILNode>> childNodes;
                        childNodes = this->reduceIfCastBlocks(thenNode, argName, ty);
                        for (auto childNode : childNodes) {
                            returnNodes.push_back(childNode);
                        }
                    }
                    return returnNodes;
                } else {
                    std::vector<std::shared_ptr<LILNode>> returnNodes;
                    for (auto elseNode : fc->getElse()) {
                        std::vector<std::shared_ptr<LILNode>> childNodes;
                        childNodes = this->reduceIfCastBlocks(elseNode, argName, ty);
                        for (auto childNode : childNodes) {
                            returnNodes.push_back(childNode);
                        }
                    }
                    return returnNodes;
                }

            }
            break;
        }

        default:
            break;
    }
    std::vector<std::shared_ptr<LILNode>> ret;
    ret.push_back(node->clone());
    return ret;
}

void LILStructureLowerer::_process(std::shared_ptr<LILFunctionCall> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILFlowControl> value)
{
    this->processChildren(value->getThen());
    this->processChildren(value->getElse());
}

void LILStructureLowerer::_process(std::shared_ptr<LILFlowControlCall> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->process(arg);
    }
}

void LILStructureLowerer::_process(std::shared_ptr<LILInstruction> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILIfInstruction> value)
{
    for (auto thenNode : value->getThen()) {
        this->process(thenNode);
    }
    for (auto elseNode : value->getElse()) {
        this->process(elseNode);
    }
}

void LILStructureLowerer::_process(std::shared_ptr<LILValueList> value)
{
    this->processChildren(value->getValues());
}

void LILStructureLowerer::_process(std::shared_ptr<LILDocumentation> value)
{
}

void LILStructureLowerer::_process(std::shared_ptr<LILIndexAccessor> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->process(arg);
    }
}


void LILStructureLowerer::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process(*it);
    };
}
