#include "patch.h"
#include <openssl/sha.h>

const u32 CPatch::s_uSignature = SDW_CONVERT_ENDIAN32('3PS\0');
const u8 CPatch::s_uCurrentVersionMajor = 1;
const u8 CPatch::s_uCurrentVersionMinor = 0;
const u8 CPatch::s_uCurrentVersionPatchLevel = 0;

CPatch::CPatch()
	: m_fpOld(nullptr)
	, m_fpPatch(nullptr)
	, m_uVersion(0)
{
	memset(&m_3dsPatchSystemHeader, 0, sizeof(m_3dsPatchSystemHeader));
}

CPatch::~CPatch()
{
}

void CPatch::SetFileName(const UString& a_sFileName)
{
	m_sFileName = a_sFileName;
}

void CPatch::SetPatchFileName(const UString& a_sPatchFileName)
{
	m_sPatchFileName = a_sPatchFileName;
}

bool CPatch::ApplyPatchFile()
{
	m_fpOld = UFopen(m_sFileName.c_str(), USTR("rb+"));
	if (m_fpOld == nullptr)
	{
		return false;
	}
	m_fpPatch = UFopen(m_sPatchFileName.c_str(), USTR("rb"));
	if (m_fpPatch == nullptr)
	{
		fclose(m_fpOld);
		return false;
	}
	Fseek(m_fpPatch, -8, SEEK_END);
	n64 n3psOffset = 0;
	fread(&n3psOffset, 8, 1, m_fpPatch);
	Fseek(m_fpPatch, -n3psOffset, SEEK_END);
	fread(&m_3dsPatchSystemHeader, sizeof(m_3dsPatchSystemHeader), 1, m_fpPatch);
	if (m_3dsPatchSystemHeader.Signature != s_uSignature)
	{
		fclose(m_fpPatch);
		fclose(m_fpOld);
		UPrintf(USTR("ERROR: not support patch file %") PRIUS USTR("\n\n"), m_sPatchFileName.c_str());
		return false;
	}
	calculateVersion();
	if (m_uVersion > 0x010000)
	{
		fclose(m_fpPatch);
		fclose(m_fpOld);
		UPrintf(USTR("ERROR: not support patch file version %") PRIUS USTR(".%") PRIUS USTR(".%") PRIUS USTR("\n\n"), AToU(Format("%" PRIu8, m_3dsPatchSystemHeader.VersionMajor)).c_str(), AToU(Format("%" PRIu8, m_3dsPatchSystemHeader.VersionMinor)).c_str(), AToU(Format("%" PRIu8, m_3dsPatchSystemHeader.VersionPatchLevel)).c_str());
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
		Fseek(m_fpOld, nOffset, SEEK_SET);
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
		fclose(m_fpPatch);
		fclose(m_fpOld);
		UPrintf(USTR("ERROR: %") PRIUS USTR(" was already patched\n\n"), m_sFileName.c_str());
		return true;
	}
	Fseek(m_fpPatch, -n3psOffset, SEEK_END);
	Fseek(m_fpPatch, sizeof(m_3dsPatchSystemHeader), SEEK_CUR);
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
			Fseek(m_fpPatch, 48, SEEK_CUR);
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
			size_t uSize = 0;
			const size_t uBufferSize = 0x10000;
			static u8 uBuffer[uBufferSize] = {};
			size_t uOffsetByte = static_cast<size_t>(SDW_BIT64(uPatchCommand >> 1 & 3));
			size_t uSizeByte = static_cast<size_t>(SDW_BIT64(uPatchCommand & 1));
			fread(&nOffset, uOffsetByte, 1, m_fpPatch);
			fread(&uSize, uSizeByte, 1, m_fpPatch);
			uSize++;
			fread(uBuffer, 1, uSize, m_fpPatch);
			executeSeekWrite(bSeekSet, nOffset, uSize, uBuffer);
		}
		else
		{
			UPrintf(USTR("ERROR: unknown patch command %") PRIUS USTR("\n\n"), AToU(Format("%02" PRIX8, uPatchCommand)).c_str());
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
				Fseek(m_fpOld, a_nFromOffset + nIndex * nBufferSize, SEEK_SET);
				fread(pBuffer, 1, static_cast<size_t>(nSize), m_fpOld);
				Fseek(m_fpOld, a_nToOffset + nIndex * nBufferSize, SEEK_SET);
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
				Fseek(m_fpOld, a_nFromOffset + a_nSize, SEEK_SET);
				fread(pBuffer, 1, static_cast<size_t>(nSize), m_fpOld);
				Seek(m_fpOld, a_nToOffset + a_nSize);
				fwrite(pBuffer, 1, static_cast<size_t>(nSize), m_fpOld);
			}
		}
		delete[] pBuffer;
	}
}

void CPatch::executeSet(n64 a_nStartOffset, n64 a_nSize, u8 a_uData)
{
	Fseek(m_fpOld, a_nStartOffset, SEEK_SET);
	PadFile(m_fpOld, a_nSize, a_uData);
}

void CPatch::executeChangeSize(n64 a_nSize)
{
	Chsize(Fileno(m_fpOld), a_nSize);
}

void CPatch::executeSeekWrite(bool a_bSeekSet, n64 a_nOffset, size_t a_nSize, u8* a_pData)
{
	Fseek(m_fpOld, a_nOffset, a_bSeekSet ? SEEK_SET : SEEK_CUR);
	fwrite(a_pData, 1, a_nSize, m_fpOld);
}
