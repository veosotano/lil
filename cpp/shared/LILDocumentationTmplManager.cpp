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

#include "LILDocumentationTmplManager.h"
#include "LILErrorMessage.h"

using namespace LIL;

extern void LILPrintErrors(const std::vector<LILErrorMessage> & errors, const LILString & code);

LILDocumentationTmplManager::LILDocumentationTmplManager()
{
}

LILDocumentationTmplManager::~LILDocumentationTmplManager()
{
}

void LILDocumentationTmplManager::setTemplatePath(const std::string & path)
{
    this->_tmplPath = path;
}

void LILDocumentationTmplManager::loadTemplate(const std::string & file)
{
    if (this->_tmpls.count(file) == 0) {
        std::string filePath = this->_tmplPath+"/"+file;
        std::ifstream filestream(filePath, std::ios::in);
        if (filestream.fail()) {
            LILErrorMessage ei;
            ei.message = "\nERROR: Failed to read the file "+filePath;
            ei.file = filePath;
            ei.line = 0;
            ei.column = 0;
            std::vector<LILErrorMessage> errors = { ei };
            LILPrintErrors(errors, "");
            return;
        }
        
        std::stringstream buffer;
        buffer << filestream.rdbuf();
        
        this->_tmpls[file] = buffer.str();
    }
}


std::string LILDocumentationTmplManager::renderTemplate(const std::string & file, const std::unordered_map<std::string, std::string> tmplData) const
{
    std::string ret;
    //we first sort the ranges with a map
    std::map<size_t, std::pair<std::string, size_t>> ranges;
    const auto & str = this->_tmpls.at(file);
    for (const auto & pair : tmplData) {
        auto key = pair.first;
        auto keyPos = str.find("{{"+key+"}}");
        if (keyPos != std::string::npos) {
            ranges[keyPos] = { key,  keyPos+key.length()+4 };
        }
    }
    size_t i = 0;
    for (const auto & pair : ranges) {
        ret.append(str.substr(i, (pair.first-i)));
        ret.append(tmplData.at(pair.second.first));
        i = pair.second.second;
    }
    ret.append(str.substr(i, std::string::npos));
    return ret;
}
