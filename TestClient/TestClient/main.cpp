#include "Client.h"
#include <random>

const std::string SERVER_IP = "127.0.0.1";

const int MAX_TRIAL = 10;
const int MAX_THREADS = 1;

std::array<std::unique_ptr<Client>, MAX_TRIAL> gClients;
bool gLoop = true;
IOCP gIOCP;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution id_gen{ 0, MAX_TRIAL - 1 };
std::uniform_int_distribution type_gen{ 1, 3 };

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

void GenLoginPacket(int id)
{
	std::cout << "[" << id << "] Generating login packet ";
	CS::login_packet pck{};
	pck.size = sizeof(CS::login_packet);
	pck.type = CS::LOGIN;
	std::string client_name = "Host" + std::to_string(id);
	strncpy_s(pck.name, client_name.c_str(), client_name.size());
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GenChatPacket(int id)
{
	std::cout << "[" << id << "] Generating chat packet ";
	CS::chat_packet pck{};
	pck.size = sizeof(CS::chat_packet);
	pck.type = CS::CHAT;
	std::string chat = "Hello from Host" + std::to_string(id);
	strncpy_s(pck.message, chat.c_str(), chat.size());
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void GenMovePacket(int id)
{
	std::cout << "[" << id << "] Generating move packet ";
	CS::move_packet pck{};
	pck.size = sizeof(CS::move_packet);
	pck.type = CS::MOVE;
	pck.direction = id;
	auto now = std::chrono::steady_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	pck.move_time = now_ms.time_since_epoch().count();
	gClients[id]->PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
}

int main()
{
	for (int i = 0; i < MAX_TRIAL; i++)
	{
		gClients[i] = std::make_unique<Client>(i);
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
		int id = 1;
		int type = type_gen(gen);

		switch (type)
		{
		case CS::LOGIN:
		{
			GenLoginPacket(id);
			if (id % 2 == 0) break;
		}
		case CS::CHAT:
		{
			GenChatPacket(id);
			if (id % 2 == 0) break;
		}
		case CS::MOVE:
		{
			GenMovePacket(id);
			break;
		}
		default:
			std::cout << "Wrong type genned\n";
			break;
		}
		std::cout << "\n";
		gClients[id]->Send();
		std::this_thread::sleep_for(2.3s);
	}
}