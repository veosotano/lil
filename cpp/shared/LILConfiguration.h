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
 *      This file is a container for configuration values
 *
 ********************************************************************/

#ifndef LILCONFIGURATION
#define LILCONFIGURATION

#include "LILShared.h"

namespace LIL {
    class LILNode;
    
    class LILConfiguration {
    public:
        LILConfiguration();
        ~LILConfiguration();
        bool hasConfig(const std::string & name) const;
        bool getConfigBool(const std::string & name) const;
        const std::string getConfigString(const std::string & name) const;
        long int getConfigInt(const std::string & name) const;
        std::vector<std::shared_ptr<LILNode>> & getConfigItems(const std::string & name);
        void applyConfig(const std::shared_ptr<LILNode> & node);
        void clearConfig(const std::string & name);
        void setConfig(const std::string & name, std::shared_ptr<LILNode> value);
        void addConfig(const std::string & name, std::shared_ptr<LILNode> value);
        void printConfig() const;
        std::string extractString(std::shared_ptr<LILNode> val) const;
    private:
        std::unordered_map<std::string, std::vector<std::shared_ptr<LILNode>>> _values;
        std::vector<std::shared_ptr<LILNode>> _empty;
    };
}

#endif /* LILCONFIGURATION */
