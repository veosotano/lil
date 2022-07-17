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
 *      This file prints nodes as text in a hierarchical tree
 *
 ********************************************************************/

#include "LILDocumentationWriter.h"
#include "LILDocumentationTmplManager.h"
#include "../ast/LILNode.h"
#include "../ast/LILNodeToString.h"
#include "../ast/LILPointerType.h"
#include "../ast/LILRootNode.h"
#include "../ast/LILStaticArrayType.h"

using namespace LIL;

std::vector<size_t> LILDocumentationWriterNoItems;

LILDocumentationWriter::LILDocumentationWriter()
{
}

LILDocumentationWriter::~LILDocumentationWriter()
{
}



void LILDocumentationWriter::initializeVisit()
{
    if (this->getVerbose()) {
        if (this->getPrintHeadline()) {
            std::cerr << "\n\n";
            std::cerr << "============================\n";
            std::cerr << "=== WRITING DOCUMENTATION ==\n";
            std::cerr << "============================\n\n";
        } else {
            std::cerr << "\n";
        }
    }
}

void LILDocumentationWriter::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    
    auto str = this->_value->getContent().data();
    if (str.substr(0, 5) == "class") {
        std::vector<std::shared_ptr<LILDocumentation>> aliases;
        std::vector<std::shared_ptr<LILDocumentation>> member_vars;
        std::vector<std::shared_ptr<LILDocumentation>> vvars;
        std::vector<std::shared_ptr<LILDocumentation>> member_fns;
        
        auto className = str.substr(7, std::string::npos);
        auto classDecl = rootNode->findClassWithName(className);
        
        std::unordered_map<std::string, std::string> tmplData;
        tmplData["LIL_DOC_CLASS_NAME"] = "@"+className;
        std::string description;
        for (const auto & childNode : this->_value->getChildNodes()) {
            if (childNode->getNodeType() == NodeTypeDocumentation) {
                auto childDoc = std::static_pointer_cast<LILDocumentation>(childNode);
                auto childStr = childDoc->getContent().data();
                if (childStr.substr(0, 3) == "var") {
                    member_vars.push_back(childDoc);
                } else if (childStr.substr(0, 4) == "vvar") {
                    vvars.push_back(childDoc);
                }  else if (childStr.substr(0, 2) == "fn") {
                    member_fns.push_back(childDoc);
                } else if (childStr.substr(0, 5) == "alias") {
                    aliases.push_back(childDoc);
                } else if (childStr.substr(0, 4) == "href") {
                    auto linkStr = childStr.substr(5, std::string::npos);
                    tmplData["LIL_DOC_CODE_HREF"] = linkStr;
                    tmplData["LIL_DOC_CODE_LABEL"] = linkStr;
                } else {
                    description += "<p>"+childStr+"</p>";
                }
            } else if (childNode->getNodeType() == NodeTypeForeignLang) {
                auto fLa = std::static_pointer_cast<LILForeignLang>(childNode);
                tmplData["LIL_DOC_CLASS_EXAMPLE_CODE"] = fLa->getContent().data();
            }
        }
        tmplData["LIL_DOC_CLASS_DESCRIPTION"] = description;
        if (aliases.size() > 0) {
            tmplData["LIL_DOC_ALIASES"] = this->writeAliasesTemplate(aliases, classDecl.get());
        } else {
            tmplData["LIL_DOC_ALIASES"] = "";
        }
        if (vvars.size() > 0) {
            tmplData["LIL_DOC_VVARS"] = this->writeVvarsTemplate(vvars, classDecl.get());
        } else {
            tmplData["LIL_DOC_VVARS"] = "";
        }
        if (member_vars.size() > 0) {
            tmplData["LIL_DOC_MEMBER_VARS"] = this->writeMemberVarsTemplate(member_vars, classDecl.get());
        } else {
            tmplData["LIL_DOC_MEMBER_VARS"] = "";
        }
        if (member_fns.size() > 0) {
            tmplData["LIL_DOC_MEMBER_FNS"] = this->writeMemberFnsTemplate(member_fns, classDecl.get());
        } else {
            tmplData["LIL_DOC_MEMBER_FNS"] = "";
        }
        
        this->_tmplManager->loadTemplate("lil_doc_class_tmpl.html");
        this->_result = this->_tmplManager->renderTemplate("lil_doc_class_tmpl.html", tmplData);
    }
}

