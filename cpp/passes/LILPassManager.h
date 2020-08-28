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
        void addPass(std::unique_ptr<LILVisitor> visitor);
        void execute(std::shared_ptr<LILRootNode> rootNode, const LILString & code);
        const std::vector<std::unique_ptr<LILVisitor>> & getPasses() const;
    
        bool getVerbose() const;
        void setVerbose(bool value);
        bool hasErrors() const;
        
    private:
        std::vector<std::unique_ptr<LILVisitor>> _visitors;
        bool _verbose;
    };
}

#endif
