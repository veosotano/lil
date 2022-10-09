/********************************************************************
 *
 *	  LIL Is a Language
 *
 *	  AUTHORS: Miro Keller
 *
 *	  COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *	  LICENSE: see LICENSE file
 *
 *	  This file implements cloning behavior
 *
 ********************************************************************/

#include "LILShared.h"
#include "LILClonable.h"

using namespace LIL;

LILClonable::~LILClonable()
{
}

std::shared_ptr<LILClonable> LILClonable::clone() const
{
	return this->cloneImpl();
}

std::shared_ptr<LILClonable> LILClonable::cloneImpl() const
{
	return std::shared_ptr<LILClonable>(new LILClonable(*this));
}
