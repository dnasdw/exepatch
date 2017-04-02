#ifndef PATCH_H_
#define PATCH_H_

#include "utility.h"

class CPatch
{
public:
	enum EPatchCommand
	{
		kPatchCommandOver,
		kPatchCommandCheck,
		kPatchCommandMove,
		kPatchCommandSet,
		kPatchCommandChangeSize,
		kPatchCommandSeekWrite = 0x10
	};
	struct S3dsPatchSystemHeader
	{
		u32 Signature;
		u8 VersionMajor;
		u8 VersionMinor;
		u8 VersionPatchLevel;
		u8 Reserved;
		n64 ExtDataOffset;
	};
	CPatch();
	~CPatch();
	void SetFileName(const string& a_sFileName);
	void SetPatchFileName(const UString& a_sPatchFileName);
	bool ApplyPatchFile();
	static const u32 s_uSignature;
	static const u8 s_uCurrentVersionMajor;
	static const u8 s_uCurrentVersionMinor;
	static const u8 s_uCurrentVersionPatchLevel;
private:
	void calculateVersion();
	void executeMove(n64 a_nFromOffset, n64 a_nToOffset, n64 a_nSize);
	void executeSet(n64 a_nStartOffset, n64 a_nSize, u8 a_uData);
	void executeChangeSize(n64 a_nSize);
	void executeSeekWrite(bool a_bSeekSet, n64 a_nOffset, size_t a_nSize, u8* a_pData);
	string m_sFileName;
	UString m_sPatchFileName;
	FILE* m_fpOld;
	FILE* m_fpPatch;
	S3dsPatchSystemHeader m_3dsPatchSystemHeader;
	u32 m_uVersion;
};

#endif	// PATCH_H_
