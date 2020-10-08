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
 *      This file defines the base class visitors
 *
 ********************************************************************/

#ifndef LILVISITOR_H
#define LILVISITOR_H

#include "LILNode.h"

namespace LIL
{
    class LILClassDecl;
    class LILErrorMessage;
    class LILVarName;
    class LILRootNode;

    class LILVisitor
    {
    public:
        LILVisitor();
        virtual ~LILVisitor();
        virtual void initializeVisit() = 0;
        virtual void performVisit(std::shared_ptr<LILRootNode> rootNode);
        virtual void visit(LILNode * node) = 0;
        bool hasErrors() const;
        void printErrors(const LILString & code) const;
        std::vector<std::string> splitString(std::string code, std::string delimiter) const;
        
        std::vector<LILErrorMessage> errors;

        void setPrintHeadline(bool value);
        bool getPrintHeadline() const;
        void setVerbose(bool value);
        bool getVerbose() const;
        void setDebug(bool value);
        bool getDebug() const;
        std::shared_ptr<LILNode> findNodeForVarName(LILVarName * name) const;
        std::shared_ptr<LILType> findTypeForVarName(std::shared_ptr<LILVarName> name) const;
        LILString decorate(LILString ns, LILString className, LILString name, std::shared_ptr<LILType> type) const;
        LILString typeToString(std::shared_ptr<LILType> type) const;
        
        void setRootNode(std::shared_ptr<LILRootNode> value);
        std::shared_ptr<LILRootNode> getRootNode() const;

        std::shared_ptr<LILClassDecl> findClassWithName(const LILString & name) const;
        std::shared_ptr<LILClassDecl> findAncestorClass(std::shared_ptr<LILNode> node) const;

    private:
        bool _printHeadline;
        bool _verbose;
        bool _debug;
        std::shared_ptr<LILRootNode> _rootNode;
    };
}

#endif
