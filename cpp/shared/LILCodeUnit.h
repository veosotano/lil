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

        void run();
        void buildAST();
        void runPasses();
        void compileToO(std::string outName);
        void compileToS(std::string outName);
        void printToOutput();
        void setIsMain(bool value);
        void setVerbose(bool value);
        void setDebugAST(bool value);
        void setDebugNeedsImporter(bool value);
        void setDebugASTValidator(bool value);
        void setDebugTypeGuesser(bool value);
        void setDebugStructureLowerer(bool value);
        void setDebugMethodInserter(bool value);
        void setDebugNameLowerer(bool value);
        void setDebugFieldSorter(bool value);
        void setDebugParameterSorter(bool value);

    private:
        LILCodeUnitPrivate * d;
    };
}

#endif
