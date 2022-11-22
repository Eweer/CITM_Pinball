#ifndef __DIRENT_H__
#define __DIRENT_H__
#include "dirent/dirent.h"
#endif //__DIRENT_H__

#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <string_view>
#include <functional>

//  NULL just in case ----------------------

#ifdef NULL
#undef NULL
#endif
#define NULL  0

constexpr auto TICKS_FOR_NEXT_FRAME = (1000 / 60);
constexpr auto FPS_INTERVAL = 1.0f;

// Deletes a buffer
#define RELEASE( x ) \
	{						\
	if( x != NULL )		\
		{					  \
	  delete x;			\
	  x = NULL;			  \
		}					  \
	}

// Deletes an array of buffers
#define RELEASE_ARRAY( x ) \
	{							  \
	if( x != NULL )			  \
		{							\
	  delete[] x;				\
	  x = NULL;					\
		}							\
							  \
	}

#define IN_RANGE( value, min, max ) ( ((value) >= (min) && (value) <= (max)) ? 1 : 0 )
#define MIN( a, b ) ( ((a) < (b)) ? (a) : (b) )
#define MAX( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define TO_BOOL( a )  ( (a != 0) ? true : false )
#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

using uint = unsigned int;
using uchar = unsigned char;
using uint32 = unsigned int;
using uint64 = unsigned long long;

constexpr unsigned int str2int(const char *str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

template <class VALUE_TYPE> void SWAP(VALUE_TYPE& a, VALUE_TYPE& b)
{
	VALUE_TYPE tmp = a;
	a = b;
	b = tmp;
}

// Standard string size
#define SHORT_STR	 32
#define MID_STR	    255
#define HUGE_STR   8192

// Joins a path and file
inline const char* const PATH(const char* folder, const char* file)
{
	static char path[MID_STR];
	sprintf_s(path, MID_STR, "%s/%s", folder, file);
	return path;
}

inline int DescAlphasort(const struct dirent **c, const struct dirent **d)
{
	return alphasort(d, c);
}

struct StringHash
{
	using is_transparent = void;
	[[nodiscard]] size_t operator()(const char *txt) const
	{
		return std::hash<std::string_view>{}(txt);
	}
	[[nodiscard]] size_t operator()(std::string_view txt) const
	{
		return std::hash<std::string_view>{}(txt);
	}
	[[nodiscard]] size_t operator()(const std::string &txt) const
	{
		return std::hash<std::string>{}(txt);
	}
};

// Performance macros
#define PERF_START(timer) timer.Start()
#define PERF_PEEK(timer) LOG("%s took %f ms", __FUNCTION__, timer.ReadMs())

#endif	// __DEFS_H__