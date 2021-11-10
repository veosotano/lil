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
 *      This file is responsible for coordinating a build
 *
 ********************************************************************/

#ifndef LILBUILDMANAGER
#define LILBUILDMANAGER

#include "LILNode.h"

namespace LIL {
    class LILCodeUnit;
    class LILConfiguration;
    class LILErrorMessage;
    class LILRule;
    
    class LILBuildManager
    {
    public:
        LILBuildManager();
        virtual ~LILBuildManager();
        void read();
        void configure();
        void build();
        bool hasErrors() const;
        void setDirectory(LILString value);
        void setFile(LILString value);
        void setCompilerDir(LILString value);
        void setVerbose(bool value);
        void setNoConfigureDefaults(bool value);
        void setDebugConfigureDefaults(bool value);
        void setPrintOnly(bool value);
        void setSingleFile(bool value);
        void setWarningLevel(int value);
        void setArguments(std::vector<LILString> && args);
    private:
        std::unique_ptr<LILConfiguration> _config;
        std::unique_ptr<LILCodeUnit> _codeUnit;
        std::vector<LILErrorMessage> _errors;
        std::vector<LILString> _arguments;
        LILString _directory;
        LILString _file;
        LILString _compilerDir;
        LILString _minOSVersion;
        bool _hasErrors;
        bool _debug;
        bool _verbose;
        bool _noConfigureDefaults;
        bool _debugConfigureDefaults;
        bool _compileToS;
        int _warningLevel;
    };
}

#endif /* LILBuildManager */
