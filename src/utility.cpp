#include "utility.h"

FILE* Fopen(const char* a_pFileName, const char* a_pMode, bool a_bVerbose /* = true */)
{
	FILE* fp = fopen(a_pFileName, a_pMode);
	if (fp == nullptr && a_bVerbose)
	{
		printf("ERROR: open file %s failed\n\n", a_pFileName);
	}
	return fp;
}

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
FILE* FopenW(const wchar_t* a_pFileName, const wchar_t* a_pMode, bool a_bVerbose /* = true */)
{
	FILE* fp = _wfopen(a_pFileName, a_pMode);
	if (fp == nullptr && a_bVerbose)
	{
		wprintf(L"ERROR: open file %ls failed\n\n", a_pFileName);
	}
	return fp;
}
#endif

bool Seek(FILE* a_fpFile, n64 a_nOffset)
{
	if (fflush(a_fpFile) != 0)
	{
		return false;
	}
	int nFd = Fileno(a_fpFile);
	if (nFd == -1)
	{
		return false;
	}
	Fseek(a_fpFile, 0, SEEK_END);
	n64 nFileSize = Ftell(a_fpFile);
	if (nFileSize < a_nOffset)
	{
		n64 nOffset = Lseek(nFd, a_nOffset - 1, SEEK_SET);
		if (nOffset == -1)
		{
			return false;
		}
		fputc(0, a_fpFile);
		fflush(a_fpFile);
	}
	else
	{
		Fseek(a_fpFile, a_nOffset, SEEK_SET);
	}
	return true;
}

void PadFile(FILE* a_fpFile, n64 a_nPadSize, u8 a_uPadData)
{
	const n64 nBufferSize = 0x100000;
	u8* pBuffer = new u8[nBufferSize];
	memset(pBuffer, a_uPadData, nBufferSize);
	while (a_nPadSize > 0)
	{
		n64 nSize = a_nPadSize > nBufferSize ? nBufferSize : a_nPadSize;
		fwrite(pBuffer, 1, static_cast<size_t>(nSize), a_fpFile);
		a_nPadSize -= nSize;
	}
	delete[] pBuffer;
}

#if defined(SDW_MAIN)
extern int UMain(int argc, UChar* argv[]);

int main(int argc, char* argv[])
{
	SetLocale();
	int nArgc = 0;
	UChar** pArgv = nullptr;
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
	pArgv = CommandLineToArgvW(GetCommandLineW(), &nArgc);
	if (pArgv == nullptr)
	{
		return 1;
	}
#else
	nArgc = argc;
	pArgv = argv;
#endif
	int nResult = UMain(nArgc, pArgv);
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
	LocalFree(pArgv);
#endif
	return nResult;
}
#endif

const UString& UGetModuleFileName()
{
	const u32 uMaxPath = 4096;
	static UString sFileName;
	if (!sFileName.empty())
	{
		return sFileName;
	}
	sFileName.resize(uMaxPath, USTR('\0'));
	u32 uSize = 0;
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
	uSize = GetModuleFileNameW(nullptr, &*sFileName.begin(), uMaxPath);
#elif SDW_PLATFORM == SDW_PLATFORM_MACOS
	char szPath[uMaxPath] = {};
	u32 uPathSize = uMaxPath;
	if (_NSGetExecutablePath(szPath, &uPathSize) != 0)
	{
		sFileName.clear();
		printf("ERROR: _NSGetExecutablePath error\n\n");
	}
	else if (realpath(szPath, &*sFileName.begin()) == nullptr)
	{
		sFileName.clear();
		printf("ERROR: realpath error\n\n");
	}
	uSize = strlen(sFileName.c_str());
#elif SDW_PLATFORM == SDW_PLATFORM_LINUX || SDW_PLATFORM == SDW_PLATFORM_CYGWIN
	ssize_t nCount = readlink("/proc/self/exe", &*sFileName.begin(), uMaxPath);
	if (nCount == -1)
	{
		sFileName.clear();
		printf("ERROR: readlink /proc/self/exe error\n\n");
	}
	else
	{
		sFileName[nCount] = '\0';
	}
	uSize = strlen(sFileName.c_str());
#endif
	sFileName.erase(uSize);
	sFileName = Replace(sFileName, USTR('\\'), USTR('/'));
	return sFileName;
}

