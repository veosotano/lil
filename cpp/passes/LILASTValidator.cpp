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
 *      This file checks the AST for logical coherence
 *
 ********************************************************************/

#include "LILASTValidator.h"

using namespace LIL;

LILASTValidator::LILASTValidator()
: _debug(false)
{
}

LILASTValidator::~LILASTValidator()
{
}

void LILASTValidator::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "=====  AST VALIDATION  =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILASTValidator::visit(LILNode *node)
{
    this->validate(node);
}

void LILASTValidator::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    LILVisitor::performVisit(rootNode);
    if (this->getVerbose() && !this->hasErrors()) {
        std::cerr << "All OK\n\n";
    }
}

void LILASTValidator::illegalNodeType(LILNode* illegalNode, LILNode* container)
{
    LILErrorMessage ei;
    ei.message =  "A node of type " + LILNode::nodeTypeToString(container->getNodeType()) + " can't contain a node of type " + LILNode::nodeTypeToString(illegalNode->getNodeType());
    LILNode::SourceLocation sl = illegalNode->getSourceLocation();
    ei.file = sl.file;
    ei.line = sl.line;
    ei.column = sl.column;
    this->errors.push_back(ei);
}

void LILASTValidator::validate(LILNode * node)
{
    if (this->_debug) {
        std::cerr << "## validating " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBool:
        {
            LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeNumberLiteral:
        {
            LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypePercentage:
        {
            LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
            this->validate(*value);
            break;
            break;
        }
        case NodeTypeExpression:
        {
            LILExpression * value = static_cast<LILExpression *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeStringLiteral:
        {
            LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeStringFunction:
        {
            LILStringFunction * value = static_cast<LILStringFunction *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeNull:
        {
            LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeVarDecl:
        {
            LILVarDecl * value = static_cast<LILVarDecl *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeClassDecl:
        {
            LILClassDecl * value = static_cast<LILClassDecl *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeObjectDefinition:
        {
            LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeAssignment:
        {
            LILAssignment * value = static_cast<LILAssignment *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeValuePath:
        {
            LILValuePath * value = static_cast<LILValuePath *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypePropertyName:
        {
            LILPropertyName * value = static_cast<LILPropertyName *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeVarName:
        {
            LILVarName * value = static_cast<LILVarName *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeRule:
        {
            LILRule * value = static_cast<LILRule *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeSimpleSelector:
        {
            LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeSelectorChain:
        {
            LILSelectorChain * value = static_cast<LILSelectorChain *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeSelector:
        {
            LILSelector * value = static_cast<LILSelector *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeCombinator:
        {
            LILCombinator * value = static_cast<LILCombinator *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeFilter:
        {
            LILFilter * value = static_cast<LILFilter *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeFlag:
        {
            LILFlag * value = static_cast<LILFlag *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeFunctionCall:
        {
            LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeFlowControl:
        {
            LILFlowControl * value = static_cast<LILFlowControl *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeFlowControlCall:
        {
            LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            this->validate(*value);
            break;
        }
        case NodeTypeType:
        {
            break;
        }

        default:
            std::cerr << "Error: unkonwn node type to validate\n";
            break;
    }
    if (LILNode::isContainerNode(node->getNodeType())) {
        this->validateChildren(node->getChildNodes());
    }
}

void LILASTValidator::validate(LILBoolLiteral value)
{
    //nothing to validate
}

void LILASTValidator::validate(LILNumberLiteral value)
{
    if (value.getValue().length() == 0) {
        LILErrorMessage ei;
        ei.message =  "Number literal was emtpy";
        LILNode::SourceLocation sl = value.getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    }
}

void LILASTValidator::validate(LILPercentageLiteral value)
{
    //todo
}

void LILASTValidator::validate(LILExpression value)
{
    auto left = value.getLeft();
    switch (left->getNodeType()) {
        case NodeTypeNumberLiteral:
        case NodeTypeExpression:
        case NodeTypeValuePath:
        case NodeTypeFunctionCall:
            break;
            
        default:
        {
            this->illegalNodeType(left.get(), &value);
            break;
        }
    }
    auto right = value.getRight();
    switch (right->getNodeType()) {
        case NodeTypeNumberLiteral:
        case NodeTypeExpression:
        case NodeTypeValuePath:
        case NodeTypeFunctionCall:
            break;
            
        default:
        {
            this->illegalNodeType(right.get(), &value);
            break;
        }
    }
}

void LILASTValidator::validate(LILStringLiteral value)
{

}

void LILASTValidator::validate(LILStringFunction value)
{
    auto children = value.getChildNodes();
    for (size_t i=0, j=children.size(); i<j; ++i) {
        NodeType childType = children[i]->getNodeType();
        switch (childType) {
            case NodeTypeNull:
            case NodeTypeBool:
            case NodeTypeNumberLiteral:
            case NodeTypePercentage:
            case NodeTypeVarName:
            case NodeTypeValuePath:
            case NodeTypeExpression:
            case NodeTypeNegation:
            case NodeTypeFunctionCall:
            case NodeTypeStringLiteral:
            case NodeTypeStringFunction:
            {
                break;
            }
            default:
            {
                this->illegalNodeType(children[i].get(), &value);
                return;
            }
        }
    }
}

void LILASTValidator::validate(LILNullLiteral value)
{

}

void LILASTValidator::validate(LILVarDecl value)
{

}

void LILASTValidator::validate(LILClassDecl value)
{

}

void LILASTValidator::validate(LILObjectDefinition value)
{

}

void LILASTValidator::validate(LILAssignment value)
{

}

void LILASTValidator::validate(LILValuePath value)
{

}

void LILASTValidator::validate(LILPropertyName value)
{
}

void LILASTValidator::validate(LILRule value)
{

}

void LILASTValidator::validate(LILSimpleSelector value)
{

}

void LILASTValidator::validate(LILSelectorChain value)
{

}

void LILASTValidator::validate(LILSelector value)
{
}

void LILASTValidator::validate(LILCombinator value)
{
}

void LILASTValidator::validate(LILFilter value)
{
}

void LILASTValidator::validate(LILFlag value)
{
}

void LILASTValidator::validate(LILVarName value)
{
}

void LILASTValidator::validate(LILFunctionDecl value)
{
    switch (value.getFunctionDeclType()) {
        case FunctionDeclTypeFn:
        {
            auto args = value.getArguments();
            for (size_t i=0, j=args.size(); i<j; ++i) {
                NodeType argType = args[i]->getNodeType();
                if (argType != NodeTypeVarDecl) {
                    LILErrorMessage ei;
                    ei.message =  "Functions only accept var declarations as arguments, found " + LILNode::nodeTypeToString(args[i]->getNodeType()) + " instead.";
                    LILNode::SourceLocation loc = args[i]->getSourceLocation();
                    ei.file = loc.file;
                    ei.line = loc.line;
                    ei.column = loc.column;
                    this->errors.push_back(ei);
                }
            }

            auto evals = value.getBody();
            for (size_t i=0, j=evals.size(); i<j; ++i) {
                NodeType evalType = evals[i]->getNodeType();
                switch (evalType) {
                    case NodeTypeValuePath:
                    case NodeTypeFunctionCall:
                    case NodeTypeVarDecl:
                    case NodeTypeAssignment:
                    case NodeTypeUnaryExpression:
                    case NodeTypeFlowControlCall:
                    case NodeTypeFlowControl:
                    {
                        break;
                    }
                    case NodeTypeFunctionDecl:
                    {
                        std::shared_ptr<LILFunctionDecl> fd = std::static_pointer_cast<LILFunctionDecl>(evals[i]);
                        switch (fd->getFunctionDeclType()) {
                            case FunctionDeclTypeInsert:
                            case FunctionDeclTypeOverride:
                            {
                                break;
                            }

                            default:
                                this->illegalNodeType(evals[i].get(), &value);
                                break;
                        }

                        break;
                    }

                    default:
                    {
                        this->illegalNodeType(evals[i].get(), &value);
                        return;
                    }
                }
            }
            break;
        }

        default:
            break;
    }
}

void LILASTValidator::validate(LILFunctionCall value)
{
    switch (value.getFunctionCallType()) {
        case FunctionCallTypeNone:
        {
            auto grandpa = value.getParentNode();
            if (grandpa && grandpa->isA(NodeTypeValuePath)) {
                auto vp = std::static_pointer_cast<LILValuePath>(grandpa);
                auto firstNode = vp->getNodes().front();
                if (firstNode->isA(NodeTypeVarName)) {
                    auto varName = std::static_pointer_cast<LILVarName>(firstNode);
                    auto fn = this->findNodeForVarName(varName.get());
                    if (!fn) {
                        LILErrorMessage ei;
                        ei.message =  "Function "+varName->getName()+" not found";
                        LILNode::SourceLocation sl = value.getSourceLocation();
                        ei.file = sl.file;
                        ei.line = sl.line;
                        ei.column = sl.column;
                        this->errors.push_back(ei);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

void LILASTValidator::validate(LILFlowControl value)
{
    
}

void LILASTValidator::validate(LILFlowControlCall value)
{
    
}

void LILASTValidator::validate(LILInstruction value)
{
}

void LILASTValidator::validateChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->validate((*it).get());
    };
}

void LILASTValidator::setDebug(bool value)
{
    this->_debug = value;
}
