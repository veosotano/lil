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
 *      This file contains all the os-specific implementations
 *
 ********************************************************************/

#ifndef LILPLATFORMSUPPORT_H
#define LILPLATFORMSUPPORT_H

#include "LILBasicValues.h"

std::string LIL_getCurrentDir();
std::string LIL_getAutoTargetString();
void LIL_makeDir(const std::string & dir);
std::string LIL_getExecutablePath();
std::string LIL_getExecutableDir();

#endif /* LILPLATFORMSUPPORT_H */
