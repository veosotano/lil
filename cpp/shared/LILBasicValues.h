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
 *      This file contains the most fundamental value types to be used
 *      throughout the system
 *
 ********************************************************************/

#ifndef LILBASICVALUES_H
#define LILBASICVALUES_H

#include <algorithm>
#include <cstdio>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <deque>
#include <limits>
#include <list>
#include <stack>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iostream>
#include <cwchar>

namespace LIL
{
    struct LILRange
    {
    public:
        size_t location;
        size_t length;
    };

    struct LILVersion
    {
    public:
        int major;
        int minor;
        int patch;
        int build;
	};

	typedef float LILUnitF32;
	typedef double LILUnitF64;
	typedef unsigned char LILUnitI8;
	typedef short int LILUnitI16;
    typedef long int LILUnitI32;
	typedef long long int LILUnitI64;

	struct LILPoint
	{
	public:
		LILUnitF64 x;
		LILUnitF64 y;
    };

	struct LILSize
	{
	public:
		LILUnitF64 width;
		LILUnitF64 height;
    };

	struct LILRect
	{
	public:
		LILPoint origin;
		LILSize size;
    };
}

#endif
