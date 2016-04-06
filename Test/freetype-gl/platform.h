#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdlib.h>

//-------------------------------------------------
// stdint.h is not available on VS2008 or lower
//-------------------------------------------------
#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif // _MSC_VER


namespace ftgl 
{


#ifdef __APPLE__
    /* strndup() was only added in OSX lion */
    char * strndup( const char *s1, size_t n);
#elif defined(_WIN32) || defined(_WIN64)
    /* does not exist on windows */
    char * strndup( const char *s1, size_t n);
#	if !defined(_MSC_VER) || _MSC_VER < 1800
		double round(double v);
#	endif // _MSC_VER
#    pragma warning (disable: 4244) // suspend warnings
#endif // _WIN32 || _WIN64


}

#endif /* __PLATFORM_H__ */
