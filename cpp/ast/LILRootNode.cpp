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
 *      This is a node that participates in local variables
 *
 ********************************************************************/

#include "LILRootNode.h"
#include "LILAliasDecl.h"
#include "LILAssignment.h"
#include "LILClassDecl.h"
#include "LILConversionDecl.h"
#include "LILDocumentation.h"
#include "LILEnum.h"
#include "LILFunctionDecl.h"
#include "LILInstruction.h"
#include "LILNodeToString.h"
#include "LILPropertyName.h"
#include "LILRule.h"
#include "LILSelector.h"
#include "LILSelectorChain.h"
#include "LILSimpleSelector.h"
#include "LILSnippetInstruction.h"
#include "LILStringLiteral.h"
#include "LILTypeDecl.h"
#include "LILValuePath.h"
#include "LILVarDecl.h"
#include "LILVarName.h"

using namespace LIL;

LILRootNode::LILRootNode()
: LILVarNode(NodeTypeRoot)
{

}

LILRootNode::LILRootNode(const LILRootNode & other)
: LILVarNode(other)
, _localVars(other._localVars)
, _classes(other._classes)
, _aliases(other._aliases)
, _types(other._types)
, _conversions(other._conversions)
, _constants(other._constants)
, _snippets(other._snippets)
, _initializers(other._initializers)
, _docs(other._docs)
, _rules(other._rules)
, _config(other._config)
{

}

LILRootNode::~LILRootNode()
{
    
}

bool LILRootNode::isRootNode() const
{
    return true;
}

const std::vector<std::shared_ptr<LILNode>> & LILRootNode::getNodes() const
{
    return this->getChildNodes();
}

void LILRootNode::add(std::shared_ptr<LILNode> node, bool addToNodeTree)
{
    switch (node->getNodeType()) {
        case NodeTypeVarDecl:
        {
            auto vd = std::static_pointer_cast<LILVarDecl>(node);
            if (addToNodeTree) {
                this->addNode(node);
            }
            //local variables on root are globals
            this->setLocalVariable(vd->getName(), node);
            break;
        }
        case NodeTypeFunctionDecl:
        {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
            
            auto fnTy = std::static_pointer_cast<LILFunctionType>(fd->getType());
            if (!fnTy) {
                std::cerr << "FN DECL HAD NO TY FAIL!!!!!!!!!!\n";
                break;
            }
            for ( auto arg : fnTy->getArguments()) {
                if (arg->isA(NodeTypeVarDecl)) {
                    auto argVd = std::static_pointer_cast<LILVarDecl>(arg);
                    fd->setLocalVariable(argVd->getName(), argVd);
                }
            }
            if (addToNodeTree) {
                this->addNode(fd);
            }
            this->setLocalVariable(fd->getName(), fd);
            break;
        }
        case NodeTypeClassDecl:
        {
            if (addToNodeTree) {
                this->addNode(node);
            }
            this->addClass(std::static_pointer_cast<LILClassDecl>(node));
            break;
        }
        case NodeTypeAliasDecl:
        {
            if (addToNodeTree) {
                this->addNode(node);
            }
            this->addAlias(std::static_pointer_cast<LILAliasDecl>(node));
            break;
        }
        case NodeTypeTypeDecl:
        {
            if (addToNodeTree) {
                this->addNode(node);
            }
            this->addType(std::static_pointer_cast<LILTypeDecl>(node));
            break;
        }
        case NodeTypeConversionDecl:
        {
            if (addToNodeTree) {
                this->addNode(node);
            }
            this->addConversion(std::static_pointer_cast<LILConversionDecl>(node));
            break;
        }
        case NodeTypeInstruction:
        {
            auto instr = std::static_pointer_cast<LILInstruction>(node);
            switch (instr->getInstructionType()) {
                case InstructionTypeNone:
                case InstructionTypeNew:
                case InstructionTypeMove:
                case InstructionTypeRGB:
                case InstructionTypeRGBA:
                case InstructionTypeRGBAA:
                case InstructionTypeRRGGBB:
                case InstructionTypeRRGGBBA:
                case InstructionTypeRRGGBBAA:
                case InstructionTypeGrayscale1:
                case InstructionTypeGrayscale2:
                case InstructionTypeDelete:
                case InstructionTypeSnippet:
                case InstructionTypeBug:
                case InstructionTypeArg:
                case InstructionTypeExpand:
                case InstructionTypeResource:
                    //do nothing
                    break;

                case InstructionTypeIf:
                case InstructionTypeNeeds:
                case InstructionTypeImport:
                case InstructionTypePaste:
                case InstructionTypeGetConfig:
                {
                    if (addToNodeTree) {
                        this->addNode(instr);
                    }
                    break;
                }
                case InstructionTypeConfigure:
                {
                    if (addToNodeTree) {
                        this->addNode(instr);
                    }
                    this->addConfigureInstr(instr);
                    break;
                }
                case InstructionTypeExport:
                {
                    for (auto instrNode : instr->getChildNodes()) {
                        this->add(instrNode);
                    }
                    break;
                }
                case InstructionTypeGPU:
                {
                    for (auto instrNode : instr->getChildNodes()) {
                        this->addGPUNode(instrNode);
                    }
                    break;
                }
            }
            break;
        }
        case NodeTypeSnippetInstruction:
        {
            auto snInstr = std::static_pointer_cast<LILSnippetInstruction>(node);
            this->addSnippet(snInstr);
            if (addToNodeTree) {
                this->addNode(node);
            }
            break;
        }
        case NodeTypeIfInstruction:
        {
            if (addToNodeTree) {
                this->addNode(node);
            }
            break;
        }
        case NodeTypeForeignLang:
        {
            if (addToNodeTree) {
                this->addNode(node);
            }
            break;
        }
        case NodeTypeDocumentation:
        {
            this->addDoc(std::static_pointer_cast<LILDocumentation>(node));
            break;
        }
        case NodeTypeRule:
        {
            this->addRule(std::static_pointer_cast<LILRule>(node));
            break;
        }
        case NodeTypeEnum:
        {
            auto enm = std::static_pointer_cast<LILEnum>(node);
            if (addToNodeTree) {
                this->addNode(enm);
            }
            this->setLocalVariable(enm->getName(), enm);
            this->addEnum(enm);
            break;
        }
        default:
            this->addEvaluable(node);
            break;
    }
}