void SetLocale()
{
#if SDW_PLATFORM == SDW_PLATFORM_MACOS
	setlocale(LC_ALL, "en_US.UTF-8");
#else
	setlocale(LC_ALL, "");
#endif
}

#if (SDW_COMPILER == SDW_COMPILER_MSC && SDW_COMPILER_VERSION < 1600) || (SDW_PLATFORM == SDW_PLATFORM_WINDOWS && SDW_COMPILER != SDW_COMPILER_MSC)
wstring U8ToW(const string& a_sString)
{
	int nLength = MultiByteToWideChar(CP_UTF8, 0, a_sString.c_str(), -1, nullptr, 0);
	wchar_t* pTemp = new wchar_t[nLength];
	MultiByteToWideChar(CP_UTF8, 0, a_sString.c_str(), -1, pTemp, nLength);
	wstring sString = pTemp;
	delete[] pTemp;
	return sString;
}
#elif (SDW_COMPILER == SDW_COMPILER_GNUC && SDW_COMPILER_VERSION < 50400) || SDW_PLATFORM == SDW_PLATFORM_CYGWIN
wstring U8ToW(const string& a_sString)
{
	return TSToS<string, wstring>(a_sString, "UTF-8", "WCHAR_T");
}
#else
wstring U8ToW(const string& a_sString)
{
	static wstring_convert<codecvt_utf8<wchar_t>> c_cvt_u8;
	return c_cvt_u8.from_bytes(a_sString);
}
#endif

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
wstring AToW(const string& a_sString)
{
	int nLength = MultiByteToWideChar(CP_ACP, 0, a_sString.c_str(), -1, nullptr, 0);
	wchar_t* pTemp = new wchar_t[nLength];
	MultiByteToWideChar(CP_ACP, 0, a_sString.c_str(), -1, pTemp, nLength);
	wstring sString = pTemp;
	delete[] pTemp;
	return sString;
}
#else
wstring AToW(const string& a_sString)
{
	return U8ToW(a_sString);
}
#endif

#if defined(SDW_XCONVERT)
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
wstring XToW(const string& a_sString, int a_nCodePage, const char* a_pCodeName)
{
	int nLength = MultiByteToWideChar(a_nCodePage, 0, a_sString.c_str(), -1, nullptr, 0);
	wchar_t* pTemp = new wchar_t[nLength];
	MultiByteToWideChar(a_nCodePage, 0, a_sString.c_str(), -1, pTemp, nLength);
	wstring sString = pTemp;
	delete[] pTemp;
	return sString;
}
#else
wstring XToW(const string& a_sString, int a_nCodePage, const char* a_pCodeName)
{
	return TSToS<string, wstring>(a_sString, a_pCodeName, "WCHAR_T");
}
#endif
#endif

static const int s_nFormatBufferSize = 4096;

string FormatV(const char* a_szFormat, va_list a_vaList)
{
	static char c_szBuffer[s_nFormatBufferSize] = {};
	vsnprintf(c_szBuffer, s_nFormatBufferSize, a_szFormat, a_vaList);
	return c_szBuffer;
}

wstring FormatV(const wchar_t* a_szFormat, va_list a_vaList)
{
	static wchar_t c_szBuffer[s_nFormatBufferSize] = {};
	vswprintf(c_szBuffer, s_nFormatBufferSize, a_szFormat, a_vaList);
	return c_szBuffer;
}

string Format(const char* a_szFormat, ...)
{
	va_list vaList;
	va_start(vaList, a_szFormat);
	string sFormatted = FormatV(a_szFormat, vaList);
	va_end(vaList);
	return sFormatted;
}

wstring Format(const wchar_t* a_szFormat, ...)
{
	va_list vaList;
	va_start(vaList, a_szFormat);
	wstring sFormatted = FormatV(a_szFormat, vaList);
	va_end(vaList);
	return sFormatted;
}