std::string LILDocumentationWriter::writeAliasesTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl)
{
    this->_tmplManager->loadTemplate("lil_doc_alias_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_code_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_class_aliases_tmpl.html");

    std::string aliasStr;
    for (const auto & doc : docs) {
        auto str = doc->getContent().data();
        auto aliasName = str.substr(6, std::string::npos);
        auto alias = classDecl->getAliasNamed(aliasName);
        if (!alias) {
            std::cerr << "!! ALIAS NOT FOUND FAIL!!!!!!!!!!!!\n";
            continue;
        }

        std::unordered_map<std::string, std::string> tmplData;
        tmplData["LIL_DOC_ALIAS_NAME"] = aliasName;
        tmplData["LIL_DOC_ALIAS_TYPE"] =  LILNodeToString::stringify(alias.get()).data();
        
        std::string description;
        for (const auto & childNode : doc->getChildNodes()) {
            if (childNode->getNodeType() == NodeTypeDocumentation) {
                auto childDoc = std::static_pointer_cast<LILDocumentation>(childNode);
                description += "<p>"+childDoc->getContent().data()+"</p>";
            } else if (childNode->getNodeType() == NodeTypeForeignLang) {
                auto fLa = std::static_pointer_cast<LILForeignLang>(childNode);
                tmplData["LIL_DOC_ALIAS_CODE_EXAMPLE"] = this->_htmlEncode(fLa->getContent().data());
            }
        }
        tmplData["LIL_DOC_ALIAS_DESC"] = description;
        if (tmplData.count("LIL_DOC_ALIAS_CODE_EXAMPLE")) {
            std::unordered_map<std::string, std::string> codeTmplData;
            codeTmplData["LIL_DOC_CODE_EXAMPLE"] = tmplData["LIL_DOC_ALIAS_CODE_EXAMPLE"];
            tmplData["LIL_DOC_ALIAS_CODE_EXAMPLE"] = this->_tmplManager->renderTemplate("lil_doc_code_tmpl.html", codeTmplData);
        } else {
            tmplData["LIL_DOC_ALIAS_CODE_EXAMPLE"] = "";
        }

        aliasStr += this->_tmplManager->renderTemplate("lil_doc_alias_tmpl.html", tmplData);
    }
    
    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_ALIASES"] = aliasStr;
    return this->_tmplManager->renderTemplate("lil_doc_class_aliases_tmpl.html", tmplData);
}

std::string LILDocumentationWriter::writeMemberVarsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl)
{
    this->_tmplManager->loadTemplate("lil_doc_var_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_code_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_class_member_vars_tmpl.html");

    std::string varsStr;
    for (const auto & doc : docs) {
        auto str = doc->getContent().data();
        auto varName = str.substr(4, std::string::npos);
        auto field = classDecl->getFieldNamed(varName);
        if (!field) {
            std::cerr << "!! FIELD NOT FOUND FAIL!!!!!!!!!!!!\n";
            continue;
        }

        std::unordered_map<std::string, std::string> tmplData;
        tmplData["LIL_DOC_VAR_NAME"] = varName;
        tmplData["LIL_DOC_VAR_TYPE"] =  "var."+LILNodeToString::stringify(field->getType().get()).data();
        
        std::string description;
        for (const auto & childNode : doc->getChildNodes()) {
            if (childNode->getNodeType() == NodeTypeDocumentation) {
                auto childDoc = std::static_pointer_cast<LILDocumentation>(childNode);
                description += "<p>"+childDoc->getContent().data()+"</p>";
            } else if (childNode->getNodeType() == NodeTypeForeignLang) {
                auto fLa = std::static_pointer_cast<LILForeignLang>(childNode);
                tmplData["LIL_DOC_VAR_CODE_EXAMPLE"] = this->_htmlEncode(fLa->getContent().data());
            }
        }
        tmplData["LIL_DOC_VAR_DESC"] = description;
        if (tmplData.count("LIL_DOC_VAR_CODE_EXAMPLE")) {
            std::unordered_map<std::string, std::string> codeTmplData;
            codeTmplData["LIL_DOC_CODE_EXAMPLE"] = tmplData["LIL_DOC_VAR_CODE_EXAMPLE"];
            tmplData["LIL_DOC_VAR_CODE_EXAMPLE"] = this->_tmplManager->renderTemplate("lil_doc_code_tmpl.html", codeTmplData);
        } else {
            tmplData["LIL_DOC_VAR_CODE_EXAMPLE"] = "";
        }

        varsStr += this->_tmplManager->renderTemplate("lil_doc_var_tmpl.html", tmplData);
    }
    
    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_VARS"] = varsStr;
    return this->_tmplManager->renderTemplate("lil_doc_class_member_vars_tmpl.html", tmplData);
}

