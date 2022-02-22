#include "stdafx.h"
#include "gameFramework.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int mCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(820);
#endif
	try
	{
		GameFramework app;
		app.SetResolution(1920, 1080);

		if (!app.InitFramework())
			return 0;		
		app.Run();
	}
	catch (DxException& error)
	{
		MessageBox(nullptr, error.ToString().c_str(), L"HR ERROR", MB_OK);
		return 0;
	}
}