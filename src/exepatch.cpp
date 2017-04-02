#include "patch.h"

int main(int argc, char* argv[])
{
	SetLocale();
	if (argc != 2)
	{
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
		MessageBoxW(nullptr, XToW("ʧ��", 936, "CP936").c_str(), L"", MB_OK);
#endif
		return 1;
	}
	CPatch patch;
	patch.SetFileName(argv[1]);
	patch.SetPatchFileName(UGetModuleFileName());
	bool bResult = patch.ApplyPatchFile();
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
	MessageBoxW(nullptr, bResult ? XToW("���", 936, "CP936").c_str() : XToW("ʧ��", 936, "CP936").c_str(), L"", MB_OK);
#endif
	return bResult ? 0 : 1;
}
