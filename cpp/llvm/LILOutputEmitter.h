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
 *      This file generates the final output using LLVM
 *
 ********************************************************************/

#ifndef LILOUTPUTEMITTER_H
#define LILOUTPUTEMITTER_H

#include "LILShared.h"

namespace llvm
{
    class Module;
}

namespace LIL
{
    class LILCodeUnit;
    class LILOutputEmitterPrivate;
    class LILOutputEmitter
    {
    public:
        LILOutputEmitter();
        virtual ~LILOutputEmitter();
        
        llvm::Module * getLLVMModule() const;
        
        void setFile(LILString file);
        LILString getFile() const;
        void setDir(LILString dir);
        LILString getDir() const;
        void setSource(LILString source);
        LILString getSource() const;
        bool hasErrors() const;
        
        void run(LILCodeUnit * cu);
        void compileToO(std::string outName, LILCodeUnit * cu);
        void compileToS(std::string outName, LILCodeUnit * cu);
        void printToOutput(LILCodeUnit * cu);
        void setVerbose(bool value);
        void setDebugIREmitter(bool value);
        
    private:
        LILOutputEmitterPrivate * d;
    };
}

#endif
