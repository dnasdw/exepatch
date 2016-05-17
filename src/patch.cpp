#include "patch.h"
#include <openssl/sha.h>

const u32 CPatch::s_uSignature = CONVERT_ENDIAN('3PS\0');
const u8 CPatch::s_uCurrentVersionMajor = 1;
const u8 CPatch::s_uCurrentVersionMinor = 0;
const u8 CPatch::s_uCurrentVersionPatchLevel = 0;

CPatch::CPatch()
	: m_pFileName(nullptr)
	, m_pPatchFileName(nullptr)
	, m_fpOld(nullptr)
	, m_fpPatch(nullptr)
	, m_uVersion(0)
{
	memset(&m_3dsPatchSystemHeader, 0, sizeof(m_3dsPatchSystemHeader));
}

CPatch::~CPatch()
{
}

void CPatch::SetFileName(const char* a_pFileName)
{
	m_pFileName = a_pFileName;
}

void CPatch::SetPatchFileName(const char* a_pPatchFileName)
{
	m_pPatchFileName = a_pPatchFileName;
}

bool CPatch::ApplyPatchFile()
{
	m_fpOld = FFopen(m_pFileName, "rb+");
	if (m_fpOld == nullptr)
	{
		return false;
	}
	m_fpPatch = FFopen(m_pPatchFileName, "rb");
	if (m_fpPatch == nullptr)
	{
		fclose(m_fpOld);
		return false;
	}
	FFseek(m_fpPatch, -8, SEEK_END);
	n64 n3psOffset = 0;
	fread(&n3psOffset, 8, 1, m_fpPatch);
	FFseek(m_fpPatch, -n3psOffset, SEEK_END);
	fread(&m_3dsPatchSystemHeader, sizeof(m_3dsPatchSystemHeader), 1, m_fpPatch);
	if (m_3dsPatchSystemHeader.Signature != s_uSignature)
	{
		printf("ERROR: not support patch file %s\n\n", m_pPatchFileName);
		fclose(m_fpPatch);
		fclose(m_fpOld);
		return false;
	}
	calculateVersion();
	if (m_uVersion > 0x010000)
	{
		printf("ERROR: not support patch file version %" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n\n", m_3dsPatchSystemHeader.VersionMajor, m_3dsPatchSystemHeader.VersionMinor, m_3dsPatchSystemHeader.VersionPatchLevel);
		fclose(m_fpPatch);
		fclose(m_fpOld);
		return false;
	}
	bool bResult = false;
	bool bPatched = false;
	u8 uPatchCommand = 0;
	fread(&uPatchCommand, 1, 1, m_fpPatch);
	if (uPatchCommand == kPatchCommandCheck)
	{
		bPatched = true;
	}
	while (uPatchCommand == kPatchCommandCheck)
	{
		n64 nOffset = 0;
		n64 nSize = 0;
		u8 uSHA256New[32] = {};
		fread(&nOffset, 8, 1, m_fpPatch);
		fread(&nSize, 8, 1, m_fpPatch);
		fread(uSHA256New, 1, 32, m_fpPatch);
		FFseek(m_fpOld, nOffset, SEEK_SET);
		u8* pData = new u8[static_cast<size_t>(nSize)];
		fread(pData, 1, static_cast<size_t>(nSize), m_fpOld);
		u8 uSHA256Old[32] = {};
		SHA256(pData, static_cast<size_t>(nSize), uSHA256Old);
		if (memcmp(uSHA256Old, uSHA256New, 32) != 0)
		{
			bPatched = false;
			break;
		}
		fread(&uPatchCommand, 1, 1, m_fpPatch);
	}
	if (bPatched)
	{
		printf("ERROR: %s was already patched\n\n", m_pFileName);
		fclose(m_fpPatch);
		fclose(m_fpOld);
		return true;
	}
	FFseek(m_fpPatch, -n3psOffset, SEEK_END);
	FFseek(m_fpPatch, sizeof(m_3dsPatchSystemHeader), SEEK_CUR);
	do
	{
		fread(&uPatchCommand, 1, 1, m_fpPatch);
		if (uPatchCommand == kPatchCommandOver)
		{
			bResult = true;
			break;
		}
		else if (uPatchCommand == kPatchCommandCheck)
		{
			FFseek(m_fpPatch, 48, SEEK_CUR);
		}
		else if (uPatchCommand == kPatchCommandMove)
		{
			n64 nFromOffset = 0;
			n64 nToOffset = 0;
			n64 nSize = 0;
			fread(&nFromOffset, 8, 1, m_fpPatch);
			fread(&nToOffset, 8, 1, m_fpPatch);
			fread(&nSize, 8, 1, m_fpPatch);
			executeMove(nFromOffset, nToOffset, nSize);
		}
		else if (uPatchCommand == kPatchCommandSet)
		{
			n64 nStartOffset = 0;
			n64 nSize = 0;
			u8 uData = 0;
			fread(&nStartOffset, 8, 1, m_fpPatch);
			fread(&nSize, 8, 1, m_fpPatch);
			fread(&uData, 1, 1, m_fpPatch);
			executeSet(nStartOffset, nSize, uData);
		}
		else if (uPatchCommand == kPatchCommandChangeSize)
		{
			n64 nSize = 0;
			fread(&nSize, 8, 1, m_fpPatch);
			executeChangeSize(nSize);
		}
		else if (uPatchCommand >= kPatchCommandSeekWrite && uPatchCommand <= kPatchCommandSeekWrite + 0xF)
		{
			bool bSeekSet = (uPatchCommand & 8) == 0;
			n64 nOffset = 0;
			size_t nSize = 0;
			const size_t nBufferSize = 0x10000;
			static u8 uBuffer[nBufferSize] = {};
			size_t nOffsetByte = 1 << (uPatchCommand >> 1 & 3);
			size_t nSizeByte = 1 << (uPatchCommand & 1);
			fread(&nOffset, nOffsetByte, 1, m_fpPatch);
			fread(&nSize, nSizeByte, 1, m_fpPatch);
			nSize++;
			fread(uBuffer, 1, nSize, m_fpPatch);
			executeSeekWrite(bSeekSet, nOffset, nSize, uBuffer);
		}
		else
		{
			printf("ERROR: unknown patch command %02" PRIX8 "\n\n", uPatchCommand);
			break;
		}
	} while (true);
	fclose(m_fpPatch);
	fclose(m_fpOld);
	return bResult;
}

void CPatch::calculateVersion()
{
	m_uVersion = m_3dsPatchSystemHeader.VersionMajor << 16 | m_3dsPatchSystemHeader.VersionMinor << 8 | m_3dsPatchSystemHeader.VersionPatchLevel;
}

void CPatch::executeMove(n64 a_nFromOffset, n64 a_nToOffset, n64 a_nSize)
{
	if (a_nFromOffset != a_nToOffset)
	{
		const n64 nBufferSize = 0x100000;
		u8* pBuffer = new u8[nBufferSize];
		int nIndex = 0;
		if (a_nFromOffset > a_nToOffset)
		{
			while (a_nSize > 0)
			{
				n64 nSize = a_nSize > nBufferSize ? nBufferSize : a_nSize;
				FFseek(m_fpOld, a_nFromOffset + nIndex * nBufferSize, SEEK_SET);
				fread(pBuffer, 1, static_cast<size_t>(nSize), m_fpOld);
				FFseek(m_fpOld, a_nToOffset + nIndex * nBufferSize, SEEK_SET);
				fwrite(pBuffer, 1, static_cast<size_t>(nSize), m_fpOld);
				a_nSize -= nSize;
				nIndex++;
			}
		}
		else
		{
			while (a_nSize > 0)
			{
				n64 nSize = a_nSize > nBufferSize ? nBufferSize : a_nSize;
				a_nSize -= nSize;
				FFseek(m_fpOld, a_nFromOffset + a_nSize, SEEK_SET);
				fread(pBuffer, 1, static_cast<size_t>(nSize), m_fpOld);
				FSeek(m_fpOld, a_nToOffset + a_nSize);
				fwrite(pBuffer, 1, static_cast<size_t>(nSize), m_fpOld);
			}
		}
		delete[] pBuffer;
	}
}

void CPatch::executeSet(n64 a_nStartOffset, n64 a_nSize, u8 a_uData)
{
	FFseek(m_fpOld, a_nStartOffset, SEEK_SET);
	FPadFile(m_fpOld, a_nSize, a_uData);
}

void CPatch::executeChangeSize(n64 a_nSize)
{
	FChsize(FFileno(m_fpOld), a_nSize);
}

void CPatch::executeSeekWrite(bool a_bSeekSet, n64 a_nOffset, size_t a_nSize, u8* a_pData)
{
	FFseek(m_fpOld, a_nOffset, a_bSeekSet ? SEEK_SET : SEEK_CUR);
	fwrite(a_pData, 1, a_nSize, m_fpOld);
}
