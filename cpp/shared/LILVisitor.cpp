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
 *      This file defines the base class for visitors
 *
 ********************************************************************/

#include "LILVisitor.h"
#include "LILClassDecl.h"
#include "LILErrorMessage.h"
#include "LILVarName.h"
#include "LILVarNode.h"
#include "LILRootNode.h"
#include "LILFunctionType.h"

using namespace LIL;

LILVisitor::LILVisitor()
: _printHeadline(true)
, _verbose(false)
, _debug(false)
{
}

LILVisitor::~LILVisitor()
{
}

void LILVisitor::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    for (const auto & node : nodes) {
        node->accept(this);
    }
}

bool LILVisitor::hasErrors() const
{
    size_t errorSize = this->errors.size();
    return errorSize > 0;
}

void LILVisitor::printErrors(const LILString & code) const
{
    if (this->errors.size() == 0) {
        return;
    }

    std::vector<std::string> lines = this->splitString(code.chardata(), "\n");

    std::cerr << "\nFound ";
    std::cerr << errors.size();
    std::cerr << " errors in your code:\n";
    for (auto it=errors.begin(); it!=errors.end(); ++it) {
        LILErrorMessage ei = *it;
        std::cerr << ei.message.chardata();
        std::cerr << " on line ";
        std::cerr << ei.line;
        std::cerr << " column ";
        std::cerr << ei.column;
        std::cerr << "\n\n";

        if (ei.line > 2)
        {
            std::cerr << ei.line - 1;
            std::cerr << ": ";
            std::cerr << lines[ei.line-2];
            std::cerr << "\n";
        }

        std::cerr << ei.line;
        std::cerr << ": ";
        std::cerr << lines[ei.line-1];
        std::cerr << "\n";
        std::string indicator = "   ";
        if (ei.column > 2) {
            for (unsigned i=0; i<ei.column-3; ++i) {
                indicator += " ";
            }
        }
        indicator+= "__^__\n";
        std::cerr << indicator;
        if (ei.line < lines.size()-1)
        {
            std::cerr << ei.line+1;
            std::cerr << ": ";
            std::cerr << lines[ei.line];
            std::cerr << "\n";
        }

    }
    std::cerr << "\n";
}

std::vector<std::string> LILVisitor::splitString(std::string phrase, std::string delimiter) const
{
    std::vector<std::string> list;
    std::string s = phrase;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
}

void LILVisitor::setPrintHeadline(bool value)
{
    this->_printHeadline = value;
}

bool LILVisitor::getPrintHeadline() const
{
    return this->_printHeadline;
}

void LILVisitor::setVerbose(bool value)
{
    this->_verbose = value;
}

bool LILVisitor::getVerbose() const
{
    return this->_verbose;
}

void LILVisitor::setDebug(bool value)
{
    this->_debug = value;
}

bool LILVisitor::getDebug() const
{
    return this->_debug;
}

std::shared_ptr<LILNode> LILVisitor::findNodeForVarName(LILVarName * name) const
{
    std::shared_ptr<LILNode> parent = name->getParentNode();
    LILString nameStr = name->getName();
    while (parent) {
        if(parent->isVarNode()){
            std::shared_ptr<LILVarNode> vn = std::static_pointer_cast<LILVarNode>(parent);
            std::shared_ptr<LILNode> localVar = vn->getVariable(nameStr);
            if (localVar) {
                return localVar;
            }
        }
        parent = parent->getParentNode();
    }
    return nullptr;
}

LILString LILVisitor::decorate(LILString ns, LILString className, LILString name, std::shared_ptr<LILType> type) const
{
    LILString ret("_lil_");
    if (ns.length() > 0) {
        ret += LILString("n") + ns + "_";
    }
    if (className.length() > 0) {
        ret += LILString("c") + LILString::number((LILUnitF64)className.length()) + "_" + className + "_";
    }
    ret += LILString("f") + LILString::number((LILUnitF64)name.length()) + "_" + name;
    if(type){
        ret += this->typeToString(type);
    }

    return ret;
}

LILString LILVisitor::typeToString(std::shared_ptr<LILType> type) const
{
    LILString ret("");
    auto ft = std::dynamic_pointer_cast<LILFunctionType>(type);
    if (ft) {
        const auto & args = ft->getArguments();
        if (args.size() == 0) {
            return ret;
        } else {
            ret = "_";
        }
        for (size_t i=0, j=args.size(); i<j; ++i) {
            const auto & argFt = std::dynamic_pointer_cast<LILFunctionType>(args[i]);
            auto tyName = this->typeToString(args[i]);
            if (argFt) {
                ret += LILString("f0") + tyName;
            } else {
                ret += "a" + LILString::number((LILUnitI64)tyName.length())+"_"+tyName;
            }
            if (i<j-1) {
                ret += "_";
            }
        }
    } else {
        ret = type->getName();
    }
    return ret;
}

void LILVisitor::setRootNode(std::shared_ptr<LILRootNode> value)
{
    this->_rootNode = value;
}

std::shared_ptr<LILRootNode> LILVisitor::getRootNode() const
{
    return this->_rootNode;
}

std::shared_ptr<LILClassDecl> LILVisitor::findClassWithName(const LILString & name) const
{
    for (auto classVal : this->getRootNode()->getClasses()) {
        if (classVal->getName() == name) {
            return classVal;
        }
    }
    return nullptr;
}
