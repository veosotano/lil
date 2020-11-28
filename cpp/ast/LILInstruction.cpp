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

#include "LILInstruction.h"

using namespace LIL;

LILInstruction::LILInstruction()
: LIL::LILNode(NodeTypeInstruction)
, _instructionType(InstructionTypeNone)
, _isColorInstruction(false)
, _verbose(true)
{
    this->_instructionType = InstructionTypeNone;
}

LILInstruction::LILInstruction(const LILInstruction &other)
: LILNode(other)
{
    this->_instructionType = other._instructionType;
    this->_name = other._name;
    this->_isColorInstruction = other._isColorInstruction;
    this->_argument = other._argument;
    this->_verbose = other._verbose;
}

std::shared_ptr<LILInstruction> LILInstruction::clone() const
{
    return std::static_pointer_cast<LILInstruction> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILInstruction::cloneImpl() const
{
    std::shared_ptr<LILInstruction> clone(new LILInstruction(*this));
    if (this->_argument) {
        clone->setArgument(this->_argument->clone());
    }
    return clone;
}

LILInstruction::~LILInstruction()
{
    
}

LILString LILInstruction::stringRep()
{
    switch (this->getInstructionType()) {
        case InstructionTypeGrayscale1:
        case InstructionTypeGrayscale2:
        case InstructionTypeRGB:
        case InstructionTypeRGBA:
        case InstructionTypeRGBAA:
        case InstructionTypeRRGGBB:
        case InstructionTypeRRGGBBA:
        case InstructionTypeRRGGBBAA:
        {
            return this->getName();
        }

        case InstructionTypeNew:
        case InstructionTypeMove:
        case InstructionTypeDelete:
        case InstructionTypeNeeds:
        case InstructionTypeConfigure:
        case InstructionTypeFinally:
        {
            return this->getName();
        }
            
        default:
            break;
    }
    return "Instruction";
}

void LILInstruction::receiveNodeData(const LILString &data)
{
    
}

void LILInstruction::setInstructionType(InstructionType value)
{
    this->_instructionType = value;
}

InstructionType LILInstruction::getInstructionType() const
{
    return this->_instructionType;
}

bool LILInstruction::isA(InstructionType otherType) const
{
    return this->_instructionType == otherType;
}

void LILInstruction::setName(LILString value)
{
    this->_name = value;
}

LILString LILInstruction::getName() const
{
    return this->_name;
}


void LILInstruction::setIsColorInstruction(bool value)
{
    this->_isColorInstruction = value;
}

bool LILInstruction::getIsColorInstruction() const
{
    return this->_isColorInstruction;
}

void LILInstruction::setArgument(std::shared_ptr<LILNode> value)
{
    this->_argument = value;
}

std::shared_ptr<LILNode> LILInstruction::getArgument() const
{
    return this->_argument;
}

void LILInstruction::setVerbose(bool value)
{
    this->_verbose = value;
}

bool LILInstruction::getVerbose() const
{
    return this->_verbose;
}
