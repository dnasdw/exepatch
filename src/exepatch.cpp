#include "patch.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
#if EXEPATCH_COMPILER == COMPILER_MSC
		MessageBoxW(nullptr, L"ʧ��", L"", MB_OK);
#endif
		return 1;
	}
	CPatch patch;
	patch.SetFileName(argv[1]);
	patch.SetPatchFileName(FGetModuleFile().c_str());
	bool bResult = patch.ApplyPatchFile();
#if EXEPATCH_COMPILER == COMPILER_MSC
	MessageBoxW(nullptr, bResult ? L"���" : L"ʧ��", L"", MB_OK);
#endif
	return bResult ? 0 : 1;
}
