#include "common.h"
#include "LobbyServer.h"

#include <iostream>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	try {
		LobbyServer server(EndPoint::Any(SERVER_PORT));
		server.Run();
		return 0;
	}
	catch (NetException& ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}
}