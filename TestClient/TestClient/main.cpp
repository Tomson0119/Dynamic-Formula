#include "Client.h"

const std::string SERVER_IP = "127.0.0.1";

const int MAX_TRIAL = 100;
std::array<std::unique_ptr<Client>, MAX_TRIAL> gClients;

int main()
{
	for (int i = 0; i < MAX_TRIAL; i++)
	{
		gClients[i] = std::make_unique<Client>();
		if (gClients[i]->Connect(SERVER_IP, SERVER_PORT) == false)
		{
			std::cout << "Connection failed [" << i << "]\n";
		}
	}

	while (true) std::this_thread::sleep_for(100ms);
}