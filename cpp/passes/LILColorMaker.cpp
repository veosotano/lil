/********************************************************************
 *
 *	  LIL Is a Language
 *
 *	  AUTHORS: Miro Keller
 *
 *	  COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *	  LICENSE: see LICENSE file
 *
 *	  This file converts color instructions into object definitions
 *
 ********************************************************************/


#include "LILColorMaker.h"
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
#include "LILRule.h"
#include "LILStringLiteral.h"
#include "LILUnaryExpression.h"
#include "LILValueList.h"
#include "LILVarDecl.h"

using namespace LIL;

LILColorMaker::LILColorMaker()
{
}

LILColorMaker::~LILColorMaker()
{
}

void LILColorMaker::initializeVisit()
{
	if (this->getVerbose()) {
		std::cerr << "\n\n";
		std::cerr << "============================\n";
		std::cerr << "======  MAKING COLORS  =====\n";
		std::cerr << "============================\n\n";
	}
}

void LILColorMaker::visit(LILNode *node)
{
	
}

void LILColorMaker::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
	this->setRootNode(rootNode);
	std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
	for (auto node : nodes) {
		this->processColorInstr(node);
	}
	for (auto node : rootNode->getInitializers()) {
		this->processColorInstr(node);
	}
}

bool LILColorMaker::processColorInstr(std::shared_ptr<LILNode> node)
{
	if (this->getDebug()) {
		std::cerr << "##  processing color instructions in " + LILNode::nodeTypeToString(node->getNodeType()).data() + " " + LILNodeToString::stringify(node.get()).data() + " ##\n";
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
		case NodeTypeSIMDType:
		case NodeTypeFlag:
		case NodeTypeFilter:
		case NodeTypeSelector:
		case NodeTypeCombinator:
		case NodeTypeForeignLang:
		case NodeTypeComment:
		case NodeTypeInvalid:
		case NodeTypeDocumentation:
		case NodeTypeNegation:
		case NodeTypeStringFunction:
			//do nothing
			break;
			
		case NodeTypeExpression:
		{
			auto value = std::static_pointer_cast<LILExpression>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeUnaryExpression:
		{
			auto value = std::static_pointer_cast<LILUnaryExpression>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeVarDecl:
		case NodeTypeConstDecl:
		{
			auto value = std::static_pointer_cast<LILVarDecl>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeConversionDecl:
		{
			auto value = std::static_pointer_cast<LILConversionDecl>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeEnum:
		{
			auto value = std::static_pointer_cast<LILEnum>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeObjectDefinition:
		{
			auto value = std::static_pointer_cast<LILObjectDefinition>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeAssignment:
		{
			auto value = std::static_pointer_cast<LILAssignment>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeFunctionDecl:
		{
			auto value = std::static_pointer_cast<LILFunctionDecl>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeFunctionCall:
		{
			auto value = std::static_pointer_cast<LILFunctionCall>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeFlowControl:
		{
			auto value = std::static_pointer_cast<LILFlowControl>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeFlowControlCall:
		{
			auto value = std::static_pointer_cast<LILFlowControlCall>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeInstruction:
		{
			auto value = std::static_pointer_cast<LILInstruction>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeValueList:
		{
			auto value = std::static_pointer_cast<LILValueList>(node);
			return this->_processColorInstr(value);
		}
		case NodeTypeRule:
		{
			auto rule = std::static_pointer_cast<LILRule>(node);
			for (const auto & val : rule->getValues()) {
				this->processColorInstr(val);
			}
			for (const auto & childRule : rule->getChildRules()) {
				this->processColorInstr(childRule);
			}
			break;
		}
		case NodeTypeClassDecl:
		case NodeTypeSelectorChain:
		case NodeTypeSimpleSelector:
		case NodeTypeValuePath:
		case NodeTypeIndexAccessor:
		case NodeTypeIfInstruction:
		case NodeTypeSnippetInstruction:
		{
			for (const auto & child : node->getChildNodes()) {
				this->processColorInstr(child);
			}
			break;
		}
	}
	return false;
}

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILExpression> value)
{
	auto left = value->getLeft();
	if (!left) {
		std::cerr << "EXPRESSION HAD NO LEFT FAIL !!!!!\n\n";
		return false;
	}
	this->_nodeBuffer.emplace_back();
	bool removeLeft = this->processColorInstr(left);
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
	bool removeRight = this->processColorInstr(right);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILUnaryExpression> value)
{
	auto val = value->getValue();
	if (val && val->isA(InstructionTypeIf)) {
		this->_nodeBuffer.emplace_back();
		bool remove = this->processColorInstr(val);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILVarDecl> value)
{
	auto initVal = value->getInitVal();
	if (initVal) {
		std::vector<std::shared_ptr<LILNode>> buf;
		this->_nodeBuffer.push_back(buf);
		this->processColorInstr(initVal);
		if (this->_nodeBuffer.back().size() > 0) {
			value->setInitVals(this->_nodeBuffer.back());
		}
		this->_nodeBuffer.pop_back();
	}
	return false;
}

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILObjectDefinition> value)
{
	bool hasChanges = false;
	std::vector<std::shared_ptr<LILNode>> resultNodes;
	for (auto node : value->getNodes()) {
		this->_nodeBuffer.emplace_back();
		bool remove = this->processColorInstr(node);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILAssignment> value)
{
	auto val = value->getValue();
	if (val) {
		if (val->isA(NodeTypeInstruction))
		{
			auto instr = std::static_pointer_cast<LILInstruction>(val);
			if (instr->getIsColorInstruction()) {
				this->_nodeBuffer.emplace_back();
				bool remove = this->processColorInstr(val);
				if (remove) {
					value->clearValue();
				}
				if (this->_nodeBuffer.back().size() > 0) {
					value->setValue(this->_nodeBuffer.back().back());
				}
				this->_nodeBuffer.pop_back();
			} else {
				this->processColorInstr(val);
			}
		}
		else
		{
			this->processColorInstr(val);
		}
	}
	return false;
}

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILFunctionDecl> value)
{
	bool hasChangesBody = false;
	std::vector<std::shared_ptr<LILNode>> resultNodes;
	for (auto node : value->getBody()) {
		this->_nodeBuffer.emplace_back();
		bool remove = this->processColorInstr(node);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILFunctionCall> value)
{
	bool hasChangesArgs = false;
	std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
	for (auto node : value->getArguments()) {
		this->_nodeBuffer.emplace_back();
		bool remove = this->processColorInstr(node);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILFlowControl> value)
{
	bool hasChangesArgs = false;
	std::vector<std::shared_ptr<LILNode>> resultNodesArgs;
	for (auto node : value->getArguments()) {
		this->_nodeBuffer.emplace_back();
		bool remove = this->processColorInstr(node);
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
		bool remove = this->processColorInstr(node);
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
		bool remove = this->processColorInstr(node);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILFlowControlCall> value)
{
	auto arg = value->getArgument();
	if (arg) {
		this->processColorInstr(arg);
	}
	return false;
}

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILInstruction> value)
{
	if (value->getIsColorInstruction()) {
		auto instrTy = value->getInstructionType();
		double redHexVal = 0.0;
		double blueHexVal = 0.0;
		double greenHexVal = 0.0;
		double alphaHexVal = 1.0;

		//get the values
		switch (instrTy) {
			case InstructionTypeNone:
			case InstructionTypeImport:
			case InstructionTypeNew:
			case InstructionTypePaste:
			case InstructionTypeIf:
			case InstructionTypeArg:
			case InstructionTypeConfigure:
			case InstructionTypeGetConfig:
			case InstructionTypeBug:
			case InstructionTypeMove:
			case InstructionTypeNeeds:
			case InstructionTypeExport:
			case InstructionTypeDelete:
			case InstructionTypeSnippet:
			case InstructionTypeExpand:
			case InstructionTypeGPU:
			case InstructionTypeResource:
			{
				//do nothing
				break;
			}

			case InstructionTypeGrayscale1:
			case InstructionTypeGrayscale2:
			{
				std::string digit = value->getName().data();
				std::string hexStr = "0x"+digit;
				if (instrTy == InstructionTypeGrayscale1) {
					hexStr += digit;
				}
				double hexVal = std::stod(hexStr) / 255.0;
				redHexVal = hexVal;
				greenHexVal = hexVal;
				blueHexVal = hexVal;
				break;
			}

			case InstructionTypeRGB:
			case InstructionTypeRGBA:
			case InstructionTypeRGBAA:
			{
				std::string digits = value->getName().data();
				std::string redDigit = digits.substr(0, 1);
				std::string redHexStr = "0x"+redDigit+redDigit;
				redHexVal = std::stod(redHexStr) / 255.0;
				std::string greenDigit = digits.substr(1, 1);
				std::string greenHexStr = "0x"+greenDigit+greenDigit;
				greenHexVal = std::stod(greenHexStr) / 255.0;
				std::string blueDigit = digits.substr(2, 1);
				std::string blueHexStr = "0x"+blueDigit+blueDigit;
				blueHexVal = std::stod(blueHexStr) / 255.0;
				if (instrTy == InstructionTypeRGBA) {
					std::string alphaDigit = digits.substr(3, 1);
					std::string alphaHexStr = "0x"+alphaDigit+alphaDigit;
					alphaHexVal = std::stod(alphaHexStr) / 255.0;
				} else if (instrTy == InstructionTypeRGBAA) {
					std::string alphaDigit = digits.substr(3, 2);
					std::string alphaHexStr = "0x"+alphaDigit;
					alphaHexVal = std::stod(alphaHexStr) / 255.0;
				}
				break;
			}

			case InstructionTypeRRGGBB:
			case InstructionTypeRRGGBBA:
			case InstructionTypeRRGGBBAA:
			{
				std::string digits = value->getName().data();
				std::string redDigit = digits.substr(0, 2);
				std::string redHexStr = "0x"+redDigit;
				redHexVal = std::stod(redHexStr) / 255.0;
				std::string greenDigit = digits.substr(2, 2);
				std::string greenHexStr = "0x"+greenDigit;
				greenHexVal = std::stod(greenHexStr) / 255.0;
				std::string blueDigit = digits.substr(4, 2);
				std::string blueHexStr = "0x"+blueDigit;
				blueHexVal = std::stod(blueHexStr) / 255.0;
				if (instrTy == InstructionTypeRRGGBBA) {
					std::string alphaDigit = digits.substr(6, 1);
					std::string alphaHexStr = "0x"+alphaDigit+alphaDigit;
					alphaHexVal = std::stod(alphaHexStr) / 255.0;
				} else if (instrTy == InstructionTypeRRGGBBAA) {
					std::string alphaDigit = digits.substr(6, 2);
					std::string alphaHexStr = "0x"+alphaDigit;
					alphaHexVal = std::stod(alphaHexStr) / 255.0;
				}
				break;
			}
		}

		//make the object definition
		auto numTy = LILType::make("f64");
		auto objDef = std::make_shared<LILObjectDefinition>();
		auto objTy = std::make_shared<LILObjectType>();
		objTy->setName("rgb");
		objDef->setType(objTy);
		//red
		auto redNumLit = std::make_shared<LILNumberLiteral>();
		redNumLit->setType(numTy);
		redNumLit->setValue(LILString::number(redHexVal));
		auto red = std::make_shared<LILAssignment>();
		auto redPn = std::make_shared<LILPropertyName>();
		redPn->setName("red");
		red->setType(numTy);
		red->setSubject(redPn);
		red->setValue(redNumLit);
		objDef->addChild(red);
		//green
		auto greenNumLit = std::make_shared<LILNumberLiteral>();
		greenNumLit->setType(numTy);
		greenNumLit->setValue(LILString::number(greenHexVal));
		auto green = std::make_shared<LILAssignment>();
		auto greenPn = std::make_shared<LILPropertyName>();
		greenPn->setName("green");
		green->setType(numTy);
		green->setSubject(greenPn);
		green->setValue(greenNumLit);
		objDef->addNode(green);
		//blue
		auto blueNumLit = std::make_shared<LILNumberLiteral>();
		blueNumLit->setType(numTy);
		blueNumLit->setValue(LILString::number(blueHexVal));
		auto blue = std::make_shared<LILAssignment>();
		auto bluePn = std::make_shared<LILPropertyName>();
		bluePn->setName("blue");
		blue->setType(numTy);
		blue->setSubject(bluePn);
		blue->setValue(blueNumLit);
		objDef->addNode(blue);
		//alpha
		auto alphaNumLit = std::make_shared<LILNumberLiteral>();
		alphaNumLit->setType(numTy);
		alphaNumLit->setValue(LILString::number(alphaHexVal));
		auto alpha = std::make_shared<LILAssignment>();
		auto alphaPn = std::make_shared<LILPropertyName>();
		alphaPn->setName("alpha");
		alpha->setType(numTy);
		alpha->setSubject(alphaPn);
		alpha->setValue(alphaNumLit);
		objDef->addNode(alpha);
		this->_nodeBuffer.back().push_back(objDef);

		return true;
	} else {
		auto arg = value->getArgument();
		if (arg) {
			this->_nodeBuffer.emplace_back();
			bool remove = this->processColorInstr(arg);
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
			bool remove = this->processColorInstr(node);
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
	return false;
}

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILValueList> value)
{
	bool hasChanges = false;
	std::vector<std::shared_ptr<LILNode>> resultNodes;
	for (auto node : value->getValues()) {
		this->_nodeBuffer.emplace_back();
		bool remove = this->processColorInstr(node);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILConversionDecl> value)
{
	bool hasChangesBody = false;
	std::vector<std::shared_ptr<LILNode>> resultNodes;
	for (auto node : value->getBody()) {
		this->_nodeBuffer.emplace_back();
		bool remove = this->processColorInstr(node);
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

bool LILColorMaker::_processColorInstr(std::shared_ptr<LILEnum> value)
{
	for (auto node : value->getValues()) {
		this->processColorInstr(node);
	}
	return false;
}
