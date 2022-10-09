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
 *	  This file traverses the ast in search for resource fields
 *
 ********************************************************************/

#include "LILResourceGatherer.h"
#include "../ast/LILAssignment.h"
#include "../ast/LILClassDecl.h"
#include "../ast/LILObjectDefinition.h"
#include "../ast/LILPropertyName.h"
#include "../ast/LILRootNode.h"
#include "../ast/LILRule.h"
#include "../ast/LILStringLiteral.h"
#include "../ast/LILValuePath.h"
#include "../ast/LILVarDecl.h"

using namespace LIL;

LILResourceGatherer::LILResourceGatherer()
{
}

LILResourceGatherer::~LILResourceGatherer()
{
}

void LILResourceGatherer::initializeVisit()
{
	if (this->getVerbose()) {
		std::cerr << "\n\n";
		std::cerr << "============================\n";
		std::cerr << "===  RESOURCE GATHERING  ===\n";
		std::cerr << "============================\n\n";
	}
}

void LILResourceGatherer::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
	this->setRootNode(rootNode);
}

const std::vector<LILString> LILResourceGatherer::gatherResources() const
{
	std::vector<LILString> ret;
	for (const auto & child : this->getRootNode()->getNodes()) {
		if (child->getNodeType() == NodeTypeRule) {
			auto rule = std::static_pointer_cast<LILRule>(child);
			auto ruleRet = this->_gatherResources(rule.get());
			if (ruleRet.size() > 0) {
				ret.insert(ret.end(), ruleRet.begin(), ruleRet.end());
			}
		}
	}
	return ret;
}

const std::vector<LILString> LILResourceGatherer::_gatherResources(LILRule * rule) const
{
	std::vector<LILString> ret;
	auto ty = rule->getType();
	if (!ty) {
		return ret;
	}
	auto cdNode = this->findClassWithName(ty->getName());
	if (cdNode && (cdNode->getNodeType() == NodeTypeClassDecl)) {
		auto cd = this->findClassWithName(ty->getName());
	
		if (ty && (ty->getTypeType() == TypeTypeObject)) {
			for (const auto & value : rule->getValues()) {
				if (value->getNodeType() == NodeTypeAssignment) {
					auto as = std::static_pointer_cast<LILAssignment>(value);
					const auto & asVal = as->getValue();
					if (asVal && (asVal->getNodeType() == NodeTypeStringLiteral)) {
						auto strLit = std::static_pointer_cast<LILStringLiteral>(asVal);
						const auto & subj = as->getSubject();
						if (!subj) {
							std::cerr << "ASSIGNMENT HAD NO SUBJECT FAIL !!!! \n";
							break;
						}
						std::shared_ptr<LILVarDecl> vd;
						if (subj->getNodeType() == NodeTypePropertyName) {
							auto pn = std::static_pointer_cast<LILPropertyName>(subj);
							auto fieldNode = cd->getFieldNamed(pn->getName());
							if (!fieldNode) {
								fieldNode = this->findExpandedField(cd, pn->getName());
							}
							if (fieldNode && fieldNode->getNodeType() == NodeTypeVarDecl) {
								vd = std::static_pointer_cast<LILVarDecl>(fieldNode);
							}
						} else if (subj->getNodeType() == NodeTypeValuePath) {
							auto vp = std::static_pointer_cast<LILValuePath>(subj);
							auto remoteNode = this->findNodeForValuePath(vp.get());
							if (remoteNode && remoteNode->isA(NodeTypeVarDecl)) {
								vd = std::static_pointer_cast<LILVarDecl>(remoteNode);
							}
						}
						if (vd) {
							if (vd->getIsResource()) {
								ret.push_back(strLit->getValue().stripQuotes());
							} else if (vd->getType()->getTypeType() == TypeTypeObject) {
								auto initVal = vd->getInitVal();
								if (initVal->getNodeType() == NodeTypeObjectDefinition) {
									vd = this->recursiveGetResourceVd(static_cast<LILObjectDefinition *>(initVal.get()));
								}
								if (vd && vd->getIsResource()) {
									ret.push_back(strLit->getValue().stripQuotes());
								}
							}
						}
					}
				}
			}
		}
	}
	for (const auto & childRule : rule->getChildRules()) {
		std::vector<LILString> ruleRet = this->_gatherResources(childRule.get());
		if (ruleRet.size() > 0) {
			ret.insert(ret.end(), ruleRet.begin(), ruleRet.end());
		}
	}
	return ret;
}

std::shared_ptr<LILVarDecl> LILResourceGatherer::recursiveGetResourceVd(LILObjectDefinition * objDef) const
{
	std::shared_ptr<LILVarDecl> ret;
	auto ty = objDef->getType();
	if (ty->getTypeType() != TypeTypeObject) {
		return ret;
	}
	for (const auto & value : objDef->getNodes()) {
		if (value->getNodeType() != NodeTypeAssignment) {
			continue;
		}
		auto as = std::static_pointer_cast<LILAssignment>(value);
		const auto & asVal = as->getValue();
		if (asVal && (asVal->getNodeType() == NodeTypeStringLiteral)) {
			auto strLit = std::static_pointer_cast<LILStringLiteral>(asVal);
			const auto & subj = as->getSubject();
			if (!subj) {
				std::cerr << "ASSIGNMENT HAD NO SUBJECT FAIL !!!! \n";
				break;
			}
			if (subj->getNodeType() == NodeTypePropertyName) {
				auto pn = std::static_pointer_cast<LILPropertyName>(subj);
				auto cd = this->findClassWithName(ty->getName());
				auto fieldNode = cd->getFieldNamed(pn->getName());
				if (!fieldNode) {
					fieldNode = this->findExpandedField(cd, pn->getName());
				}
				if (fieldNode && fieldNode->getNodeType() == NodeTypeVarDecl) {
					ret = std::static_pointer_cast<LILVarDecl>(fieldNode);
				}
			} else if (subj->getNodeType() == NodeTypeValuePath) {
				auto vp = std::static_pointer_cast<LILValuePath>(subj);
				auto remoteNode = this->findNodeForValuePath(vp.get());
				if (remoteNode && remoteNode->isA(NodeTypeVarDecl)) {
					ret = std::static_pointer_cast<LILVarDecl>(remoteNode);
				}
			}
			if (ret) {
				if (ret->getIsResource()) {
					return ret;
				} else if (ret->getType()->getTypeType() == TypeTypeObject) {
					auto initVal = ret->getInitVal();
					if (initVal->getNodeType() == NodeTypeObjectDefinition) {
						ret = this->recursiveGetResourceVd(static_cast<LILObjectDefinition *>(initVal.get()));
					}
					if (ret && ret->getIsResource()) {
						return ret;
					}
				}
			}
		}
	}
	
	return ret;
}
