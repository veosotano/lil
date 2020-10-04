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
        resultNodes.push_back(node);
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->process(node);
        for (auto newNode : this->_nodeBuffer.back()) {
            resultNodes.push_back(newNode);
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
        std::cerr << "## validating " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
        std::cerr << "## lowering structure " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBool:
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
            value->setChildNodes(this->_nodeBuffer.back());
        }
        this->_nodeBuffer.pop_back();
    }
}

void LILStructureLowerer::_process(std::shared_ptr<LILClassDecl> value)
{
    std::vector<std::shared_ptr<LILNode>> newNodes;

    std::vector<std::shared_ptr<LILNode>> nodes = value->getMethods();
    
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : nodes) {
        resultNodes.push_back(node);
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->process(node);
        for (auto newNode : this->_nodeBuffer.back()) {
            resultNodes.push_back(newNode);
            newNodes.push_back(newNode);
        }
        this->_nodeBuffer.pop_back();
    }
    if (newNodes.size() > 0) {
        
        for (auto newNode : newNodes) {
            value->addMethod(newNode);
        }
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
                for (auto tyArg : fnTy->getArguments()) {
                    if ((tyArg->getTypeType() == TypeTypeMultiple) && !tyArg->getIsWeakType()) {

                        for (auto argChild : std::static_pointer_cast<LILMultipleType>(tyArg)->getTypes()) {
                            auto newFd = std::make_shared<LILFunctionDecl>();
                            newFd->setFunctionDeclType(FunctionDeclTypeFn);
                            newFd->needsNameMangling = true;
                            auto newFnType = std::make_shared<LILFunctionType>();
                            newFnType->addArgument(argChild->clone());
                            newFnType->setReturnType(fnTy->getReturnType());
                            newFd->setType(newFnType);
                            newFd->setName(value->getName());

                            std::vector<std::shared_ptr<LILNode>> newArgs;
                            for (auto funArg : value->getArguments()){
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

                                //resolve "if is" blocks
                                std::vector<std::shared_ptr<LILNode>> newBody;
                                for (auto node : value->getBody()) {
                                    auto newChildNodes = this->reduceIfIsBlocks(node, argClone->getName(), argChild->getName());
                                    for (auto child : newChildNodes) {
                                        newBody.push_back(child);
                                    }
                                }
                                newFd->setBody(newBody);
                            }

                            for (auto newArg : newArgs) {
                                newFd->addArgument(newArg);
                            }

                            this->_nodeBuffer.back().push_back(newFd);
                        }

                    }
                }
            }
            break;
        }

        default:
            break;
    }
    
    this->processChildren(value->getBody());
}
std::vector<std::shared_ptr<LILNode>> LILStructureLowerer::reduceIfIsBlocks(std::shared_ptr<LILNode> node, LILString argName, LILString tyName)
{
    switch (node->getNodeType()) {
        case NodeTypeFlowControl:
        {
            bool doReduce = false;
            auto fc = std::static_pointer_cast<LILFlowControl>(node);
            if (fc->getFlowControlType() == FlowControlTypeIfIs) {
                auto fcArgName = fc->getArguments().front();
                if (fcArgName->isA(NodeTypeVarName)) {
                    doReduce = std::static_pointer_cast<LILVarName>(fcArgName)->getName() == argName;
                }
            }

            if (doReduce) {
                auto fcTyName = fc->getArguments().back();
                bool tyMatches = false;
                if (fcTyName->isA(NodeTypeType)) {
                    tyMatches = std::static_pointer_cast<LILType>(fcTyName)->getName() == tyName;
                }
                if (tyMatches) {
                    std::vector<std::shared_ptr<LILNode>> returnNodes;
                    for (auto thenNode : fc->getThen()) {
                        std::vector<std::shared_ptr<LILNode>> childNodes;
                        childNodes = this->reduceIfIsBlocks(thenNode, argName, tyName);
                        for (auto childNode : childNodes) {
                            returnNodes.push_back(childNode);
                        }
                    }
                    return returnNodes;
                } else {
                    std::vector<std::shared_ptr<LILNode>> returnNodes;
                    for (auto elseNode : fc->getElse()) {
                        std::vector<std::shared_ptr<LILNode>> childNodes;
                        childNodes = this->reduceIfIsBlocks(elseNode, argName, tyName);
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
    this->process(value->getArgument());
}

void LILStructureLowerer::_process(std::shared_ptr<LILInstruction> value)
{
}

void LILStructureLowerer::processChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->process(*it);
    };
}
