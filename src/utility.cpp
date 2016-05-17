#include "utility.h"

const string& FGetModuleFile()
{
	const int nMaxPath = 4096;
	static string sFile;
	sFile.clear();
	sFile.resize(nMaxPath, '\0');
	size_t uSize = 0;
#if EXEPATCH_COMPILER == COMPILER_MSC
	uSize = GetModuleFileNameA(nullptr, &sFile.front(), nMaxPath);
#elif defined(EXEPATCH_APPLE)
	char path[nMaxPath] = {};
	u32 uPathSize = static_cast<u32>(sizeof(path));
	if (_NSGetExecutablePath(path, &uPathSize) != 0)
	{
		printf("ERROR: _NSGetExecutablePath error\n\n");
		sFile.erase();
	}
	else if (realpath(path, &sFile.front()) == nullptr)
	{
		sFile.erase();
	}
	uSize = strlen(sFile.c_str());
#else
	ssize_t nCount = readlink("/proc/self/exe", &sFile.front(), nMaxPath);
	if (nCount == -1)
	{
		printf("ERROR: readlink /proc/self/exe error\n\n");
		sFile.erase();
	}
	else
	{
		sFile[nCount] = '\0';
	}
	uSize = strlen(sFile.c_str());
#endif
	sFile.erase(uSize);
	replace(sFile.begin(), sFile.end(), '\\', '/');
	return sFile;
}

void FPadFile(FILE* a_fpFile, n64 a_nPadSize, u8 a_uPadData)
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

FILE* FFopenA(const char* a_pFileName, const char* a_pMode)
{
	FILE* fp = fopen(a_pFileName, a_pMode);
	if (fp == nullptr)
	{
		printf("ERROR: open file %s failed\n\n", a_pFileName);
	}
	return fp;
}

bool FSeek(FILE* a_fpFile, n64 a_nOffset)
{
	if (fflush(a_fpFile) != 0)
	{
		return false;
	}
	int nFd = FFileno(a_fpFile);
	if (nFd == -1)
	{
		return false;
	}
	FFseek(a_fpFile, 0, SEEK_END);
	n64 nFileSize = FFtell(a_fpFile);
	if (nFileSize < a_nOffset)
	{
		n64 nOffset = FLseek(nFd, a_nOffset - 1, SEEK_SET);
		if (nOffset == -1)
		{
			return false;
		}
		fputc(0, a_fpFile);
		fflush(a_fpFile);
	}
	else
	{
		FFseek(a_fpFile, a_nOffset, SEEK_SET);
	}
	return true;
}
