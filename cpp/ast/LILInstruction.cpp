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

LILString LILInstruction::instructionTypeToString(InstructionType instrType)
{
    switch (instrType) {
        case InstructionTypeNew:
            return "new";
        case InstructionTypeMove:
            return "move";
        case InstructionTypeDelete:
            return "delete";
        case InstructionTypeGrayscale1:
            return "grayscale1";
        case InstructionTypeGrayscale2:
            return "grayscale2";
        case InstructionTypeRGB:
            return "rgb";
        case InstructionTypeRGBA:
            return "rgba";
        case InstructionTypeRGBAA:
            return "rgbaa";
        case InstructionTypeRRGGBB:
            return "rrggbb";
        case InstructionTypeRRGGBBA:
            return "rrggbba";
        case InstructionTypeRRGGBBAA:
            return "rrggbbaa";
        case InstructionTypeConfigure:
            return "configure";
        case InstructionTypeGetConfig:
            return "getConfig";
        case InstructionTypeNeeds:
            return "needs";
        case InstructionTypeImport:
            return "import";
        case InstructionTypeExport:
            return "export";
        case InstructionTypeIf:
            return "if";
        case InstructionTypePaste:
            return "paste";
        case InstructionTypeSnippet:
            return "snippet";
        case InstructionTypeBug:
            return "bug";
        case InstructionTypeArg:
            return "arg";
        case InstructionTypeExpand:
            return "expand";
        default:
            return "ERROR: unknown instruction type";
    }
}

LILInstruction::LILInstruction(NodeType nodeTy)
: LILTypedNode(nodeTy)
, _instructionType(InstructionTypeNone)
, _isColorInstruction(false)
, _verbose(true)
, _receivesBody(true)
{
}

LILInstruction::LILInstruction(const LILInstruction &other)
: LILTypedNode(other)
{
    this->_instructionType = other._instructionType;
    this->_name = other._name;
    this->_isColorInstruction = other._isColorInstruction;
    this->_argument = other._argument;
    this->_verbose = other._verbose;
    this->_receivesBody = other._receivesBody;
}

std::shared_ptr<LILInstruction> LILInstruction::clone() const
{
    return std::static_pointer_cast<LILInstruction> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILInstruction::cloneImpl() const
{
    std::shared_ptr<LILInstruction> clone(new LILInstruction(*this));
    clone->clearChildNodes();
    if (this->_argument) {
        clone->setArgument(this->_argument->clone());
    }
    //clone LILTypedNode
    if (this->_type) {
        clone->setType(this->_type->clone());
    }
    return clone;
}

LILInstruction::~LILInstruction()
{
    
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
    if (this->_argument) {
        this->removeNode(this->_argument);
    }
    this->addNode(value);
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

bool LILInstruction::getReceivesBody() const
{
    return this->_receivesBody;
}

void LILInstruction::setReceivesBody(bool value)
{
    this->_receivesBody = value;
}