std::string LILDocumentationWriter::writeVvarsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl)
{
    this->_tmplManager->loadTemplate("lil_doc_var_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_code_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_class_vvars_tmpl.html");

    std::string varsStr;
    for (const auto & doc : docs) {
        auto str = doc->getContent().data();
        auto varName = str.substr(5, std::string::npos);
        auto field = classDecl->getFieldNamed(varName);
        if (!field) {
            std::cerr << "!! FIELD NOT FOUND FAIL!!!!!!!!!!!!\n";
            continue;
        }

        std::unordered_map<std::string, std::string> tmplData;
        tmplData["LIL_DOC_VAR_NAME"] = varName;
        tmplData["LIL_DOC_VAR_TYPE"] =  "vvar."+LILNodeToString::stringify(field->getType().get()).data();
        
        std::string description;
        for (const auto & childNode : doc->getChildNodes()) {
            if (childNode->getNodeType() == NodeTypeDocumentation) {
                auto childDoc = std::static_pointer_cast<LILDocumentation>(childNode);
                description += "<p>"+childDoc->getContent().data()+"</p>";
            } else if (childNode->getNodeType() == NodeTypeForeignLang) {
                auto fLa = std::static_pointer_cast<LILForeignLang>(childNode);
                tmplData["LIL_DOC_VAR_CODE_EXAMPLE"] = this->_htmlEncode(fLa->getContent().data());
            }
        }
        tmplData["LIL_DOC_VAR_DESC"] = description;
        if (tmplData.count("LIL_DOC_VAR_CODE_EXAMPLE")) {
            std::unordered_map<std::string, std::string> codeTmplData;
            codeTmplData["LIL_DOC_CODE_EXAMPLE"] = tmplData["LIL_DOC_VAR_CODE_EXAMPLE"];
            tmplData["LIL_DOC_VAR_CODE_EXAMPLE"] = this->_tmplManager->renderTemplate("lil_doc_code_tmpl.html", codeTmplData);
        } else {
            tmplData["LIL_DOC_VAR_CODE_EXAMPLE"] = "";
        }

        varsStr += this->_tmplManager->renderTemplate("lil_doc_var_tmpl.html", tmplData);
    }
    
    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_VVARS"] = varsStr;
    return this->_tmplManager->renderTemplate("lil_doc_class_vvars_tmpl.html", tmplData);
}

std::string LILDocumentationWriter::writeMemberFnsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILClassDecl * classDecl)
{
    this->_tmplManager->loadTemplate("lil_doc_fn_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_fn_args_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_class_member_fns_tmpl.html");

    std::string fnsStr;
    for (const auto & doc : docs) {
        auto str = doc->getContent().data();
        auto fnName = str.substr(3, std::string::npos);
        auto methodNode = classDecl->getMethodNamed(fnName);
        if (!methodNode) {
            std::cerr << "!! METHOD NOT FOUND FAIL!!!!!!!!!!!!\n";
            continue;
        }
        auto method = std::static_pointer_cast<LILFunctionDecl>(methodNode);

        std::unordered_map<std::string, std::string> tmplData;
        tmplData["LIL_DOC_FN_NAME"] = fnName;
        auto fnTy = method->getFnType();
        const auto & fnTyArgs = fnTy->getArguments();
        std::vector<std::shared_ptr<LILNode>> argsWithoutSelf;
        if (fnTyArgs.size() > 1) {
            argsWithoutSelf.insert(argsWithoutSelf.begin(), fnTyArgs.begin() + 1, fnTyArgs.end());
        }
        auto newFnTy = std::make_shared<LILFunctionType>();
        newFnTy->setParentNode(fnTy->getParentNode());
        newFnTy->setName("fn");
        newFnTy->setArguments(argsWithoutSelf);
        const auto & returnType = fnTy->getReturnType();
        if (returnType) {
            newFnTy->setReturnType(returnType);
        }
        
        newFnTy->setIsNullable(fnTy->getIsNullable());
        tmplData["LIL_DOC_FN_TYPE"] =  LILNodeToString::stringify(newFnTy.get()).data();
        
        std::string description;
        std::vector<std::shared_ptr<LILDocumentation>> args;
        for (const auto & childNode : doc->getChildNodes()) {
            if (childNode->getNodeType() == NodeTypeDocumentation) {
                auto childDoc = std::static_pointer_cast<LILDocumentation>(childNode);
                const auto & childStr = childDoc->getContent().data();
                if (childStr.substr(0, 3) == "var") {
                    args.push_back(childDoc);
                } else {
                    description += "<p>"+childDoc->getContent().data()+"</p>";
                }
            } else if (childNode->getNodeType() == NodeTypeForeignLang) {
                auto fLa = std::static_pointer_cast<LILForeignLang>(childNode);
                tmplData["LIL_DOC_FN_CODE_EXAMPLE"] = this->_htmlEncode(fLa->getContent().data());
            }
        }
        tmplData["LIL_DOC_FN_DESC"] = description;
        if (args.size() > 0) {
            tmplData["LIL_DOC_FN_ARGUMENTS"] = this->writeFnArgsTemplate(args, method.get());
        } else {
            tmplData["LIL_DOC_FN_ARGUMENTS"] = "";
        }
        
        if (tmplData.count("LIL_DOC_FN_CODE_EXAMPLE")) {
            std::unordered_map<std::string, std::string> codeTmplData;
            codeTmplData["LIL_DOC_CODE_EXAMPLE"] = tmplData["LIL_DOC_FN_CODE_EXAMPLE"];
            tmplData["LIL_DOC_FN_CODE_EXAMPLE"] = this->_tmplManager->renderTemplate("lil_doc_code_tmpl.html", codeTmplData);
        } else {
            tmplData["LIL_DOC_FN_CODE_EXAMPLE"] = "";
        }

        fnsStr += this->_tmplManager->renderTemplate("lil_doc_fn_tmpl.html", tmplData);
    }
    
    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_FNS"] = fnsStr;
    return this->_tmplManager->renderTemplate("lil_doc_class_member_fns_tmpl.html", tmplData);
}

