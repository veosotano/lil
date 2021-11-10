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
 *      This file is a container for configuration values
 *
 ********************************************************************/

#include "LILConfiguration.h"
#include "LILAssignment.h"
#include "LILBoolLiteral.h"
#include "LILStringLiteral.h"
#include "LILNumberLiteral.h"
#include "LILNodeToString.h"
#include "LILPropertyName.h"
#include "LILUnaryExpression.h"
#include "LILStringFunction.h"
#include "LILValueList.h"
#include "LILVarName.h"

using namespace LIL;

LILConfiguration::LILConfiguration()
{
    
}

LILConfiguration::~LILConfiguration()
{
    
}

bool LILConfiguration::hasConfig(const std::string &name) const
{
    return this->_values.count(name);
}

bool LILConfiguration::getConfigBool(const std::string &name, bool defaultValue) const
{
    if (this->_values.count(name)) {
        auto & vals = this->_values.at(name);
        if (vals.size() > 0) {
            auto lastVal = vals.back();
            switch (lastVal->getNodeType()) {
                case NodeTypeBoolLiteral:
                {
                    auto boolVal = std::static_pointer_cast<LILBoolLiteral>(lastVal);
                    return boolVal->getValue();
                }
                default:
                    break;
            }
        }
    } else {
        return defaultValue;
    }
    return false;
}

const std::string LILConfiguration::getConfigString(const std::string &name) const
{
    if (this->_values.count(name)) {
        auto & vals = this->_values.at(name);
        if (vals.size() > 0) {
            auto lastVal = vals.back();
            return this->extractString(lastVal);
        }
    }
    return "";
}

long int LILConfiguration::getConfigInt(const std::string &name) const
{
    //fixme
    return 0;
}

std::vector<std::shared_ptr<LILNode>> & LILConfiguration::getConfigItems(const std::string & name)
{
    if (this->_values.count(name)) {
        return this->_values[name];
    }
    return this->_empty;
}

void LILConfiguration::applyConfig(const std::shared_ptr<LILNode> & node)
{
    if (node->isA(NodeTypeAssignment)) {
        auto as = std::static_pointer_cast<LILAssignment>(node);
        const auto & subj = as->getSubject();
        if (subj->isA(NodeTypePropertyName)) {
            auto pn = std::static_pointer_cast<LILPropertyName>(subj);
            auto name = pn->getName();
            auto val = as->getValue();
            if (val->isA(NodeTypeValueList)) {
                this->clearConfig(name.data());
                auto vl = std::static_pointer_cast<LILValueList>(val);
                for (const auto & listVal : vl->getValues()) {
                    this->addConfig(name.data(), listVal);
                }
            } else {
                this->setConfig(name.data(), as->getValue());
            }
        }
    }
    else if (node->isA(UnaryExpressionTypeSum))
    {
        auto ue = std::static_pointer_cast<LILUnaryExpression>(node);
        const auto & subj = ue->getSubject();
        LILString name;
        if (subj->isA(NodeTypePropertyName)) {
            auto pn = std::static_pointer_cast<LILPropertyName>(subj);
            name = pn->getName();
        } else if (subj->isA(NodeTypeVarName)){
            auto vn = std::static_pointer_cast<LILVarName>(subj);
            name = vn->getName();
        }
        auto val = ue->getValue();
        if (val->isA(NodeTypeValueList)) {
            auto vl = std::static_pointer_cast<LILValueList>(val);
            for (const auto & listVal : vl->getValues()) {
                this->addConfig(name.data(), listVal);
            }
        } else {
            this->addConfig(name.data(), ue->getValue());
        }
    }
}

void LILConfiguration::clearConfig(const std::string & name)
{
    this->_values.erase(name);
}

void LILConfiguration::setConfig(const std::string & name, std::shared_ptr<LILNode> value)
{
    std::vector<std::shared_ptr<LILNode>> theVector;
    theVector.push_back(value);
    this->_values[name] = theVector;
}

void LILConfiguration::addConfig(const std::string & name, std::shared_ptr<LILNode> value)
{
    if (this->_values.count(name)) {
        auto & theVector = this->_values[name];
        theVector.push_back(value);
    } else {
        std::vector<std::shared_ptr<LILNode>> theVector;
        theVector.push_back(value);
        this->_values[name] = theVector;
    }
}

void LILConfiguration::printConfig() const
{
    for (auto value : this->_values) {
        auto vals = value.second;
        std::cerr << value.first << ": ";
        if (vals.size() > 1) {
            std::cerr << "\n";
            for (auto val : vals) {
                if (val->isA(NodeTypeStringFunction)) {
                    std::cerr << "   " << this->extractString(val) << "\n";
                } else {
                    std::cerr << "   " << LILNodeToString::stringify(val.get()).data() << "\n";
                }
            }
        } else {
            const auto & val = vals.back();
            if (val->isA(NodeTypeStringFunction)) {
                std::cerr << this->extractString(val) << "\n";
            } else {
                std::cerr << LILNodeToString::stringify(val.get()).data() << "\n";
            }
        }
    }
}

std::string LILConfiguration::extractString(std::shared_ptr<LILNode> val) const
{
    switch (val->getNodeType()) {
        case NodeTypeStringLiteral:
        {
            auto stringLiteral = std::static_pointer_cast<LILStringLiteral>(val);
            const auto & strvalQuotes = stringLiteral->getValue();
            return strvalQuotes.stripQuotes().data();
        }
        case NodeTypePropertyName:
        {
            auto pn = std::static_pointer_cast<LILPropertyName>(val);
            auto name = pn->getName().data();
            if (this->_values.count(name)) {
                return this->getConfigString(name);
            }
            break;
        }
        case NodeTypeVarName:
        {
            auto vn = std::static_pointer_cast<LILVarName>(val);
            auto name = vn->getName().data();
            if (this->_values.count(name)) {
                return this->getConfigString(name);
            }
            break;
        }

        case NodeTypeStringFunction:
        {
            auto stringFunc = std::static_pointer_cast<LILStringFunction>(val);
            LILString ret = stringFunc->getStartChunk().data();
            const auto & children = stringFunc->getChildNodes();
            const auto & chunks = stringFunc->getMidChunks();
            for (size_t i=0,j=children.size(); i<j; i+=1) {
                const auto & node = children[i];
                auto str = this->extractString(node);
                if (str.length() > 0) {
                    ret.append(str);
                }
                if (chunks.size() > i) {
                    ret.append(chunks[i].data());
                }
            }
            ret.append(stringFunc->getEndChunk().data());
            return ret.stripQuotes().data();
        }
            
        default:
            break;
    }
    
    return "";
}
