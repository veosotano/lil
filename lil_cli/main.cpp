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
 *      This file is the main compiler driver, used through the CLI
 *
 ********************************************************************/
#include <iostream>
#include <fstream>

#if defined(_WIN32) || defined(_WIN64)

#include <direct.h>
#define current_dir _getcwd

#else

#include <unistd.h>
#define current_dir getcwd

#endif

#include "LILShared.h"
#include "LILBuildManager.h"
#include "LILCodeUnit.h"
#include "LILRootNode.h"

using namespace LIL;

int main(int argc, const char * argv[]) {
    if (argc == 1) {
        return 0;
    }

    bool verbose = false;
    bool noConfigureDefaults = false;
    bool debugConfigureDefaults = false;

    int warningLevel = 0;
    std::string inName;
    std::string localDir;
    std::vector<LILString> arguments;
    
    for (int i=1, j=argc; i<j; /*intentionally left blank*/) {
        std::string command = argv[i];
        if (command == "-w"){
            //set warning levels
            if (argc<i+1) {
                std::cerr << "Error: no warning level given after -w argument\n";
                exit(-1);
            }
            ++i;
            std::string warningLevelStr = argv[i];
            std::cerr << "Using warning level "+warningLevelStr;
            warningLevel = std::stoi(warningLevelStr);
            ++i;

        } else if (command == "-v" || command == "--verbose") {
            //verbose output
            verbose = true;
            arguments.push_back(command);
            ++i;

        } else if (command == "--no-configure-defaults") {
            noConfigureDefaults = true;
            arguments.push_back(command);
            ++i;

        } else if (command == "--debug-configure-defaults") {
            debugConfigureDefaults = true;
            arguments.push_back(command);
            ++i;
            
        } else if (command.substr(0, 2) == "--") {
            arguments.push_back(command);
            ++i;

        } else {
            //anything else is interpreted as the input filename
            inName = command;
            ++i;
        }
    }
    
    size_t slashIndex = inName.find_last_of("/");
    if (slashIndex != std::string::npos) {
        localDir = inName.substr(0, slashIndex);
    } else {
        localDir = "";
    }

    if (verbose) {
        std::cerr << "Initializing...\n\n";
        std::cerr << "Input file was: \n";
        std::cerr << inName << "\n\n";
        std::cerr << "Arguments:\n";
        for (auto arg : arguments) {
            std::cerr << arg.data() << "\n";
        }
        std::cerr << "\n\n";
    }
    
    char cwdBuf[FILENAME_MAX];
    current_dir(cwdBuf, FILENAME_MAX);
    std::string directory(cwdBuf);

    std::unique_ptr<LILBuildManager> buildMgr = std::make_unique<LILBuildManager>();

    buildMgr->setArguments(std::move(arguments));

    buildMgr->setDirectory(directory);
    buildMgr->setFile(inName);

    buildMgr->setNoConfigureDefaults(noConfigureDefaults);
    buildMgr->setDebugConfigureDefaults(debugConfigureDefaults);
    buildMgr->setVerbose(verbose);
    buildMgr->setWarningLevel(warningLevel);

    buildMgr->read();
    if (buildMgr->hasErrors()) {
        return -1;
    }
    buildMgr->configure();
    if (buildMgr->hasErrors()) {
        return -1;
    }
    buildMgr->build();
    if (buildMgr->hasErrors()) {
        return -1;
    }
    return 0;
}
