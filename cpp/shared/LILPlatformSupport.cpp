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
 *	  This file contains all the os-specific implementations
 *
 ********************************************************************/

#include "LILPlatformSupport.h"


#if defined(_WIN32) || defined(_WIN64)

#include <direct.h>
#include <Shlwapi.h>
#include <io.h>

std::string LIL_getAutoTargetString()
{
	return "windows";
}

std::string LIL_getCurrentDir()
{
	char cwdBuf[FILENAME_MAX];
	_getcwd(cwdBuf, FILENAME_MAX);
	return cwdBuf;
}

void LIL_makeDir(const std::string & path)
{
	_mkdir(path.c_str());
}

std::string LIL_getExecutablePath()
{
	char rawPathName[MAX_PATH];
	GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
	return std::string(rawPathName);
}

std::string LIL_getExecutableDir()
{
	std::string executablePath = LIL_getExecutablePath();
	char* exePath = new char[executablePath.length()];
	strcpy(exePath, executablePath.c_str());
	PathRemoveFileSpecA(exePath);
	std::string directory = std::string(exePath);
	delete[] exePath;
	return directory;
}

std::string LIL_mergePaths(std::string pathA, std::string pathB)
{
	char combined[MAX_PATH];
	PathCombineA(combined, pathA.c_str(), pathB.c_str());
	std::string mergedPath(combined);
	return mergedPath;
}

#elif defined(__APPLE__)

#include <unistd.h>
#include <libgen.h>
#include <mach-o/dyld.h>

#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_OS_SIMULATOR
std::string LIL_getAutoTargetString()
{
	return "ios";
}
#elif TARGET_OS_IPHONE && TARGET_OS_MACCATALYST
std::string LIL_getAutoTargetString()
{
	return "ios";
}
#elif TARGET_OS_IPHONE
std::string LIL_getAutoTargetString()
{
	return "ios";
}
#elif TARGET_OS_MAC
std::string LIL_getAutoTargetString()
{
	return "mac";
}
#endif

std::string LIL_getCurrentDir()
{
	char cwdBuf[FILENAME_MAX];
	getcwd(cwdBuf, FILENAME_MAX);
	std::string ret(cwdBuf);
	return ret;
}

void LIL_makeDir(const std::string & path)
{
	std::string commandStr = "mkdir -p \""+path+"\"";
	system(commandStr.c_str());
}

std::string LIL_getExecutablePath() {
	char rawPathName[PATH_MAX];
	char realPathName[PATH_MAX];
	uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);
	
	if(!_NSGetExecutablePath(rawPathName, &rawPathSize)) {
		realpath(rawPathName, realPathName);
	}
	return  std::string(realPathName);
}

std::string LIL_getExecutableDir() {
	std::string executablePath = LIL_getExecutablePath();
	char *executablePathStr = new char[executablePath.length() + 1];
	strcpy(executablePathStr, executablePath.c_str());
	char* executableDir = dirname(executablePathStr);
	delete [] executablePathStr;
	return std::string(executableDir);
}

#elif __ANDROID__ || __linux

#include <unistd.h>
#include <limits.h>
#include <libgen.h>

#if __ANDROID 
std::string LIL_getAutoTargetString()
{
	return "android";
}
#else
std::string LIL_getAutoTargetString()
{
	return "linux";
}
#endif

std::string LIL_getCurrentDir()
{
	char cwdBuf[FILENAME_MAX];
	getcwd(cwdBuf, FILENAME_MAX);
	std::string ret(cwdBuf);
	return ret;
}

std::string LIL_getExecutablePath() {
	char rawPathName[PATH_MAX];
	realpath("/proc/self/exe", rawPathName);
	return  std::string(rawPathName);
}

std::string LIL_getExecutableDir() {
	std::string executablePath = LIL_getExecutablePath();
	char *executablePathStr = new char[executablePath.length() + 1];
	strcpy(executablePathStr, executablePath.c_str());
	char* executableDir = dirname(executablePathStr);
	delete [] executablePathStr;
	return std::string(executableDir);
}

std::string LIL_mergePaths(std::string pathA, std::string pathB) {
	return pathA+"/"+pathB;
}

#endif
