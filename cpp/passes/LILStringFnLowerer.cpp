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
 *      This file converts color instructions into object definitions
 *
 ********************************************************************/


#include "LILStringFnLowerer.h"
#include "LILASTBuilder.h"
#include "LILAssignment.h"
#include "LILConversionDecl.h"
#include "LILEnum.h"
#include "LILExpression.h"
#include "LILFlowControl.h"
#include "LILFlowControlCall.h"
#include "LILFunctionCall.h"
#include "LILFunctionDecl.h"
#include "LILInstruction.h"
#include "LILNodeToString.h"
#include "LILNumberLiteral.h"
#include "LILObjectDefinition.h"
#include "LILObjectType.h"
#include "LILPropertyName.h"
#include "LILRootNode.h"
#include "LILStringLiteral.h"
#include "LILStringFunction.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILValuePath.h"
#include "LILVarName.h"
#include "LILVarDecl.h"

using namespace LIL;

LILStringFnLowerer::LILStringFnLowerer()
: _count(0)
{
}

LILStringFnLowerer::~LILStringFnLowerer()
{
}

void LILStringFnLowerer::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "===  STRING FN LOWERING  ===\n";
        std::cerr << "============================\n\n";
    }
}

void LILStringFnLowerer::visit(LILNode *node)
{
    
}

void LILStringFnLowerer::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    for (auto node : nodes) {
        this->processStringFn(node);
    }
    for (auto node : rootNode->getInitializers()) {
        this->processStringFn(node);
    }
}

