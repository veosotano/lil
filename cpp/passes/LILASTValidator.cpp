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
    if (this->getDebug()) {
        std::cerr << "## validating " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + node->stringRep().data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBool:
        {
            LILBoolLiteral * value = static_cast<LILBoolLiteral *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeNumberLiteral:
        {
            LILNumberLiteral * value = static_cast<LILNumberLiteral *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypePercentage:
        {
            LILPercentageLiteral * value = static_cast<LILPercentageLiteral *>(node);
            this->_validate(value);
            break;
            break;
        }
        case NodeTypeExpression:
        {
            LILExpression * value = static_cast<LILExpression *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeStringLiteral:
        {
            LILStringLiteral * value = static_cast<LILStringLiteral *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeStringFunction:
        {
            LILStringFunction * value = static_cast<LILStringFunction *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeNull:
        {
            LILNullLiteral * value = static_cast<LILNullLiteral *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeType:
        {
            LILType * value = static_cast<LILType *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeVarDecl:
        {
            LILVarDecl * value = static_cast<LILVarDecl *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeClassDecl:
        {
            LILClassDecl * value = static_cast<LILClassDecl *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeObjectDefinition:
        {
            LILObjectDefinition * value = static_cast<LILObjectDefinition *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeAssignment:
        {
            LILAssignment * value = static_cast<LILAssignment *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeValuePath:
        {
            LILValuePath * value = static_cast<LILValuePath *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypePropertyName:
        {
            LILPropertyName * value = static_cast<LILPropertyName *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeVarName:
        {
            LILVarName * value = static_cast<LILVarName *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeRule:
        {
            LILRule * value = static_cast<LILRule *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeSimpleSelector:
        {
            LILSimpleSelector * value = static_cast<LILSimpleSelector *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeSelectorChain:
        {
            LILSelectorChain * value = static_cast<LILSelectorChain *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeSelector:
        {
            LILSelector * value = static_cast<LILSelector *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeCombinator:
        {
            LILCombinator * value = static_cast<LILCombinator *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFilter:
        {
            LILFilter * value = static_cast<LILFilter *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFlag:
        {
            LILFlag * value = static_cast<LILFlag *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            LILFunctionDecl * value = static_cast<LILFunctionDecl *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFunctionCall:
        {
            LILFunctionCall * value = static_cast<LILFunctionCall *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFlowControl:
        {
            LILFlowControl * value = static_cast<LILFlowControl *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFlowControlCall:
        {
            LILFlowControlCall * value = static_cast<LILFlowControlCall *>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeInstruction:
        {
            LILInstruction * value = static_cast<LILInstruction *>(node);
            this->_validate(value);
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

void LILASTValidator::_validate(LILBoolLiteral * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILNumberLiteral * value)
{
    if (value->getValue().length() == 0) {
        LILErrorMessage ei;
        ei.message =  "Number literal was emtpy";
        LILNode::SourceLocation sl = value->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else {
        if (this->getDebug()) {
            std::cerr << "Number literal was not empty. OK\n";
        }
    }
}

void LILASTValidator::_validate(LILPercentageLiteral * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILExpression * value)
{
    auto left = value->getLeft();
    switch (left->getNodeType()) {
        case NodeTypeNumberLiteral:
        case NodeTypeExpression:
        case NodeTypeValuePath:
        case NodeTypeVarName:
        case NodeTypeFunctionCall:
            break;
            
        default:
        {
            this->illegalNodeType(left.get(), value);
            break;
        }
    }
    auto right = value->getRight();
    switch (right->getNodeType()) {
        case NodeTypeNumberLiteral:
        case NodeTypeExpression:
        case NodeTypeValuePath:
        case NodeTypeVarName:
        case NodeTypeFunctionCall:
            break;
            
        default:
        {
            this->illegalNodeType(right.get(), value);
            break;
        }
    }
}

void LILASTValidator::_validate(LILStringLiteral * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILStringFunction * value)
{
    auto children = value->getChildNodes();
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
                this->illegalNodeType(children[i].get(), value);
                return;
            }
        }
    }
}

void LILASTValidator::_validate(LILNullLiteral * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILType * value)
{
    switch (value->getTypeType()) {
        case TypeTypeSingle:
        {
            auto name = value->getName();
            if (
                name != "i8"
                && name != "i16"
                && name != "i32"
                && name != "i64"
                && name != "i128"
                && name != "f32"
                && name != "f64"
                && name != "bool"
                && name != "cstr"
                && name != "str"
                ) {
                LILErrorMessage ei;
                ei.message =  "Invalid type name "+name;
                LILNode::SourceLocation sl = value->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            }
            break;
        }
        case TypeTypeObject:
        {
            auto objTy = static_cast<LILObjectType *>(value);
            auto fields = objTy->getFields();
            if (fields.size() == 0) {
                LILErrorMessage ei;
                ei.message =  "Object types need at least one field";
                LILNode::SourceLocation sl = value->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            } else if (this->getDebug()) {
                std::cerr << "Object type had " << fields.size() << " fields. OK\n";
            }
            for (auto field : fields) {
                if (!field->LILNode::isA(NodeTypeType)) {
                    this->illegalNodeType(field.get(), value);
                }
            }
            if (this->getDebug() && !this->hasErrors()) {
                std::cerr << "All subtypes in the type are types. OK\n";
            }
            break;
        }
            
        case TypeTypePointer:
        {
            auto ptrTy = static_cast<LILPointerType *>(value);
            auto arg = ptrTy->getArgument();
            if (!arg) {
                LILErrorMessage ei;
                ei.message =  "Pointer types need a subtype";
                LILNode::SourceLocation sl = value->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            }
            if (!arg->LILNode::isA(NodeTypeType)) {
                    this->illegalNodeType(arg.get(), value);
            }
            if (this->getDebug() && !this->hasErrors()) {
                std::cerr << "The subtype in the type is a type. OK\n";
            }
            break;
        }
            
        default:
            if (this->getDebug() && !this->hasErrors()) {
                std::cerr << "Nothing to do. OK\n";
            }
            break;
    }
}


void LILASTValidator::_validate(LILVarDecl * value)
{
    auto ty = value->getType();
    if (ty) {
        this->validate(ty.get());
    }
}

void LILASTValidator::_validate(LILClassDecl * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILObjectDefinition * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILAssignment * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILValuePath * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILPropertyName * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILRule * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILSimpleSelector * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILSelectorChain * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILSelector * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILCombinator * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILFilter * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILFlag * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILVarName * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILFunctionDecl * value)
{
    switch (value->getFunctionDeclType()) {
        case FunctionDeclTypeFn:
        {
            auto evals = value->getBody();
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
                                this->illegalNodeType(evals[i].get(), value);
                                break;
                        }

                        break;
                    }

                    default:
                    {
                        this->illegalNodeType(evals[i].get(), value);
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

void LILASTValidator::_validate(LILFunctionCall * value)
{
    switch (value->getFunctionCallType()) {
        case FunctionCallTypeValuePath:
        {
            auto grandpa = value->getParentNode();
            if (grandpa && grandpa->isA(NodeTypeValuePath)) {
                auto vp = std::static_pointer_cast<LILValuePath>(grandpa);
                auto firstNode = vp->getNodes().front();
                if (firstNode->isA(NodeTypeVarName)) {
                    auto varName = std::static_pointer_cast<LILVarName>(firstNode);
                    auto fn = this->findNodeForVarName(varName.get());
                    if (!fn) {
                        LILErrorMessage ei;
                        ei.message =  "Function "+varName->getName()+" not found";
                        LILNode::SourceLocation sl = vp->getSourceLocation();
                        ei.file = sl.file;
                        ei.line = sl.line;
                        ei.column = sl.column;
                        this->errors.push_back(ei);
                    }
                }
            }
            break;
        }
        case FunctionCallTypePointerTo:
        {
            if (value->getArguments().size() != 1) {
                LILErrorMessage ei;
                ei.message =  "Call to pointerTo() needs one argument";
                LILNode::SourceLocation sl = value->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            }
            break;
        }
        case FunctionCallTypeValueOf:
        {
            if (value->getArguments().size() != 1) {
                LILErrorMessage ei;
                ei.message =  "Call to valueOf() needs one argument";
                LILNode::SourceLocation sl = value->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            }
            break;
        }
        case FunctionCallTypeSet:
        {
            if (value->getArguments().size() != 2) {
                LILErrorMessage ei;
                ei.message =  "Call to set() needs two arguments";
                LILNode::SourceLocation sl = value->getSourceLocation();
                ei.file = sl.file;
                ei.line = sl.line;
                ei.column = sl.column;
                this->errors.push_back(ei);
            }
            break;
        }
        default:
            break;
    }
}

void LILASTValidator::_validate(LILFlowControl * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(LILFlowControlCall * value)
{
    auto arg = value->getArgument();
    if (arg) {
        switch (arg->getNodeType()) {
            case NodeTypeNumberLiteral:
            case NodeTypeStringLiteral:
            case NodeTypeStringFunction:
            case NodeTypeBool:
            case NodeTypeExpression:
            case NodeTypeArray:
            case NodeTypeVarName:
            case NodeTypeValuePath:
            case NodeTypeSelector:
            case NodeTypeObjectDefinition:
            case NodeTypeFunctionCall:
                break;
            default:
                this->illegalNodeType(arg.get(), value);
                break;
        }
        if (this->getDebug() && !this->hasErrors()) {
            std::cerr << "The argument is a " + LILNode::nodeTypeToString(arg->getNodeType()).data() +". OK\n";
        }
    }
}

void LILASTValidator::_validate(LILInstruction * value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::validateChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->validate((*it).get());
    };
}
