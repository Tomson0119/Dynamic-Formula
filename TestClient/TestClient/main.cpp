#include "Client.h"
#include <random>

const std::string SERVER_IP = "127.0.0.1";

const int MAX_TRIAL = 2;
const int MAX_NET_THREADS = 1;

std::array<std::unique_ptr<Client>, MAX_TRIAL> gClients;
bool gLoop = true;
IOCP gIOCP;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution id_gen{ 0, MAX_TRIAL - 1 };
std::uniform_int_distribution type_gen{ 1, 3 };

void ProcessPacket(WSAOVERLAPPEDEX* over, int id, int bytes)
{
	while (over->NetBuffer.Readable())
	{
		std::byte* packet = over->NetBuffer.BufReadPtr();
		char type = static_cast<char>(packet[1]);

		switch (type)
		{
		case SC::LOGIN_RESULT:
		{
			SC::packet_login_result* pck = reinterpret_cast<SC::packet_login_result*>(packet);
			if (pck->result == (char)LOGIN_STAT::ACCEPTED)
				std::cout << "[" << id << "] Login succeeded\n";
			else
				std::cout << "[" << id << "] Login failed (reason: " << (int)pck->result << ")\n";
			break;
		}
		case SC::ENTER_ROOM_RESULT:
		{
			SC::packet_enter_room_result* pck = reinterpret_cast<SC::packet_enter_room_result*>(packet);
			if (pck->result == (char)ROOM_STAT::ACCEPTED)
				std::cout << "[" << id << "] Entered the room (count: " << pck->num_players << "\n";
			else
				std::cout << "[" << id << "] Room request failed (reason: " << (int)pck->result << ")\n";
			break;
		}
		default:
			std::cout << "Invalid packet\n";
			break;
		}
	}
}

void ThreadFunc()
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
		{
			if (info.bytes != over_ex->WSABuffer.len)
				gClients[client_id]->Disconnect();
			delete over_ex;
			break;
		}
		case OP::RECV:
		{
			if (info.bytes == 0)
			{
				gClients[client_id]->Disconnect();
				break;
			}
			Client* client = gClients[client_id].get();
			over_ex->NetBuffer.ShiftWritePtr(info.bytes);
			ProcessPacket(over_ex, client_id, info.bytes);
			break;
		}
		default:
			std::cout << "[" << client_id << "] " << "Invalid operation\n";
			break;
		}
	}
}

void ClientFunc(int thread_id)
{
	while (true)
	{
		
		
		std::string temp;
		std::cin >> temp;
		std::this_thread::sleep_for(1s);
	}
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
			gClients[i]->RequestLogin();
			gClients[i]->Recv();
		}
	}

	std::vector<std::thread> iocp_threads;
	std::vector<std::thread> client_threads;
	for (int i = 0; i < MAX_NET_THREADS; i++)
	{
		iocp_threads.push_back(std::thread{ ThreadFunc });
	}
	for (int i = 0; i < MAX_TRIAL; i++)
	{
		client_threads.push_back(std::thread{ ClientFunc, i });
	}

	for (std::thread& thrd : client_threads)
		thrd.join();
	for (std::thread& thrd : iocp_threads)
		thrd.join();

}