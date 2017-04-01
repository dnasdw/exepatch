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
		printf("ERROR: _NSGetExecutablePath error\n\n");
		sFileName.clear();
	}
	else if (realpath(szPath, &*sFileName.begin()) == nullptr)
	{
		printf("ERROR: realpath error\n\n");
		sFileName.clear();
	}
	uSize = strlen(sFileName.c_str());
#elif SDW_PLATFORM == SDW_PLATFORM_LINUX || SDW_PLATFORM == SDW_PLATFORM_CYGWIN
	ssize_t nCount = readlink("/proc/self/exe", &*sFileName.begin(), uMaxPath);
	if (nCount == -1)
	{
		printf("ERROR: readlink /proc/self/exe error\n\n");
		sFileName.clear();
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
string WToU8(const wstring& a_sString)
{
	int nLength = WideCharToMultiByte(CP_UTF8, 0, a_sString.c_str(), -1, nullptr, 0, nullptr, nullptr);
	char* pTemp = new char[nLength];
	WideCharToMultiByte(CP_UTF8, 0, a_sString.c_str(), -1, pTemp, nLength, nullptr, nullptr);
	string sString = pTemp;
	delete[] pTemp;
	return sString;
}
#elif (SDW_COMPILER == SDW_COMPILER_GNUC && SDW_COMPILER_VERSION < 50400) || SDW_PLATFORM == SDW_PLATFORM_CYGWIN
string WToU8(const wstring& a_sString)
{
	return TSToS<wstring, string>(a_sString, "WCHAR_T", "UTF-8");
}
#else
string WToU8(const wstring& a_sString)
{
	static wstring_convert<codecvt_utf8<wchar_t>> c_cvt_u8;
	return c_cvt_u8.to_bytes(a_sString);
}
#endif

#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
string WToA(const wstring& a_sString)
{
	int nLength = WideCharToMultiByte(CP_ACP, 0, a_sString.c_str(), -1, nullptr, 0, nullptr, nullptr);
	char* pTemp = new char[nLength];
	WideCharToMultiByte(CP_ACP, 0, a_sString.c_str(), -1, pTemp, nLength, nullptr, nullptr);
	string sString = pTemp;
	delete[] pTemp;
	return sString;
}
#else
string WToA(const wstring& a_sString)
{
	return WToU8(a_sString);
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
