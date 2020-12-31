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
 *      This file implements the #if instructions
 *
 ********************************************************************/

#ifndef LILIFINSTRUCTION_H
#define LILIFINSTRUCTION_H


#include "LILInstruction.h"

namespace LIL
{
    class LILIfInstruction : public LILInstruction
    {
    public:
        LILIfInstruction();
        LILIfInstruction(const LILIfInstruction &other);
        std::shared_ptr<LILIfInstruction> clone() const;
        virtual ~LILIfInstruction();
        void receiveNodeData(const LILString &data) override;
        
        void addThen(std::shared_ptr<LILNode> node);
        const std::vector<std::shared_ptr<LILNode>> & getThen() const;
        void setThen(std::vector<std::shared_ptr<LILNode>> newThen);
        void addElse(std::shared_ptr<LILNode> node);
        const std::vector<std::shared_ptr<LILNode>> & getElse() const;
        void setElse(std::vector<std::shared_ptr<LILNode>> newElse);
        
        void setReceivesThen(bool newValue);
        bool getReceivesThen() const;
        
        void setReceivesElse(bool newValue);
        bool getReceivesElse() const;
        
    protected:
        std::shared_ptr<LILClonable> cloneImpl() const  override;
        
    private:
        std::vector<std::shared_ptr<LILNode>> _then;
        std::vector<std::shared_ptr<LILNode>> _else;
        bool _receivesThen;
        bool _receivesElse;
    };
}

#endif