void LILRootNode::clearNodes()
{
    this->clearChildNodes();
    this->_classes.clear();
    this->_rules.clear();
    this->_docs.clear();
    this->_types.clear();
    this->_aliases.clear();
    this->_enums.clear();
    this->_snippets.clear();
    this->_config.clear();
    this->_constants.clear();
    this->_mainMenu.clear();
}

void LILRootNode::appendNodes(const std::vector<std::shared_ptr<LILNode>> & nodes)
{
    this->_childNodes.insert(this->_childNodes.begin() + this->_childNodes.size(), nodes.begin(), nodes.end());
}

void LILRootNode::addClass(std::shared_ptr<LILClassDecl> value)
{
    this->_classes.push_back(value);
}

const std::vector<std::shared_ptr<LILClassDecl>> & LILRootNode::getClasses() const
{
    return this->_classes;
}

void LILRootNode::removeClass(std::shared_ptr<LILClassDecl> value)
{
    auto it = std::find(this->_classes.begin(), this->_classes.end(), value);
    if (it != this->_classes.end()) {
        this->_classes.erase(it);
    }
}

void LILRootNode::addAlias(std::shared_ptr<LILAliasDecl> value)
{
    this->_aliases.push_back(value);
}

const std::vector<std::shared_ptr<LILAliasDecl>> & LILRootNode::getAliases() const
{
    return this->_aliases;
}

void LILRootNode::addType(std::shared_ptr<LILTypeDecl> value)
{
    this->_types.push_back(value);
}

const std::vector<std::shared_ptr<LILTypeDecl>> & LILRootNode::getTypes() const
{
    return this->_types;
}

void LILRootNode::addEnum(std::shared_ptr<LILEnum> value)
{
    this->_enums.push_back(value);
}

const std::vector<std::shared_ptr<LILEnum>> & LILRootNode::getEnums() const
{
    return this->_enums;
}

void LILRootNode::addConversion(std::shared_ptr<LILConversionDecl> value)
{
    auto vd = value->getVarDecl();
    if (!vd) {
        std::cerr << "!!!!!! CONVERSION DECL HAD NO VAR DECL FAIL !!!!!!\n\n";
        return;
    }
    auto fromTy = vd->getType();
    if (!fromTy) {
        std::cerr << "!!!!!! VAR DECL OF CONVERSION DECL HAD NO TYPE FAIL !!!!!!\n\n";
        return;
    }
    auto fromTyName = LILNodeToString::stringify(fromTy.get());

    auto toTy = value->getType();
    if (!toTy) {
        std::cerr << "!!!!!! CONVERSION DECL HAD NO TARGET TYPE FAIL !!!!!!\n\n";
        return;
    }
    auto toTyName = LILNodeToString::stringify(toTy.get());
    
    this->_conversions[fromTyName+"_to_"+toTyName] = value;
}