bool LILStringFnLowerer::processStringFn(std::shared_ptr<LILNode> node)
{
    if (this->getDebug()) {
        std::cerr << "## string lowering " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
    }
    switch (node->getNodeType()) {
        case NodeTypeRoot:
        case NodeTypeNull:
        case NodeTypeBoolLiteral:
        case NodeTypeNumberLiteral:
        case NodeTypePercentage:
        case NodeTypeStringLiteral:
        case NodeTypeCStringLiteral:
        case NodeTypePropertyName:
        case NodeTypeVarName:
        case NodeTypeAliasDecl:
        case NodeTypeTypeDecl:
        case NodeTypeType:
        case NodeTypeMultipleType:
        case NodeTypeFunctionType:
        case NodeTypeObjectType:
        case NodeTypePointerType:
        case NodeTypeStaticArrayType:
        case NodeTypeFlag:
        case NodeTypeFilter:
        case NodeTypeSelector:
        case NodeTypeCombinator:
        case NodeTypeForeignLang:
        case NodeTypeComment:
        case NodeTypeInvalid:
        case NodeTypeDocumentation:
        case NodeTypeNegation:
        case NodeTypeSIMDType:
            //do nothing
            break;
    
        case NodeTypeStringFunction:
        {
            auto value = std::static_pointer_cast<LILStringFunction>(node);
            this->_processStringFn(value);
            break;
        }
        case NodeTypeExpression:
        {
            auto value = std::static_pointer_cast<LILExpression>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeUnaryExpression:
        {
            auto value = std::static_pointer_cast<LILUnaryExpression>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeVarDecl:
        case NodeTypeConstDecl:
        {
            auto value = std::static_pointer_cast<LILVarDecl>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeConversionDecl:
        {
            auto value = std::static_pointer_cast<LILConversionDecl>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeEnum:
        {
            auto value = std::static_pointer_cast<LILEnum>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeObjectDefinition:
        {
            auto value = std::static_pointer_cast<LILObjectDefinition>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeAssignment:
        {
            auto value = std::static_pointer_cast<LILAssignment>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeFunctionDecl:
        {
            auto value = std::static_pointer_cast<LILFunctionDecl>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeFunctionCall:
        {
            auto value = std::static_pointer_cast<LILFunctionCall>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeFlowControl:
        {
            auto value = std::static_pointer_cast<LILFlowControl>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeFlowControlCall:
        {
            auto value = std::static_pointer_cast<LILFlowControlCall>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeInstruction:
        {
            auto value = std::static_pointer_cast<LILInstruction>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeValueList:
        {
            auto value = std::static_pointer_cast<LILValueList>(node);
            return this->_processStringFn(value);
        }
        case NodeTypeRule:
        case NodeTypeClassDecl:
        case NodeTypeSelectorChain:
        case NodeTypeSimpleSelector:
        case NodeTypeValuePath:
        case NodeTypeIndexAccessor:
        case NodeTypeIfInstruction:
        case NodeTypeSnippetInstruction:
        {
            for (const auto & child : node->getChildNodes()) {
                this->processStringFn(child);
            }
            break;
        }
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILStringFunction> value)
{
    auto fd = std::make_shared<LILFunctionDecl>();
    fd->setSourceLocation(value->getSourceLocation());
    auto fnName = "lil_string_fn_"+LILString::number((LILUnitI64)this->_count);
    fd->setName(fnName);
    auto fnTy = std::make_shared<LILFunctionType>();
    fnTy->setReturnType(LILObjectType::make("string"));

    size_t i = 0;
    for (auto node : value->getNodes()) {
        switch (node->getNodeType()) {
            case NodeTypeVarName:
            {
                auto vd = std::make_shared<LILVarDecl>();
                vd->setSourceLocation(value->getSourceLocation());
                vd->setName("arg"+LILString::number((LILUnitI64)i));
                auto nodeTy = node->getType();
                if (!nodeTy) {
                    std::cerr << "!! NODE HAD NO TYPE FAIL!!!!!!!\n\n";
                    continue;
                }
                vd->setType(nodeTy->clone());
                fnTy->addArgument(vd);
                break;
            }
                
            default:
                std::cerr << "UNIMPLEMENTED FAIL!!!!!!!\n\n";
                break;
        }

        i += 1;
    }
    
    fd->setType(fnTy);
    this->getRootNode()->add(fd);
    
    auto retVd = std::make_shared<LILVarDecl>();
    retVd->setSourceLocation(value->getSourceLocation());
    retVd->setName("ret");
    auto strTy = LILObjectType::make("string");
    retVd->setType(strTy);
    auto strLit = std::make_shared<LILStringLiteral>();
    strLit->setSourceLocation(value->getSourceLocation());
    strLit->setValue(value->getStartChunk());
    retVd->setInitVal(strLit);

    fd->addEvaluable(retVd);

    const auto & midChunks = value->getMidChunks();
    const auto & args = fnTy->getArguments();

    i = 0;
    for (auto node : value->getNodes()) {
        auto vp = std::make_shared<LILValuePath>();
        vp->setSourceLocation(value->getSourceLocation());
        auto retVn = std::make_shared<LILVarName>();
        retVn->setName("ret");
        vp->addChild(retVn);
        auto fc = std::make_shared<LILFunctionCall>();
        fc->setFunctionCallType(FunctionCallTypeValuePath);
        fc->setName("append");
        vp->addChild(fc);
        auto argVn = std::make_shared<LILVarName>();
        argVn->setType(args.at(i)->getType()->clone());
        argVn->setName("arg"+LILString::number((LILUnitI64)i));
        fc->addArgument(argVn);
        fd->addEvaluable(vp);

        if (i < midChunks.size()) {
            auto chunkLit = std::make_shared<LILStringLiteral>();
            chunkLit->setValue(midChunks.at(i));
            auto vp2 = std::make_shared<LILValuePath>();
            vp2->setSourceLocation(value->getSourceLocation());
            auto retVn2 = std::make_shared<LILVarName>();
            retVn2->setSourceLocation(value->getSourceLocation());
            retVn2->setName("ret");
            vp2->addChild(retVn2);
            auto fc2 = std::make_shared<LILFunctionCall>();
            fc2->setFunctionCallType(FunctionCallTypeValuePath);
            fc2->setSourceLocation(value->getSourceLocation());
            fc2->setName("append");
            vp2->addChild(fc2);
            fc2->addArgument(chunkLit);
            fd->addEvaluable(vp2);
        }
        i += 1;
    }

    auto endChunkLit = std::make_shared<LILStringLiteral>();
    endChunkLit->setSourceLocation(value->getSourceLocation());
    endChunkLit->setValue(value->getEndChunk());
    auto vp3 = std::make_shared<LILValuePath>();
    vp3->setSourceLocation(value->getSourceLocation());
    auto retVn3 = std::make_shared<LILVarName>();
    retVn3->setSourceLocation(value->getSourceLocation());
    retVn3->setName("ret");
    vp3->addChild(retVn3);
    auto fc3 = std::make_shared<LILFunctionCall>();
    fc3->setFunctionCallType(FunctionCallTypeValuePath);
    fc3->setSourceLocation(value->getSourceLocation());
    fc3->setName("append");
    vp3->addChild(fc3);
    fc3->addArgument(endChunkLit);
    fd->addEvaluable(vp3);

    auto returnCall = std::make_shared<LILFlowControlCall>();
    returnCall->setSourceLocation(value->getSourceLocation());
    returnCall->setSourceLocation(value->getSourceLocation());
    returnCall->setFlowControlCallType(FlowControlCallTypeReturn);
    auto retVn = std::make_shared<LILVarName>();
    retVn->setSourceLocation(value->getSourceLocation());
    retVn->setName("ret");
    returnCall->setArgument(retVn);

    auto strFnCall = std::make_shared<LILFunctionCall>();
    strFnCall->setName(fnName);
    strFnCall->setSourceLocation(value->getSourceLocation());
    strFnCall->setReturnType(strTy->clone());
    for (auto node : value->getNodes()) {
        strFnCall->addArgument(node->clone());
    }
    fd->addEvaluable(returnCall);
    this->_nodeBuffer.back().push_back(strFnCall);

    return true;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILExpression> value)
{
    auto left = value->getLeft();
    if (!left) {
        std::cerr << "EXPRESSION HAD NO LEFT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeLeft = this->processStringFn(left);
    if (removeLeft && this->_nodeBuffer.back().size() == 0) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #arg leaves expression without left side value. Please provide a default value.";
        LILNode::SourceLocation sl = left->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 0) {
        value->setLeft(this->_nodeBuffer.back().back());
    }
    this->_nodeBuffer.pop_back();
    
    auto right = value->getRight();
    if (!right) {
        std::cerr << "EXPRESSION HAD NO RIGHT FAIL !!!!!\n\n";
        return false;
    }
    this->_nodeBuffer.emplace_back();
    bool removeRight = this->processStringFn(right);
    if (removeRight && this->_nodeBuffer.back().size() == 0) {
        LILErrorMessage ei;
        ei.message =  "Evaluation of #arg leaves expression without right side value. Please provide a default value.";
        LILNode::SourceLocation sl = right->getSourceLocation();
        ei.file = sl.file;
        ei.line = sl.line;
        ei.column = sl.column;
        this->errors.push_back(ei);
    } else if (this->_nodeBuffer.back().size() > 0) {
        value->setRight(this->_nodeBuffer.back().back());
    }
    this->_nodeBuffer.pop_back();
    
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILUnaryExpression> value)
{
    auto val = value->getValue();
    if (val && val->isA(InstructionTypeIf)) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(val);
        if (remove && this->_nodeBuffer.back().size() == 0) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #arg leaves unary expression without value. Please provide a default value.";
            LILNode::SourceLocation sl = val->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() > 0) {
            value->setValue(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILVarDecl> value)
{
    auto initVal = value->getInitVal();
    if (initVal) {
        std::vector<std::shared_ptr<LILNode>> buf;
        this->_nodeBuffer.push_back(buf);
        this->processStringFn(initVal);
        if (this->_nodeBuffer.back().size() > 0) {
            value->setInitVals(this->_nodeBuffer.back());
        }
        this->_nodeBuffer.pop_back();
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILObjectDefinition> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        value->setNodes(std::move(resultNodes));
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILAssignment> value)
{
    auto val = value->getValue();
    if (val) {
        this->processStringFn(val);
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILFunctionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        value->setBody(resultNodes);
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILFunctionCall> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesArgs.push_back(node);
        } else {
            hasChangesArgs = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesArgs.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesArgs) {
        value->setArguments(std::move(resultNodesArgs));
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILFlowControl> value)
{
    bool hasChangesArgs = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
    for (auto node : value->getArguments()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesArgs.push_back(node);
        } else {
            hasChangesArgs = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesArgs.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesArgs) {
        value->setArguments(std::move(resultNodesArgs));
    }
    
    bool hasChangesThen = false;
    std::vector<std::shared_ptr<LILNode>> resultNodesThen;
    for (auto node : value->getThen()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesThen.push_back(node);
        } else {
            hasChangesThen = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesThen.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesThen) {
        value->setThen(std::move(resultNodesThen));
    }
    
    bool hasChangesElse = false;
    
    std::vector<std::shared_ptr<LILNode>> resultNodesElse;
    for (auto node : value->getElse()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodesElse.push_back(node);
        } else {
            hasChangesElse = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodesElse.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesElse) {
        value->setElse(std::move(resultNodesElse));
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILFlowControlCall> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->processStringFn(arg);
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILInstruction> value)
{
    auto arg = value->getArgument();
    if (arg) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(arg);
        if (remove && this->_nodeBuffer.back().size() == 0) {
            LILErrorMessage ei;
            ei.message =  "Evaluation of #arg leaves instruction without argument. Please provide a default value.";
            LILNode::SourceLocation sl = arg->getSourceLocation();
            ei.file = sl.file;
            ei.line = sl.line;
            ei.column = sl.column;
            this->errors.push_back(ei);
        } else if (this->_nodeBuffer.back().size() > 0) {
            value->setArgument(this->_nodeBuffer.back().back());
        }
        this->_nodeBuffer.pop_back();
    }
    
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getChildNodes()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        value->setChildNodes(std::move(resultNodes));
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILValueList> value)
{
    bool hasChanges = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getValues()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChanges = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChanges) {
        value->setValues(resultNodes);
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILConversionDecl> value)
{
    bool hasChangesBody = false;
    std::vector<std::shared_ptr<LILNode>> resultNodes;
    for (auto node : value->getBody()) {
        this->_nodeBuffer.emplace_back();
        bool remove = this->processStringFn(node);
        if (!remove && this->_nodeBuffer.back().size() == 0) {
            resultNodes.push_back(node);
        } else {
            hasChangesBody = true;
            for (auto newNode : this->_nodeBuffer.back()) {
                resultNodes.push_back(newNode);
            }
        }
        this->_nodeBuffer.pop_back();
    }
    if (hasChangesBody) {
        value->setBody(std::move(resultNodes));
    }
    return false;
}

bool LILStringFnLowerer::_processStringFn(std::shared_ptr<LILEnum> value)
{
    for (auto node : value->getValues()) {
        this->processStringFn(node);
    }
    return false;
}
