#ifndef HH__
#define HH__

// SECTION(HEADER)
#ifndef _WIN32
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif // not _DEFAULT_SOURCE
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif // _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#else
#ifdef __MINGW32__
#ifdef _MSC_VER
#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO
#endif // not __USE_MINGW_ANSI_STDIO
#endif // _MSC_VER
#endif // __MINGW32__
#endif // _WIN32

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
// SECTION(HEADER, END)

#ifdef HH_IMPLEMENTATION
// SECTION(IMPLEMENTATION)
// implementation-exclusive includes
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
#include <math.h>
#include <float.h>
#include <limits.h>
#endif // HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE

// platform-dependent includes
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif // _WIN32
// SECTION(IMPLEMENTATION, END)
#endif // HH_IMPLEMENTATION
#endif // HH__