#include "stdafx.h"
#include "gameFramework.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int mCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	try
	{
		GameFramework app;
		app.SetResolution(800, 600);

		if (!app.InitFramework())
			return -1;		
		app.Run();
	}
	catch (std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), "ERROR", MB_OK);
		return -1;
	}
	return 0;
}