std::string LILDocumentationWriter::writeFnArgsTemplate(std::vector<std::shared_ptr<LILDocumentation>> docs, LILFunctionDecl * fnDecl)
{
    this->_tmplManager->loadTemplate("lil_doc_fn_arg_tmpl.html");
    this->_tmplManager->loadTemplate("lil_doc_fn_args_tmpl.html");

    std::string argsStr;
    for (const auto & doc : docs) {
        auto str = doc->getContent().data();
        auto argName = str.substr(4, std::string::npos);
        auto fnTy = fnDecl->getFnType();
        std::shared_ptr<LILVarDecl> argVd;
        for (const auto & arg : fnTy->getArguments()) {
            if (arg->getNodeType() == NodeTypeVarDecl) {
                auto vd = std::static_pointer_cast<LILVarDecl>(arg);
                if (vd->getName() == argName) {
                    argVd = vd;
                    break;
                }
            }
        }
        if (!argVd) {
            std::cerr << "!! ARGUMENT NOT FOUND FAIL!!!!!!!!!!!!\n";
            continue;
        }

        std::unordered_map<std::string, std::string> tmplData;
        tmplData["LIL_DOC_FN_ARG_NAME"] = argName;
        tmplData["LIL_DOC_FN_ARG_TYPE"] = LILNodeToString::stringify(argVd->getType().get()).data();
        
        std::string description;
        for (const auto & childNode : doc->getChildNodes()) {
            if (childNode->getNodeType() == NodeTypeDocumentation) {
                auto childDoc = std::static_pointer_cast<LILDocumentation>(childNode);
                description += "<p>"+childDoc->getContent().data()+"</p>";
            }
        }
        tmplData["LIL_DOC_FN_ARG_DESC"] = description;

        argsStr += this->_tmplManager->renderTemplate("lil_doc_fn_arg_tmpl.html", tmplData);
    }
    
    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_FN_ARGUMENTS"] = argsStr;
    return this->_tmplManager->renderTemplate("lil_doc_fn_args_tmpl.html", tmplData);
}

void LILDocumentationWriter::visit(LILNode *node)
{
    
}

void LILDocumentationWriter::setValue(std::shared_ptr<LILDocumentation> doc)
{
    this->_value = doc;
}

const std::shared_ptr<LILDocumentation> & LILDocumentationWriter::getValue() const
{
    return this->_value;
}

const std::string & LILDocumentationWriter::getResult() const
{
    return this->_result;
}

void LILDocumentationWriter::setTemplateManager(LILDocumentationTmplManager *mgr)
{
    this->_tmplManager = mgr;
}

