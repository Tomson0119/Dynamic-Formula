#include "Client.h"
#include <random>

const std::string SERVER_IP = "127.0.0.1";

const int MAX_TRIAL = 1;
const int MAX_NET_THREADS = 1;

std::array<std::unique_ptr<Client>, MAX_TRIAL> gClients;
bool gLoop = true;
IOCP gIOCP;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution id_gen{ 0, MAX_TRIAL - 1 };
std::uniform_int_distribution type_gen{ 1, 3 };

struct Room
{
	int player_count;
};

std::atomic_int gRoom_count;

void ProcessPacket(WSAOVERLAPPEDEX* over, int id, int bytes)
{
	while (over->NetBuffer.Readable())
	{
		std::byte* packet = over->NetBuffer.BufReadPtr();
		char type = GetPacketType(packet);

		switch (type)
		{
		case SC::LOGIN_RESULT:
		{
			SC::packet_login_result* pck = reinterpret_cast<SC::packet_login_result*>(packet);
			if (pck->result == (char)LOGIN_STAT::ACCEPTED) {
				std::cout << "[" << id << "] Login succeeded.\n";
				gClients[id]->LoginSuccessFlag.store(true, std::memory_order_release);
			}
			else {
				std::cout << "[" << id << "] Login failed (reason: " << (int)pck->result << ")\n";
			}			
			break;
		}
		case SC::REGISTER_RESULT:
		{
			SC::packet_register_result* pck = reinterpret_cast<SC::packet_register_result*>(packet);
			if (pck->result == (char)REGI_STAT::ACCEPTED)
				std::cout << "[" << id << "] Register succeeded.\n";
			else
				std::cout << "[" << id << "] Register failed (reason: " << (int)pck->result << ")\n";
			break;
		}
		case SC::ACCESS_ROOM_ACCEPT:
		{
			SC::packet_access_room_accept* pck = reinterpret_cast<SC::packet_access_room_accept*>(packet);
			gRoom_count.fetch_add(1);
			gClients[id]->RoomID = pck->room_id;
			
			std::cout << "[" << id << "] Room entered.\n";
			for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
			{
				if (pck->player_stats[i].empty == false) {
					std::cout << "   1. player: " << pck->player_stats[i].name << "\n";
					std::cout << "      color: " << (int)pck->player_stats[i].color << "\n";
					std::cout << "      ready: " << std::boolalpha<<pck->player_stats[i].ready << "\n";
				}
			}
			std::cout << "\n";

			break;
		}
		case SC::ACCESS_ROOM_DENY:
		{
			SC::packet_access_room_deny* pck = reinterpret_cast<SC::packet_access_room_deny*>(packet);
			std::cout << "[" << id << "] Room access denied (reason: " << pck->reason << "\n";
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
			client->RecvResultFlag.store(true, std::memory_order_release);
			gClients[client_id]->Recv();
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
	gClients[thread_id]->EnterLoginScreen();
	gClients[thread_id]->EnterLobbyScreen();
	gClients[thread_id]->EnterWaitRoomScreen();
	gClients[thread_id]->EnterInGameScreen();	
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