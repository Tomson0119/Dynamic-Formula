#include "Client.h"

const std::string SERVER_IP = "127.0.0.1";

const int MAX_TRIAL = 10;
const int MAX_THREADS = 1;

std::array<std::unique_ptr<Client>, MAX_TRIAL> gClients;
std::atomic_bool gLoop = true;
IOCP gIOCP;

void ThreadFunc(int id)
{
	CompletionInfo info{};
	while (gLoop)
	{
		gIOCP.GetCompletionInfo(info);

		int client_id = static_cast<int>(info.key);
		WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);
		
		if (info.success == FALSE)
		{
			gClients[client_id]->Disconnect();
			if (over_ex->Operation == OP::SEND)
				delete over_ex;
			continue;
		}

		switch (over_ex->Operation)
		{
		case OP::SEND:
			if (info.bytes != over_ex->WSABuffer.len)
				gClients[client_id]->Disconnect();
			delete over_ex;
			break;

		case OP::RECV:

			break;

		default:
			std::cout << "[" << client_id << "] " << "Invalid operation\n";
			break;
		}
	}
}

int main()
{
	for (int i = 0; i < MAX_TRIAL; i++)
	{
		gClients[i] = std::make_unique<Client>();
		if (gClients[i]->Connect(SERVER_IP, SERVER_PORT) == false)
		{
			std::cout << "Connection failed [" << i << "]\n";
		}
		else
		{
			gIOCP.RegisterDevice(gClients[i]->GetSocket(), i);
		}
	}

	std::vector<std::thread> net_threads;
	for (int i = 0; i < MAX_THREADS; i++)
	{
		net_threads.push_back(std::thread{ ThreadFunc, i });
	}

	while (true)
	{
		std::string input;
		std::getline(std::cin, input);

		CS::chat_packet chat_packet{};
		chat_packet.size = sizeof(CS::chat_packet);
		chat_packet.type = CS::CHAT;
		strncpy_s(chat_packet.message, input.c_str(), input.size());
		gClients[0]->Send(reinterpret_cast<std::byte*>(&chat_packet), sizeof(CS::chat_packet));

		//std::this_thread::sleep_for(10ms);
	}
}