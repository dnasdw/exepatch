#ifndef UTILITY_H_
#define UTILITY_H_

#define COMPILER_MSC  1
#define COMPILER_GNUC 2

#if defined(_MSC_VER)
#define EXEPATCH_COMPILER COMPILER_MSC
#define EXEPATCH_COMPILER_VERSION _MSC_VER
#else
#define EXEPATCH_COMPILER COMPILER_GNUC
#define EXEPATCH_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#if EXEPATCH_COMPILER == COMPILER_MSC
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <io.h>
#else
#if defined(EXEPATCH_APPLE)
#include <mach-o/dyld.h>
#endif
#include <unistd.h>
#endif
#if EXEPATCH_COMPILER != COMPILER_MSC || (EXEPATCH_COMPILER == COMPILER_MSC && EXEPATCH_COMPILER_VERSION >= 1800)
#include <inttypes.h>
#else
#ifndef _PFX_8
#define _PFX_8 "hh"
#endif
#ifndef _PFX_16
#define _PFX_16 "h"
#endif
#ifndef _PFX_32
#define _PFX_32 "l"
#endif
#ifndef _PFX_64
#define _PFX_64 "ll"
#endif
#ifndef PRIu8
#define PRIu8 _PFX_8 "u"
#endif
#ifndef PRIu16
#define PRIu16 _PFX_16 "u"
#endif
#ifndef PRIu32
#define PRIu32 _PFX_32 "u"
#endif
#ifndef PRIu64
#define PRIu64 _PFX_64 "u"
#endif
#ifndef PRIX8
#define PRIX8 _PFX_8 "X"
#endif
#ifndef PRIX16
#define PRIX16 _PFX_16 "X"
#endif
#ifndef PRIX32
#define PRIX32 _PFX_32 "X"
#endif
#ifndef PRIX64
#define PRIX64 _PFX_64 "X"
#endif
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <string>

using namespace std;

typedef int8_t n8;
typedef int16_t n16;
typedef int32_t n32;
typedef int64_t n64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#if EXEPATCH_COMPILER == COMPILER_MSC
#define FFopen FFopenA
#define FFseek _fseeki64
#define FFtell _ftelli64
#define FFileno _fileno
#define FLseek _lseeki64
#define FChsize _chsize_s
#define MSC_PUSH_PACKED <pshpack1.h>
#define MSC_POP_PACKED <poppack.h>
#define GNUC_PACKED
#else
#define FFopen FFopenA
#define FFseek fseeko
#define FFtell ftello
#define FFileno fileno
#define FLseek lseek
#define FChsize ftruncate
#define MSC_PUSH_PACKED <stdlib.h>
#define MSC_POP_PACKED <stdlib.h>
#define GNUC_PACKED __attribute__((packed))
#endif

#define CONVERT_ENDIAN(n) (((n) >> 24 & 0xFF) | ((n) >> 8 & 0xFF00) | (((n) & 0xFF00) << 8) | (((n) & 0xFF) << 24))

const string& FGetModuleFile();

void FPadFile(FILE* a_fpFile, n64 a_nPadSize, u8 a_uPadData);

FILE* FFopenA(const char* a_pFileName, const char* a_pMode);

bool FSeek(FILE* a_fpFile, n64 a_nOffset);

#endif	// UTILITY_H_
