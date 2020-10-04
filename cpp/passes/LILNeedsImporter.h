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
 *      This file imports code from other files when using #needs
 *
 ********************************************************************/

#ifndef LILNEEDSIMPORTER_H
#define LILNEEDSIMPORTER_H

#include "LILVisitor.h"

namespace LIL
{
    class LILRootNode;
    class LILNeedsImporter : public LILVisitor
    {
    public:
        LILNeedsImporter();
        virtual ~LILNeedsImporter();
        void initializeVisit() override;
        void visit(LILNode * node) override;
        void performVisit(std::shared_ptr<LILRootNode> rootNode) override;
        void setDir(LILString dir);
        LILString getDir() const;
        bool getDebugAST() const;
        void setDebugAST(bool value);

    private:
        LILString _dir;
        bool _debugAST;
        std::vector<LILString> _resolveFilePaths(LILString argStr) const;
        std::vector<std::string> _glob(const std::string& pattern) const;
        void _getNodesForImport(std::vector<std::shared_ptr<LILNode>> * newNodes, std::shared_ptr<LILRootNode> rootNode);
    };
}


#endif /* LILNEEDSIMPORTER_H */