const std::map<LILString, std::shared_ptr<LILConversionDecl>> & LILRootNode::getConversions() const
{
    return this->_conversions;
}

std::shared_ptr<LILConversionDecl> LILRootNode::getConversionNamed(LILString name)
{
    if (this->_conversions.count(name)) {
        return this->_conversions[name];
    }
    return nullptr;

}

const std::map<LILString, std::shared_ptr<LILSnippetInstruction>> & LILRootNode::getSnippets() const
{
    return this->_snippets;
}

std::shared_ptr<LILSnippetInstruction> LILRootNode::getSnippetNamed(LILString key)
{
    if (this->_snippets.count(key)) {
        return this->_snippets.at(key);
    }
    return nullptr;
}

void LILRootNode::addSnippet(std::shared_ptr<LILSnippetInstruction> snippet)
{
    this->_snippets[snippet->getName()] = snippet;
}

void LILRootNode::addEvaluable(std::shared_ptr<LILNode> node)
{
    this->_initializers.push_back(node);
}

bool LILRootNode::hasInitializers() const
{
    return this->_initializers.size() > 0;
}

const std::vector<std::shared_ptr<LILNode>> & LILRootNode::getInitializers() const
{
    return this->_initializers;
}

void LILRootNode::clearInitializers()
{
    this->_initializers.clear();
}

void LILRootNode::addDoc(std::shared_ptr<LILDocumentation> value)
{
    this->addNode(value);
    this->_docs.push_back(value);
}

const std::vector<std::shared_ptr<LILDocumentation>> & LILRootNode::getDocs() const
{
    return this->_docs;
}

bool LILRootNode::hasRules() const
{
    return this->_rules.size() > 0;
}

void LILRootNode::addRule(std::shared_ptr<LILRule> value)
{
    const auto & selChNode = value->getSelectorChain();
    if (selChNode && selChNode->getNodeType() == NodeTypeSelectorChain) {
        const auto & selCh = std::static_pointer_cast<LILSelectorChain>(selChNode);
        const auto & selChNodes = selCh->getNodes();
        if (selChNodes.size() == 1) {
            const auto & simpleSel = std::static_pointer_cast<LILSimpleSelector>(selChNodes.front());
            const auto & simpleSelNodes = simpleSel->getNodes();
            if (simpleSelNodes.size() == 1) {
                const auto & sel = std::static_pointer_cast<LILSelector>(simpleSelNodes.front());
                if (sel->isA(SelectorTypeMainMenu)) {
                    this->addNode(value);
                    for (auto childRule : value->getChildRules()) {
                        this->_mainMenu.push_back(childRule);
                    }
                    return;
                }
            }
        }
    }
    this->addNode(value);
    this->_rules.push_back(value);
}

const std::vector<std::shared_ptr<LILRule>> & LILRootNode::getRules() const
{
    return this->_rules;
}

bool LILRootNode::hasMainMenu() const
{
    return this->_mainMenu.size() > 0;
}

const std::vector<std::shared_ptr<LILNode>> & LILRootNode::getMainMenuItems() const
{
    return this->_mainMenu;
}

void LILRootNode::clearMainMenuItems()
{
    this->_mainMenu.clear();
}

void LILRootNode::addConfigureInstr(const std::shared_ptr<LILInstruction> & instr)
{
    this->_config.push_back(instr);
}

const std::vector<std::shared_ptr<LILNode>> & LILRootNode::getConfigure() const
{
    return this->_config;
}

void LILRootNode::addGPUNode(const std::shared_ptr<LILNode> & node)
{
    this->_gpuNodes.push_back(node);
}

const std::vector<std::shared_ptr<LILNode>> & LILRootNode::getGPUNodes() const
{
    return this->_gpuNodes;
}

std::shared_ptr<LILClassDecl> LILRootNode::findClassWithName(const LILString & name) const
{
    for (auto classVal : this->getClasses()) {
        if (classVal->getName() == name) {
            return classVal;
        }
    }
    return nullptr;
}

std::shared_ptr<LILEnum> LILRootNode::findEnumWithName(const LILString & name) const
{
    for (auto enumVal : this->getEnums()) {
        if (enumVal->getName() == name) {
            return enumVal;
        }
    }
    return nullptr;
}
