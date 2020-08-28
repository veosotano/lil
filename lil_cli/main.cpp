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

#include "LILShared.h"
#include "LILCodeUnit.h"

using namespace LIL;

int main(int argc, const char * argv[]) {
    if (argc == 1) {
        return 0;
    }
    
    bool printOnly = false;
    bool compileToO = false;
    bool compileToS = false;
    bool verbose = false;
    bool debugAST = false;
    bool debugFieldSorter = false;
    bool debugParameterSorter = false;
    bool debugASTValidator = false;
    bool debugTypeGuesser = false;
    bool debugStructureLowerer = false;
    bool debugNameLowerer = false;
    bool debugIREmitter = false;
    int warningLevel = 0;
    std::string outName;
    std::string inName;
    
    for (int i=1, j=argc; i<j; /*intentionally left blank*/) {
        std::string command = argv[i];
        if (command == "-c" || command == "--just-compile") {
            //compile to .o file
            compileToO = true;
            if (compileToS) {
                compileToS = false;
            }
            ++i;
            
        } else if (command == "-s" || command == "--assembly"){
            //compile to assembly
            if (compileToO) {
                std::cerr << "Error: cannot use -c and -s at the same time.\n";
                exit(-1);
            } else {
                compileToS = true;
            }
            ++i;
            
        } else if (command == "-w"){
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
            
        } else if (command == "-o" || command == "--output"){
            //output to given filename
            if (argc<i+1) {
                std::cerr << "Error: no filename given after -o argument\n";
                exit(-1);
            }
            ++i;
            outName = argv[i];
            ++i;
        } else if (command == "-v" || command == "--verbose") {
            //verbose output
            verbose = true;
            ++i;
            
        } else if (command == "--print-only") {
            //output to std out instead of writing file
            printOnly = true;
            ++i;
            
        
        } else if (command == "--debug-ast") {
            debugAST = true;
            ++i;
            
        } else if (command == "--debug-field-sorter") {
            debugFieldSorter = true;
            ++i;
            
        } else if (command == "--debug-parameter-sorter") {
            debugParameterSorter = true;
            ++i;
            
        } else if (command == "--debug-validator") {
            debugASTValidator = true;
            ++i;
            
        } else if (command == "--debug-type-guesser") {
            debugTypeGuesser = true;
            ++i;
            
        } else if (command == "--debug-name-lowerer") {
            debugNameLowerer = true;
            ++i;
            
        } else if (command == "--debug-structure-lowerer") {
            debugStructureLowerer = true;
            ++i;
            
        } else if (command == "--debug-ir-emitter") {
            debugIREmitter = true;
            ++i;
            
        } else {
            //anything else is interpreted as the input filename
            inName = command;
            ++i;
        }
    }
    
    if (outName.length() == 0) {
        size_t slashIndex = inName.find_last_of("/");
        if (slashIndex == std::string::npos) {
            slashIndex = 0;
        } else {
            slashIndex += 1;
        }
        size_t dotIndex = inName.find_last_of(".");
        if (dotIndex != std::string::npos && dotIndex > slashIndex) {
            std::string name = inName.substr(slashIndex, dotIndex - slashIndex);
            if (compileToS) {
                outName = name + ".s";
            } else if (compileToO) {
                outName = name + ".o";
            } else {
                outName = name + ".out";
            }
            
        } else {
            outName = inName;
        }
    }
    
    if (verbose) {
        std::cerr << "Initializing...\n\n";
        if (!printOnly) {
            if (compileToO) {
                std::cerr << "Compile to .o file\n";
            } else if (compileToS){
                std::cerr << "Compile to assembly\n";
            }
            std::cerr << "Compile "+inName+" to produce "+outName+"\n";
        }
    }

    
    std::ifstream file(inName, std::ios::in);
    if (file.fail()) {
        std::cerr << "\nERROR: Failed to read the file "+inName+"\n\n";
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    LILString lilStr(buffer.str());
    
    std::unique_ptr<LILCodeUnit> codeUnit = std::make_unique<LILCodeUnit>();
    codeUnit->setVerbose(verbose);
    codeUnit->setDebugAST(debugAST);
    codeUnit->setDebugASTValidator(debugASTValidator);
    codeUnit->setDebugFieldSorter(debugFieldSorter);
    codeUnit->setDebugParameterSorter(debugParameterSorter);
    codeUnit->setDebugTypeGuesser(debugTypeGuesser);
    codeUnit->setDebugStructureLowerer(debugStructureLowerer);
    codeUnit->setDebugNameLowerer(debugNameLowerer);
    codeUnit->setDebugIREmitter(debugIREmitter);
    codeUnit->setFile(inName);
//    codeUnit->setDir(directory);
    codeUnit->setSource(lilStr);
    
    if (printOnly) {
        codeUnit->printToOutput();
    } else {
        if (compileToS) {
            codeUnit->compileToS(outName);
        } else {
            codeUnit->compileToO(outName);
        }
    }
    
    return 0;
}
