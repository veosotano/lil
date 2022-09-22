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
    class LILElement;
    class LILRootNode;
    class LILOutputEmitterPrivate;
    class LILOutputEmitter
    {
    public:
        LILOutputEmitter();
        virtual ~LILOutputEmitter();

        void prepare();

        llvm::Module * getLLVMModule() const;
        
        void setInFile(const LILString & file);
        const LILString & getInFile() const;
        void setOutFile(const LILString & file);
        const LILString & getOutFile() const;
        void setDir(const LILString & dir);
        const LILString & getDir() const;
        void setSource(const LILString & source);
        const LILString & getSource() const;
        void setCPU(const LILString & value);
        const LILString & getCPU() const;
        void setVendor(const LILString & value);
        const LILString & getVendor() const;
        void setDOM(const std::shared_ptr<LILElement> & dom) const;
        
        void run(std::shared_ptr<LILRootNode> rootNode);
        void compileToO(std::shared_ptr<LILRootNode> rootNode);
        void compileToS(std::shared_ptr<LILRootNode> rootNode);
        void printToOutput(std::shared_ptr<LILRootNode> rootNode);
        void setVerbose(bool value);
        void setDebugIREmitter(bool value);
        
    private:
        LILOutputEmitterPrivate * d;
    };
}

#endif
