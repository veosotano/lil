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
 *      This file contains the LILPrintErrors function
 *
 ********************************************************************/

#include "LILShared.h"
#include "LILErrorMessage.h"

using namespace LIL;

std::vector<std::string> LILSplitString(std::string phrase, std::string delimiter)
{
    std::vector<std::string> list;
    std::string s = phrase;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
}

void LILPrintErrors(const std::vector<LILErrorMessage> & errors, const LILString & code)
{
    std::vector<std::string> lines = LILSplitString(code.chardata(), "\n");

    std::cerr << "\nFound ";
    std::cerr << errors.size();
    std::cerr << " errors in your code:\n";
    for (auto it=errors.begin(); it!=errors.end(); ++it) {
        LILErrorMessage ei = *it;
        std::cerr << ei.message.chardata();
        if (ei.line > 0) {
            std::cerr << " on line ";
            std::cerr << ei.line;
            std::cerr << " column ";
            std::cerr << ei.column;
            std::cerr << "\n\n";

            if (ei.line > 2)
            {
                std::cerr << ei.line - 1;
                std::cerr << ": ";
                std::cerr << lines[ei.line-2];
                std::cerr << "\n";
            }

            std::cerr << ei.line;
            std::cerr << ": ";
            std::cerr << lines[ei.line-1];
            std::cerr << "\n";
            std::string indicator = "   ";
            if (ei.column > 2) {
                for (unsigned i=0; i<ei.column-3; ++i) {
                    indicator += " ";
                }
            }
            if (ei.column == 0) {
                indicator+= "^__\n";
            } else if (ei.column == 1) {
                indicator+= "_^__\n";
            } else {
                indicator+= "__^__\n";
            }
            std::cerr << indicator;

            if (ei.line < lines.size()-1)
            {
                std::cerr << ei.line+1;
                std::cerr << ": ";
                std::cerr << lines[ei.line];
                std::cerr << "\n";
            }
        } else {
            std::cerr << "\n";
        }
    }
    std::cerr << "\n";
}
