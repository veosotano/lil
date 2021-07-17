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
#include "LILFunctionDecl.h"
#include "LILInstruction.h"
#include "LILNodeToString.h"
#include "LILRule.h"
#include "LILSnippetInstruction.h"
#include "LILTypeDecl.h"
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
            auto ty = vd->getType();
            if (
                vd->getIsExtern()
                || vd->getIsExported()
                || vd->getIsConst()
            ) {
                if (addToNodeTree) {
                    this->addNode(node);
                }
                //local variables on root are globals
                this->setLocalVariable(vd->getName(), vd);
                
            }
            else if (ty && ty->isA(TypeTypeFunction))
            {
                if (addToNodeTree) {
                    this->addNode(node);
                }
                //local variables on root are globals
                this->setLocalVariable(vd->getName(), node);
            }
            else
            {
                this->addEvaluable(node);
            }
            break;
        }
        case NodeTypeFunctionDecl:
        {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(node);
            
            auto fnTy = std::static_pointer_cast<LILFunctionType>(fd->getType());
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
                    //do nothing
                    break;

                case InstructionTypeIf:
                case InstructionTypeNeeds:
                case InstructionTypeImport:
                case InstructionTypePaste:
                {
                    if (addToNodeTree) {
                        this->addNode(instr);
                    }
                    break;
                }
                case InstructionTypeConfigure:
                {
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
        default:
            this->addEvaluable(node);
            break;
    }
}

void LILRootNode::clearNodes()
{
    this->clearChildNodes();
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

void LILRootNode::addDoc(std::shared_ptr<LILDocumentation> value)
{
    this->addNode(value);
    this->_docs.push_back(value);
}

const std::vector<std::shared_ptr<LILDocumentation>> & LILRootNode::getDocs() const
{
    return this->_docs;
}

void LILRootNode::addRule(std::shared_ptr<LILRule> value)
{
    this->addNode(value);
    this->_rules.push_back(value);
}

const std::vector<std::shared_ptr<LILRule>> & LILRootNode::getRules() const
{
    return this->_rules;
}

void LILRootNode::addConfigureInstr(const std::shared_ptr<LILInstruction> & instr)
{
    for (auto node : instr->getChildNodes()) {
        switch (node->getNodeType()) {
            case NodeTypeAssignment:
            {
                auto as = std::static_pointer_cast<LILAssignment>(node);
                const auto & subj = as->getSubject();
                const auto & value = as->getValue();
                LILString name;
                if (subj->getNodeType() == NodeTypeVarName) {
                    auto vn = std::static_pointer_cast<LILVarName>(subj);
                    name = vn->getName();
                }
                if (name.length() > 0) {
                    this->_config[name] = value;
                }
                break;
            }
                
            case NodeTypeRule:
            {
                break;
            }
                
            default:
                std::cerr << "UNEXPECTED NODE TYPE IN CONFIGURE INSTRUCTION FAIL !!!!!!!\n\n";
                break;
        }
    }
}
