//
//  LILPassManager.hpp
//  LILInterpreterTest
//
//  Created by Miro Keller on 24/7/20.
//  Copyright © 2020 Miro Keller. All rights reserved.
//

#ifndef LILPASSMANAGER_H
#define LILPASSMANAGER_H

#include "LILShared.h"

namespace LIL {
    class LILVisitor;
    class LILRootNode;
    
    class LILPassManager
    {
    public:
        LILPassManager();
        virtual ~LILPassManager();

        void execute(const std::vector<LILVisitor *> & visitors, std::shared_ptr<LILRootNode> rootNode, const LILString & code);

        bool getVerbose() const;
        void setVerbose(bool value);
        bool hasErrors() const;
        
    private:
        bool _verbose;
        bool _hasErrors;
    };
}

#endif
