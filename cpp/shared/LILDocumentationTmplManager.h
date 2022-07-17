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

#ifndef LILDOCUMENTATIONTMPLMANAGER_H
#define LILDOCUMENTATIONTMPLMANAGER_H

#include "LILShared.h"

namespace LIL
{
    class LILDocumentationTmplManager
    {
    public:
        LILDocumentationTmplManager();
        virtual ~LILDocumentationTmplManager();
        void setTemplatePath(const std::string & path);
        void loadTemplate(const std::string & file);
        std::string renderTemplate(const std::string & file, const std::unordered_map<std::string, std::string> tmplData) const;
        
    private:
        std::unordered_map<std::string, std::string> _tmpls;
        std::string _tmplPath;
    };
}

#endif /* LILDOCUMENTATIONTMPLMANAGER_H */
