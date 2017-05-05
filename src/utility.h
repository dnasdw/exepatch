#ifndef UTILITY_H_
#define UTILITY_H_

#define SDW_COMPILER_MSC   1
#define SDW_COMPILER_GNUC  2
#define SDW_COMPILER_CLANG 3

#if defined(_MSC_VER)
#define SDW_COMPILER SDW_COMPILER_MSC
#define SDW_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
#define SDW_COMPILER SDW_COMPILER_CLANG
#define SDW_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
#define SDW_COMPILER SDW_COMPILER_GNUC
#define SDW_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#error "compiler no support"
#endif

#define SDW_PLATFORM_WINDOWS 1
#define SDW_PLATFORM_LINUX   2
#define SDW_PLATFORM_MACOS   3
#define SDW_PLATFORM_CYGWIN  4

#if defined(_WIN32)
#define SDW_PLATFORM SDW_PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define SDW_PLATFORM SDW_PLATFORM_MACOS
#elif defined(__linux__)
#define SDW_PLATFORM SDW_PLATFORM_LINUX
#elif defined(__CYGWIN__)
#define SDW_PLATFORM SDW_PLATFORM_CYGWIN
#else
#error "platform no support"
#endif

#if SDW_COMPILER == SDW_COMPILER_MSC
#define SDW_MSC_PUSH_PACKED <pshpack1.h>
#define SDW_MSC_POP_PACKED <poppack.h>
#define SDW_GNUC_PACKED
#else
#define SDW_MSC_PUSH_PACKED <cstdlib>
#define SDW_MSC_POP_PACKED <cstdlib>
#define SDW_GNUC_PACKED __attribute__((packed))
#endif

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <io.h>
#if defined(SDW_MAIN)
#include <shellapi.h>
#endif
#else
#if SDW_PLATFORM == SDW_PLATFORM_MACOS
#include <mach-o/dyld.h>
#endif
#if (SDW_COMPILER == SDW_COMPILER_GNUC && SDW_COMPILER_VERSION < 50400) || SDW_PLATFORM == SDW_PLATFORM_CYGWIN || defined(SDW_XCONVERT)
#include <iconv.h>
#endif
#include <unistd.h>
#endif

#if SDW_COMPILER != SDW_COMPILER_MSC || (SDW_COMPILER == SDW_COMPILER_MSC && SDW_COMPILER_VERSION >= 1800)
#include <cinttypes>
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
#include <clocale>
#include <cstdarg>
#if SDW_COMPILER != SDW_COMPILER_MSC || (SDW_COMPILER == SDW_COMPILER_MSC && SDW_COMPILER_VERSION >= 1600)
#include <cstdint>
#else
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
#ifndef UINT64_C
#define UINT64_C(x)  (x ## ULL)
#endif
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#if SDW_COMPILER == SDW_COMPILER_CLANG || (SDW_COMPILER == SDW_COMPILER_MSC && SDW_COMPILER_VERSION >= 1600) || (SDW_COMPILER == SDW_COMPILER_GNUC && SDW_COMPILER_VERSION >= 50400)
#include <codecvt>
#endif
#include <locale>
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

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
#if SDW_COMPILER == SDW_COMPILER_MSC
#if SDW_COMPILER_VERSION < 1600
#define nullptr NULL
#endif
#endif
typedef wchar_t UChar;
typedef wstring UString;
#else
typedef char UChar;
typedef string UString;
#endif

#define SDW_BIT64(n) (UINT64_C(1) << (n))

#define SDW_CONVERT_ENDIAN32(n) (((n) >> 24 & 0xFF) | ((n) >> 8 & 0xFF00) | (((n) & 0xFF00) << 8) | (((n) & 0xFF) << 24))

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
#define Chsize _chsize_s
#define Fileno _fileno
#define UFopen FopenW
#define Fseek _fseeki64
#define Ftell _ftelli64
#define Lseek _lseeki64
#else
#define Chsize ftruncate
#define Fileno fileno
#define UFopen Fopen
#define Fseek fseeko
#define Ftell ftello
#define Lseek lseek
#endif

