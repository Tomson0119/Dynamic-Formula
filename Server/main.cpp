#include "common.h"
#include "LoginServer.h"
#include <iostream>

int main()
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try {
		LoginServer server(EndPoint::Any(SERVER_PORT));
		server.Run();
		return 0;
	}
	catch (NetException& ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}
}