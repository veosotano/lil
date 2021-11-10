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
 *      This file converts the ast into IR representation
 *
 ********************************************************************/

#ifndef LILCODEUNIT_H
#define LILCODEUNIT_H

#include "LILShared.h"

namespace llvm
{
    class Module;
}

namespace LIL
{
    class LILCodeUnitPrivate;
    class LILConfiguration;
    class LILNode;
    class LILRootNode;
    class LILCodeUnit
    {
    public:
        LILCodeUnit();
        virtual ~LILCodeUnit();

        std::shared_ptr<LILRootNode> getRootNode() const;

        void setFile(LILString file);
        LILString getFile() const;
        void setDir(LILString dir);
        LILString getDir() const;
        void setCompilerDir(LILString dir);
        LILString getCompilerDir() const;
        void setSource(LILString source);
        LILString getSource() const;
        void setNeedsConfigureDefaults(bool value);
        bool getNeedsConfigureDefaults() const;
        void setDebugConfigureDefaults(bool value);
        bool getDebugConfigureDefaults() const;
        void setIsBeingImportedWithNeeds(bool value);
        bool getIsBeingImportedWithNeeds() const;
        void setIsBeingImportedWithImport(bool value);
        bool getIsBeingImportedWithImport() const;
        const std::vector<LILString> & getArguments() const;
        void setArguments(const std::vector<LILString> & args);
        void setSuffix(const LILString & value);
        void setConstants(const std::vector<LILString> & values);
        void setImports(const std::vector<LILString> & values);
        void setConfiguration(LILConfiguration * value);

        void run();
        void buildAST();
        void runPasses();
        void runPassesForNeeds();
        void runPassesForImport();
        void compileToO(std::string outName);
        void compileToS(std::string outName);
        void printToOutput();
        void setIsMain(bool value);
        void setVerbose(bool value);
        bool getDebugStdLil() const;
        void setDebugStdLil(bool value);
        bool getImportStdLil() const;
        void setImportStdLil(bool value);
        const LILString & getStdLilPath() const;
        void setStdLilPath(const LILString & value);
        bool hasErrors() const;
        void addAlreadyImportedFile(const LILString & path, const std::vector<std::shared_ptr<LILNode>> & nodes, bool isNeeds);
        bool isAlreadyImported(const LILString & path, bool isNeeds);
        void addNeededFileForBuild(const LILString & path, bool verbose);
        const std::vector<std::pair<LILString, bool>> & getNeededFilesForBuild() const;

    private:
        LILCodeUnitPrivate * d;
    };
}

#endif