std::string LILDocumentationWriter::createBoilerplate(LILClassDecl * classDecl, LILRootNode * rootNode)
{
    this->_tmplManager->loadTemplate("lil_boilerplate_class_tmpl.doc.lil");
    std::unordered_map<std::string, std::string> tmplData;
    
    tmplData["LIL_DOC_CLASS_NAME"] = "@"+classDecl->getType()->getName().data();

    std::string aliasesStr = "";
    for (const auto & alias : classDecl->getAliases()) {
        aliasesStr += this->writeAliasBoilerplate(alias.get());
    }
    tmplData["LIL_DOC_ALIASES"] = aliasesStr;
    
    std::vector<std::shared_ptr<LILVarDecl>> vvars;
    std::vector<std::shared_ptr<LILVarDecl>> memberVars;
    for (const auto & field : classDecl->getFields()) {
        if (field->getNodeType() != NodeTypeVarDecl) {
            continue;
        }
        auto vd = std::static_pointer_cast<LILVarDecl>(field);
        if (vd->getIsVVar()) {
            vvars.push_back(vd);
        } else {
            memberVars.push_back(vd);
        }
    }
    std::string vvarsStr = "";
    for (const auto & vvar : vvars) {
        vvarsStr += this->writeVvarBoilerplate(vvar.get());
    }
    tmplData["LIL_DOC_VVARS"] = vvarsStr;
    
    std::string memberVarsStr = "";
    for (const auto & memberVar : memberVars) {
        memberVarsStr += this->writeMemberVarBoilerplate(memberVar.get());
    }
    tmplData["LIL_DOC_MEMBER_VARS"] = memberVarsStr;
    
    std::string methodsStr = "";
    for (const auto & methodPair : classDecl->getMethods()) {
        if (methodPair.second->getNodeType() == NodeTypeFunctionDecl) {
            auto fd = std::static_pointer_cast<LILFunctionDecl>(methodPair.second);
            methodsStr += this->writeMemberFnBoilerplate(fd.get());
        }
        
    }
    tmplData["LIL_DOC_MEMBER_FNS"] = methodsStr;

    return this->_tmplManager->renderTemplate("lil_boilerplate_class_tmpl.doc.lil", tmplData);
}

std::string LILDocumentationWriter::writeAliasBoilerplate(LILAliasDecl * alias) const
{
    this->_tmplManager->loadTemplate("lil_boilerplate_alias_tmpl.doc.lil");

    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_ALIAS_NAME"] = alias->getSrcType()->getName().data();
    return this->_tmplManager->renderTemplate("lil_boilerplate_alias_tmpl.doc.lil", tmplData);
}

std::string LILDocumentationWriter::writeVvarBoilerplate(LILVarDecl * vd) const
{
    this->_tmplManager->loadTemplate("lil_boilerplate_vvar_tmpl.doc.lil");

    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_VVAR_NAME"] = vd->getName().data();
    return this->_tmplManager->renderTemplate("lil_boilerplate_vvar_tmpl.doc.lil", tmplData);
}

std::string LILDocumentationWriter::writeMemberVarBoilerplate(LILVarDecl * vd) const
{
    this->_tmplManager->loadTemplate("lil_boilerplate_member_var_tmpl.doc.lil");

    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_VAR_NAME"] = vd->getName().data();
    return this->_tmplManager->renderTemplate("lil_boilerplate_member_var_tmpl.doc.lil", tmplData);
}

std::string LILDocumentationWriter::writeMemberFnBoilerplate(LILFunctionDecl * fnDecl) const
{
    this->_tmplManager->loadTemplate("lil_boilerplate_member_fn_tmpl.doc.lil");

    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_FN_NAME"] = fnDecl->getUnmangledName().data();
    
    std::string argsStr = "";
    for (const auto & arg : fnDecl->getFnType()->getArguments()) {
        if (arg->getNodeType() == NodeTypeVarDecl) {
            auto vd = std::static_pointer_cast<LILVarDecl>(arg);
            if (vd->getName() != "@self") {
                argsStr += this->writeFnArgBoilerplate(vd.get());
            }
        }
    }
    tmplData["LIL_DOC_FN_ARGUMENTS"] = argsStr;
    
    return this->_tmplManager->renderTemplate("lil_boilerplate_member_fn_tmpl.doc.lil", tmplData);
}

std::string LILDocumentationWriter::writeFnArgBoilerplate(LILVarDecl * vd) const
{
    this->_tmplManager->loadTemplate("lil_boilerplate_fn_arg_tmpl.doc.lil");

    std::unordered_map<std::string, std::string> tmplData;
    tmplData["LIL_DOC_FN_ARG_NAME"] = vd->getName().data();
    return this->_tmplManager->renderTemplate("lil_boilerplate_fn_arg_tmpl.doc.lil", tmplData);
}

std::string LILDocumentationWriter::_htmlEncode(const std::string& data) const
{
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}
