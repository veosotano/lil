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
 *      This file is a instruction node
 *
 ********************************************************************/


#ifndef LILINSTRUCTION_H
#define LILINSTRUCTION_H


#include "LILNode.h"

namespace LIL
{
    class LILInstruction : public LILNode
    {
    public:
        LILInstruction();
        LILInstruction(const LILInstruction &other);
        std::shared_ptr<LILInstruction> clone() const;
        virtual ~LILInstruction();
        virtual LILString stringRep();
        virtual void receiveNodeData(const LILString &data);
        void setInstructionType(InstructionType value);
        InstructionType getInstructionType() const;
        void setName(LILString name);
        LILString getName() const;
        void setIsColorInstruction(bool value);
        bool getIsColorInstruction() const;
        
        void setArgument(std::shared_ptr<LILNode> value);
        std::shared_ptr<LILNode> getArgument() const;
        
    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        
    private:
        LILString _name;
        std::shared_ptr<LILNode> _argument;
        InstructionType _instructionType;
        bool _isColorInstruction;
    };
}

#endif