FILE* Fopen(const char* a_pFileName, const char* a_pMode, bool a_bVerbose = true);

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
FILE* FopenW(const wchar_t* a_pFileName, const wchar_t* a_pMode, bool a_bVerbose = true);
#endif

bool Seek(FILE* a_fpFile, n64 a_nOffset);

void PadFile(FILE* a_fpFile, n64 a_nPadSize, u8 a_uPadData);

#if !defined(SDW_MAIN)
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
#define UMain wmain
#else
#define UMain main
#endif
#endif

const UString& UGetModuleFileName();

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
#define USTR(x) L##x
#define PRIUS USTR("ls")
#define UPrintf wprintf
#else
#define USTR(x) x
#define PRIUS USTR("s")
#define UPrintf printf
#endif

void SetLocale();

#if (SDW_COMPILER == SDW_COMPILER_GNUC && SDW_COMPILER_VERSION < 50400) || SDW_PLATFORM == SDW_PLATFORM_CYGWIN || (SDW_PLATFORM != SDW_PLATFORM_WINDOWS && defined(SDW_XCONVERT))
template<typename TSrc, typename TDest>
TDest TSToS(const TSrc& a_sString, const string& a_sSrcType, const string& a_sDestType)
{
	TDest sConverted;
	iconv_t cd = iconv_open(a_sDestType.c_str(), a_sSrcType.c_str());
	if (cd == reinterpret_cast<iconv_t>(-1))
	{
		return sConverted;
	}
	size_t uStringLeft = a_sString.size() * sizeof(typename TSrc::value_type);
	static const n32 c_nBufferSize = 1024;
	static const n32 c_nConvertBufferSize = c_nBufferSize - 4;
	char szBuffer[c_nBufferSize];
	typename TSrc::value_type* pString = const_cast<typename TSrc::value_type*>(a_sString.c_str());
	do
	{
		char* pBuffer = szBuffer;
		size_t uBufferLeft = c_nConvertBufferSize;
		n32 nError = iconv(cd, reinterpret_cast<char**>(&pString), &uStringLeft, &pBuffer, &uBufferLeft);
#if SDW_PLATFORM == SDW_PLATFORM_MACOS
		if (nError >= 0 || (nError == static_cast<size_t>(-1) && errno == E2BIG))
#else
		if (nError == 0 || (nError == static_cast<size_t>(-1) && errno == E2BIG))
#endif
		{
			*reinterpret_cast<typename TDest::value_type*>(szBuffer + c_nConvertBufferSize - uBufferLeft) = 0;
			sConverted += reinterpret_cast<typename TDest::value_type*>(szBuffer);
			if (nError == 0)
			{
				break;
			}
		}
		else
		{
			break;
		}
	} while (true);
	iconv_close(cd);
	return sConverted;
}
#endif

wstring U8ToW(const string& a_sString);
wstring AToW(const string& a_sString);

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
#define AToU(x) AToW(x)
#else
#define AToU(x) string(x)
#endif

#if defined(SDW_XCONVERT)
wstring XToW(const string& a_sString, int a_nCodePage, const char* a_pCodeName);
#endif

string FormatV(const char* a_szFormat, va_list a_vaList);
wstring FormatV(const wchar_t* a_szFormat, va_list a_vaList);
string Format(const char* a_szFormat, ...);
wstring Format(const wchar_t* a_szFormat, ...);

template<typename T>
T Replace(const T& a_sString, typename T::value_type a_cSubChar, typename T::value_type a_cReplacement)
{
	T sString = a_sString;
	replace(sString.begin(), sString.end(), a_cSubChar, a_cReplacement);
	return sString;
}

#endif	// UTILITY_H_
