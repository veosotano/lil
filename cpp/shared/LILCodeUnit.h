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
        void setSource(LILString source);
        LILString getSource() const;
        void setNeedsConfigureDefaults(bool value);
        bool getNeedsConfigureDefaults() const;
        void setIsBeingImportedWithNeeds(bool value);
        bool getIsBeingImportedWithNeeds() const;
        void setIsBeingImportedWithImport(bool value);
        bool getIsBeingImportedWithImport() const;
        const std::vector<LILString> & getCustomArgs() const;
        void setCustomArgs(std::vector<LILString> & args);

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
        void setDebugAST(bool value);
        void setDebugLilStd(bool value);
        void setDebugPreprocessor(bool value);
        void setDebugASTValidator(bool value);
        void setDebugTypeResolver(bool value);
        void setDebugClassTemplateLowerer(bool value);
        void setDebugTypeGuesser(bool value);
        void setDebugStructureLowerer(bool value);
        void setDebugConstantFolder(bool value);
        void setDebugMethodInserter(bool value);
        void setDebugNameLowerer(bool value);
        void setDebugFieldSorter(bool value);
        void setDebugParameterSorter(bool value);
        void setDebugTypeValidator(bool value);
        void setDebugConversionInserter(bool value);
        bool hasErrors() const;
        void addAlreadyImportedFile(const LILString & path, const std::vector<std::shared_ptr<LILNode>> & nodes, bool isNeeds);
        bool isAlreadyImported(const LILString & path, bool isNeeds);

    private:
        LILCodeUnitPrivate * d;
    };
}

#endif
