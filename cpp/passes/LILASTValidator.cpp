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
#include "LILIfInstruction.h"
#include "LILNodeToString.h"
#include "LILRootNode.h"

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

void LILASTValidator::performVisit(std::shared_ptr<LILRootNode> rootNode)
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

void LILASTValidator::validate(const std::shared_ptr<LILNode> & node)
{
    if (this->getDebug()) {
        std::cerr << "## validating " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeBoolLiteral:
        {
            std::shared_ptr<LILBoolLiteral> value = std::static_pointer_cast<LILBoolLiteral>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeNumberLiteral:
        {
            std::shared_ptr<LILNumberLiteral> value = std::static_pointer_cast<LILNumberLiteral>(node);
            this->_validate(value);
            break;
        }
        case NodeTypePercentage:
        {
            std::shared_ptr<LILPercentageLiteral> value = std::static_pointer_cast<LILPercentageLiteral>(node);
            this->_validate(value);
            break;
            break;
        }
        case NodeTypeExpression:
        {
            std::shared_ptr<LILExpression> value = std::static_pointer_cast<LILExpression>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeUnaryExpression:
        {
            std::shared_ptr<LILUnaryExpression> value = std::static_pointer_cast<LILUnaryExpression>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeStringLiteral:
        {
            std::shared_ptr<LILStringLiteral> value = std::static_pointer_cast<LILStringLiteral>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeStringFunction:
        {
            std::shared_ptr<LILStringFunction> value = std::static_pointer_cast<LILStringFunction>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeNull:
        {
            std::shared_ptr<LILNullLiteral> value = std::static_pointer_cast<LILNullLiteral>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeType:
        {
            std::shared_ptr<LILType> value = std::static_pointer_cast<LILType>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeVarDecl:
        {
            std::shared_ptr<LILVarDecl> value = std::static_pointer_cast<LILVarDecl>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeAliasDecl:
        {
            std::shared_ptr<LILAliasDecl> value = std::static_pointer_cast<LILAliasDecl>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeTypeDecl:
        {
            std::shared_ptr<LILTypeDecl> value = std::static_pointer_cast<LILTypeDecl>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeConversionDecl:
        {
            std::shared_ptr<LILConversionDecl> value = std::static_pointer_cast<LILConversionDecl>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeClassDecl:
        {
            std::shared_ptr<LILClassDecl> value = std::static_pointer_cast<LILClassDecl>(node);
            this->_validate(value);
            this->enterClassContext(value);
            break;
        }
        case NodeTypeObjectDefinition:
        {
            std::shared_ptr<LILObjectDefinition> value = std::static_pointer_cast<LILObjectDefinition>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeAssignment:
        {
            std::shared_ptr<LILAssignment> value = std::static_pointer_cast<LILAssignment>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeValuePath:
        {
            std::shared_ptr<LILValuePath> value = std::static_pointer_cast<LILValuePath>(node);
            this->_validate(value);
            break;
        }
        case NodeTypePropertyName:
        {
            std::shared_ptr<LILPropertyName> value = std::static_pointer_cast<LILPropertyName>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeVarName:
        {
            std::shared_ptr<LILVarName> value = std::static_pointer_cast<LILVarName>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeRule:
        {
            std::shared_ptr<LILRule> value = std::static_pointer_cast<LILRule>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeSimpleSelector:
        {
            std::shared_ptr<LILSimpleSelector> value = std::static_pointer_cast<LILSimpleSelector>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeSelectorChain:
        {
            std::shared_ptr<LILSelectorChain> value = std::static_pointer_cast<LILSelectorChain>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeSelector:
        {
            std::shared_ptr<LILSelector> value = std::static_pointer_cast<LILSelector>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeCombinator:
        {
            std::shared_ptr<LILCombinator> value = std::static_pointer_cast<LILCombinator>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFilter:
        {
            std::shared_ptr<LILFilter> value = std::static_pointer_cast<LILFilter>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFlag:
        {
            std::shared_ptr<LILFlag> value = std::static_pointer_cast<LILFlag>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            std::shared_ptr<LILFunctionDecl> value = std::static_pointer_cast<LILFunctionDecl>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFunctionCall:
        {
            std::shared_ptr<LILFunctionCall> value = std::static_pointer_cast<LILFunctionCall>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFlowControl:
        {
            std::shared_ptr<LILFlowControl> value = std::static_pointer_cast<LILFlowControl>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeFlowControlCall:
        {
            std::shared_ptr<LILFlowControlCall> value = std::static_pointer_cast<LILFlowControlCall>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeInstruction:
        {
            std::shared_ptr<LILInstruction> value = std::static_pointer_cast<LILInstruction>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeIfInstruction:
        {
            std::shared_ptr<LILIfInstruction> value = std::static_pointer_cast<LILIfInstruction>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeForeignLang:
        {
            std::shared_ptr<LILForeignLang> value = std::static_pointer_cast<LILForeignLang>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeDocumentation:
        {
            std::shared_ptr<LILDocumentation> value = std::static_pointer_cast<LILDocumentation>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeValueList:
        {
            std::shared_ptr<LILValueList> value = std::static_pointer_cast<LILValueList>(node);
            this->_validate(value);
            break;
        }
        case NodeTypeIndexAccessor:
        {
            std::shared_ptr<LILIndexAccessor> value = std::static_pointer_cast<LILIndexAccessor>(node);
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
    if (node->isA(NodeTypeClassDecl)) {
        this->exitClassContext();
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILBoolLiteral> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILNumberLiteral> & value)
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

void LILASTValidator::_validate(const std::shared_ptr<LILPercentageLiteral> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILExpression> & value)
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
            this->illegalNodeType(left.get(), value.get());
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
            
        case NodeTypeType:
        {
            if (!value->isA(ExpressionTypeCast)) {
                this->illegalNodeType(right.get(), value.get());
            }
            break;
        }
            
        default:
        {
            this->illegalNodeType(right.get(), value.get());
            break;
        }
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILUnaryExpression> & value)
{
    auto parent = value->getParentNode();
    if (!parent->isA(NodeTypeRule)) {
        auto val = value->getValue();
        switch (val->getNodeType()) {
            case NodeTypeNumberLiteral:
            case NodeTypeExpression:
            case NodeTypeValuePath:
            case NodeTypeVarName:
            case NodeTypeFunctionCall:
                break;
                
            default:
            {
                this->illegalNodeType(val.get(), value.get());
                break;
            }
        }
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILStringLiteral> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILStringFunction> & value)
{
    auto children = value->getChildNodes();
    for (size_t i=0, j=children.size(); i<j; ++i) {
        NodeType childType = children[i]->getNodeType();
        switch (childType) {
            case NodeTypeNull:
            case NodeTypeBoolLiteral:
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
                this->illegalNodeType(children[i].get(), value.get());
                return;
            }
        }
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILNullLiteral> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILType> & value)
{
    switch (value->getTypeType()) {
        case TypeTypeSingle:
        {
            auto name = value->getName();
            if (!LILType::isBuiltInType(value.get()) && !this->_isCustomType(value)) {
                LILErrorMessage ei;
                ei.message =  "Invalid type name "+value->getName();
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
            break;
        }
            
        case TypeTypePointer:
        {
            auto ptrTy = std::static_pointer_cast<LILPointerType>(value);
            auto arg = ptrTy->getArgument();
            if (arg && !arg->LILNode::isA(NodeTypeType)) {
                    this->illegalNodeType(arg.get(), value.get());
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

bool LILASTValidator::_isCustomType(const std::shared_ptr<LILType> & ty) const
{
    auto cd = this->getClassContext();
    if (cd) {
        for (auto alias : cd->getAliases()) {
            auto aTy = alias->getSrcType();
            if (aTy && aTy->equalTo(ty)) {
                return true;
            }
        }
    }
    auto rootNode = this->getRootNode();
    auto aliases = rootNode->getAliases();
    for (auto alias : aliases) {
        auto aTy = alias->getSrcType();
        if (aTy && aTy->equalTo(ty)) {
            return true;
        }
    }
    auto typeDecls = rootNode->getTypes();
    for (auto typeDecl : typeDecls) {
        auto tTy = typeDecl->getSrcType();
        if (tTy && tTy->equalTo(ty)) {
            return true;
        }
    }
    return false;
}

void LILASTValidator::_validate(const std::shared_ptr<LILVarDecl> & value)
{
    auto ty = value->getType();
    if (ty) {
        this->validate(ty);
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILAliasDecl> & value)
{
    if (!value->getSrcType()) {
        LILErrorMessage ei;
        ei.message =  "Alias declaration needs a source type";
        LILNode::SourceLocation sl = value->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    }
    if (!value->getDstType()) {
        LILErrorMessage ei;
        ei.message =  "Alias declaration needs a destination type";
        LILNode::SourceLocation sl = value->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILTypeDecl> & value)
{
    if (!value->getSrcType()) {
        LILErrorMessage ei;
        ei.message =  "Type declaration needs a source type";
        LILNode::SourceLocation sl = value->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    }
    
    const auto & parent = value->getParentNode();
    if (parent && parent->isRootNode()) {
        if (!value->getDstType()) {
            LILErrorMessage ei;
            ei.message =  "Type declaration needs a destination type";
            LILNode::SourceLocation sl = value->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        }
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILConversionDecl> & value)
{
    auto varDecl = value->getVarDecl();
    if (!varDecl) {
        LILErrorMessage ei;
        ei.message =  "Conversion declaration needs a variable declaration";
        LILNode::SourceLocation sl = value->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    }
    auto target = value->getType();
    if (!target) {
        LILErrorMessage ei;
        ei.message =  "Conversiond declaration needs a target type";
        LILNode::SourceLocation sl = value->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    }
    auto body = value->getBody();
    if (body.size() == 0) {
        LILErrorMessage ei;
        ei.message =  "Conversion declaration was empty";
        LILNode::SourceLocation sl = value->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILClassDecl> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILObjectDefinition> & value)
{
    auto ty = value->getType();
    if (!ty->isA(TypeTypeObject)) {
        this->illegalNodeType(ty.get(), value.get());
        return;
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILAssignment> & value)
{
    auto parent = value->getParentNode();
    if (parent && parent->isA(NodeTypeFunctionDecl)) {
        auto subject = value->getSubject();
        if (subject) {
            switch (subject->getNodeType()) {
                case NodeTypeValuePath:
                {
                    //fixme: validate this once types have been guessed
//                    auto remoteNode = this->findNodeForValuePath(static_cast<LILValuePath *>(subject.get()));
//                    if (!remoteNode) {
//                        LILErrorMessage ei;
//                        ei.message =  LILNodeToString::stringify(subject.get())+" not found";
//                        LILNode::SourceLocation sl = subject->getSourceLocation();
//                        ei.file = sl.file;
//                        ei.line = sl.line;
//                        ei.column = sl.column;
//                        this->errors.push_back(ei);
//                    }
                    break;
                }
                case NodeTypeVarName:
                {
                    auto remoteNode = this->findNodeForVarName(static_cast<LILVarName *>(subject.get()));
                    if (!remoteNode) {
                        LILErrorMessage ei;
                        ei.message =  LILNodeToString::stringify(subject.get())+" not found";
                        LILNode::SourceLocation sl = subject->getSourceLocation();
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
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILValuePath> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILPropertyName> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILRule> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILSimpleSelector> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILSelectorChain> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILSelector> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILCombinator> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILFilter> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILFlag> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILVarName> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILFunctionDecl> & value)
{
    switch (value->getFunctionDeclType()) {
        case FunctionDeclTypeFn:
        {
            auto evals = value->getBody();
            for (size_t i=0, j=evals.size(); i<j; ++i) {
                const auto & eval = evals[i];
                this->_validateFunctionDeclChild(value.get(), eval.get());
            }
            break;
        }

        default:
            break;
    }
}

void LILASTValidator::_validateFunctionDeclChild(LILFunctionDecl * value, LILNode *node)
{
    NodeType nodeType = node->getNodeType();
    switch (nodeType) {
        case NodeTypeValuePath:
        case NodeTypeFunctionCall:
        case NodeTypeVarDecl:
        case NodeTypeAssignment:
        case NodeTypeUnaryExpression:
        case NodeTypeFlowControlCall:
        case NodeTypeFlowControl:
        case NodeTypeForeignLang:
        {
            break;
        }
        case NodeTypeFunctionDecl:
        {
            auto fd = static_cast<LILFunctionDecl *>(node);
            switch (fd->getFunctionDeclType()) {
                case FunctionDeclTypeInsert:
                case FunctionDeclTypeOverride:
                {
                    break;
                }
                    
                default:
                    this->illegalNodeType(node, value);
                    break;
            }
            
            break;
        }
            
        case NodeTypeInstruction:
        {
            auto instr = static_cast<LILInstruction *>(node);
            switch (instr->getInstructionType()) {
                case InstructionTypePaste:
                {
                    break;
                }
                default:
                    this->illegalNodeType(node, value);
                    return;
            }
            break;
        }
        case NodeTypeIfInstruction:
        {
            auto ifInstr = static_cast<LILIfInstruction *>(node);
            for (auto thenNode : ifInstr->getThen()) {
                this->_validateFunctionDeclChild(value, thenNode.get());
            }
            for (auto elseNode : ifInstr->getElse()) {
                this->_validateFunctionDeclChild(value, elseNode.get());
            }
            break;
        }
   
        default:
        {
            this->illegalNodeType(node, value);
            return;
        }
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILFunctionCall> & value)
{
    switch (value->getFunctionCallType()) {
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
        case FunctionCallTypeSizeOf:
        {
            if (value->getArguments().size() != 1) {
                LILErrorMessage ei;
                ei.message =  "Call to sizeOf() needs one argument";
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

void LILASTValidator::_validate(const std::shared_ptr<LILFlowControl> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILFlowControlCall> & value)
{
    auto arg = value->getArgument();
    if (arg) {
        switch (arg->getNodeType()) {
            case NodeTypeNull:
            case NodeTypeNumberLiteral:
            case NodeTypeStringLiteral:
            case NodeTypeStringFunction:
            case NodeTypeBoolLiteral:
            case NodeTypeExpression:
            case NodeTypeVarName:
            case NodeTypeValuePath:
            case NodeTypeSelector:
            case NodeTypeObjectDefinition:
            case NodeTypeFunctionCall:
                break;
            default:
                this->illegalNodeType(arg.get(), value.get());
                break;
        }
        if (this->getDebug() && !this->hasErrors()) {
            std::cerr << "The argument is a " + LILNode::nodeTypeToString(arg->getNodeType()).data() +". OK\n";
        }
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILInstruction> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILIfInstruction> & value)
{
    for (auto thenNode : value->getThen()) {
        this->validate(thenNode);
    }
    for (auto elseNode : value->getElse()) {
        this->validate(elseNode);
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILForeignLang> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILDocumentation> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILValueList> & value)
{
    if (this->getDebug()) {
        std::cerr << "Nothing to do. OK\n";
    }
}

void LILASTValidator::_validate(const std::shared_ptr<LILIndexAccessor> & value)
{
    auto arg = value->getArgument();
    if (arg) {
        switch (arg->getNodeType()) {
            case NodeTypeNumberLiteral:
            case NodeTypeStringLiteral:
            case NodeTypeStringFunction:
            case NodeTypeExpression:
            case NodeTypeVarName:
            case NodeTypeValuePath:
            case NodeTypeFunctionCall:
                break;
            default:
                this->illegalNodeType(arg.get(), value.get());
                break;
        }
        if (this->getDebug() && !this->hasErrors()) {
            std::cerr << "The argument is a " + LILNode::nodeTypeToString(arg->getNodeType()).data() +". OK\n";
        }
    }
}

void LILASTValidator::validateChildren(const std::vector<std::shared_ptr<LILNode> > &children)
{
    for (auto it = children.begin(); it!=children.end(); ++it)
    {
        this->validate(*it);
    };
}

std::shared_ptr<LILClassDecl> LILASTValidator::getClassContext() const
{
    if (this->_classContext.size() > 0) {
        return this->_classContext.back();
    }
    return nullptr;
}

void LILASTValidator::enterClassContext(std::shared_ptr<LILClassDecl> value)
{
    this->_classContext.push_back(value);
}

void LILASTValidator::exitClassContext()
{
    this->_classContext.pop_back();
}
