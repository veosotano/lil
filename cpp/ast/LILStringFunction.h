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
 *      This file represents a string using parameters
 *
 ********************************************************************/

#ifndef LILSTRINGFUNCTION_H
#define LILSTRINGFUNCTION_H

#include "LILNode.h"

namespace LIL
{
    class LILStringFunction : public LILNode
    {
    public:
        LILStringFunction();
        LILStringFunction(const LILStringFunction & other);
        std::shared_ptr<LILStringFunction> clone() const;
        virtual ~LILStringFunction();
        void receiveNodeData(const LIL::LILString &data);
        void setStartChunk(LILString newValue);
        LILString getStartChunk() const;
        void addMidChunk(LILString newValue);
        std::vector<LILString> getMidChunks() const;
        void setEndChunk(LILString newValue);
        LILString getEndChunk() const;

        LILString stringRep();
        bool equalTo(std::shared_ptr<LILNode> otherNode);
        
        void add(std::shared_ptr<LILNode> node);
        const std::vector<std::shared_ptr<LILNode>> & getNodes() const;
        
        std::shared_ptr<LILType> getType() const override;

    private:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        LILString _startChunk;
        std::vector<LILString> _midChunks;
        LILString _endChunk;
    };
}

#endif
