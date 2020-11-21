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
 *      This file stores code written in another language
 *
 ********************************************************************/

#include "LILForeignLang.h"

using namespace LIL;

LILForeignLang::LILForeignLang()
: LILNode(NodeTypeForeignLang)
{
    
}

LILForeignLang::LILForeignLang(const LILForeignLang &other)
: LILNode(other)
{
    this->_language = other._language;
    this->_content = other._content;
}

std::shared_ptr<LILForeignLang> LILForeignLang::clone() const
{
    return std::static_pointer_cast<LILForeignLang> (this->cloneImpl());
}

std::shared_ptr<LILClonable> LILForeignLang::cloneImpl() const
{
    std::shared_ptr<LILForeignLang> clone(new LILForeignLang(*this));
    return clone;
}

LILForeignLang::~LILForeignLang()
{
    
}

void LILForeignLang::receiveNodeData(const LIL::LILString &data)
{
    this->setContent(data);
}

LILString LILForeignLang::stringRep()
{
    return this->_content;
}

void LILForeignLang::setLanguage(LILString value)
{
    this->_language = value;
}

const LILString & LILForeignLang::getLanguage() const
{
    return this->_language;
}

void LILForeignLang::setContent(LILString value)
{
    this->_content = value;
}

const LILString & LILForeignLang::getContent() const
{
    return this->_content;
}
