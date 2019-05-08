#ifndef _G_FRAME_PUBLIC_H
#define _G_FRAME_PUBLIC_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif

#pragma warning (disable:4290)

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <io.h>
#include <direct.h>

#include <string>
#include <vector>
#include <map>
#include <deque>
#include <iterator>

#ifdef WIN32
#include <process.h>
#include <winsock2.h>
#else
#include <pthread.h>
#endif

#ifdef WIN32
#define snprintf _snprintf
#define access _access
#define mkdir(a,b) _mkdir(a)
#endif 

#ifndef APPDLL
#define DLL_API 
#else
#ifdef APPDLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#ifdef WIN32
#define DLL_API __declspec(dllimport)
#else
#define DLL_API 
#endif
#endif
#endif


#define INVALID_DOUBLE             999999999.99
#define IS_INVALID_DOUBLE(n)       (((n) + 1.0)>INVALID_DOUBLE)
#define IS_DOUBLE_EQUAL(a, b)      ((a)-(b) > -0.00001 && (a)-(b) < 0.00001)


#endif
