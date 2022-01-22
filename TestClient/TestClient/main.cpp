#include "Client.h"
#include <random>
#include <conio.h>

const std::string SERVER_IP = "127.0.0.1";

const int MAX_TRIAL = 1;
const int MAX_NET_THREADS = 1;

std::array<std::unique_ptr<Client>, MAX_TRIAL> gClients;
std::atomic_bool gLoop = true;
IOCP gIOCP;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution id_gen{ 0, MAX_TRIAL - 1 };
std::uniform_int_distribution type_gen{ 1, 3 };

void HandleLoginResultPacket(SC::packet_login_result* pck, int id)
{
	if (pck->result == (char)LOGIN_STAT::ACCEPTED)
	{
		std::cout << "[" << id << "] Login succeeded.\n";
		gClients[id]->PushScene(SCENE::LOBBY);
	}
	else 
	{
		std::cout << "[" << id << "] Login failed (reason: ";
		switch (pck->result)
		{
		case (char)LOGIN_STAT::INVALID_IDPWD:
			std::cout << "Invaild ID or Password.)\n";
			break;
		}
	}
}

void HandleRegisterResultPacket(SC::packet_register_result* pck, int id)
{
	if (pck->result == (char)REGI_STAT::ACCEPTED)
		std::cout << "[" << id << "] Register succeeded.\n";
	else 
	{
		std::cout << "[" << id << "] Register failed (reason: ";
		switch (pck->result)
		{
		case (char)REGI_STAT::INVALID_IDPWD:
			std::cout << "Invalid ID or Password.)\n";
			break;

		case (char)REGI_STAT::ALREADY_EXIST:
			std::cout << "ID is already exists.)\n";
			break;
		}
	}
}

void HandleWaitPlayersInfo(SC::packet_wait_players_info* pck, int id)
{
	if (gClients[id]->RoomID < 0) 
	{
		gClients[id]->RoomID = pck->room_id;
		gClients[id]->PushScene(SCENE::ROOM);
	}

	std::cout << "[" << id << "] Room entered (ID: " << pck->room_id << ").\n";
	for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
	{
		if (pck->player_stats[i].empty == false)
		{
			std::cout << "   " << i + 1
				<< ".player: " << pck->player_stats[i].name << "\n";
			std::cout << "      color: " << (int)pck->player_stats[i].color << "\n";
			std::cout << "      ready: " << std::boolalpha << pck->player_stats[i].ready << "\n";
		}
	}
	std::cout << "\n";
}

void HandleAccessRoomDenyPacket(SC::packet_access_room_deny* pck, int id)
{
	std::cout << "[" << id << "] Room access denied (reason: ";
	switch (pck->reason)
	{
	case (char)ROOM_STAT::ROOM_IS_FULL:
		std::cout << "Room is full.)\n";
		break;

	case (char)ROOM_STAT::GAME_STARTED:
		std::cout << "Game already started.)\n";
		break;

	case (char)ROOM_STAT::MAX_ROOM_REACHED:
		std::cout << "Max room has reached.)\n";
		break;

	case (char)ROOM_STAT::INVALID_ROOM_ID:
		std::cout << "Invalid room id.)\n";
		break;
	}
}

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
			HandleLoginResultPacket(pck, id);
			break;
		}
		case SC::REGISTER_RESULT:
		{
			SC::packet_register_result* pck = reinterpret_cast<SC::packet_register_result*>(packet);
			HandleRegisterResultPacket(pck, id);
			break;
		}
		case SC::WAIT_PLAYERS_INFO:
		{
			SC::packet_wait_players_info* pck = reinterpret_cast<SC::packet_wait_players_info*>(packet);
			HandleWaitPlayersInfo(pck, id);
			break;
		}
		case SC::ACCESS_ROOM_DENY:
		{
			SC::packet_access_room_deny* pck = reinterpret_cast<SC::packet_access_room_deny*>(packet);
			HandleAccessRoomDenyPacket(pck, id);
			break;
		}
		case SC::ROOM_UPDATE_INFO:
		{
			SC::packet_room_update_info* pck = reinterpret_cast<SC::packet_room_update_info*>(packet);

			if (pck->room_closed)
				gClients[id]->EraseRoom(pck->room_id);
			else
				gClients[id]->InsertRoom(pck);
			break;
		}
		case SC::FORCE_LOGOUT:
		{
			while (gClients[id]->GetCurrentScene() == SCENE::LOGIN)
				gClients[id]->PopScene();
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
	while (gLoop.load(std::memory_order_relaxed))
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
	gClients[thread_id]->PushScene(SCENE::LOGIN);

	while (gClients[thread_id]->SceneEmpty() == false)
	{
		system("cls");
		switch (gClients[thread_id]->GetCurrentScene())
		{
		case SCENE::LOGIN:
			gClients[thread_id]->ShowLoginScreen();
			break;

		case SCENE::LOBBY:
			gClients[thread_id]->ShowLobbyScreen();
			break;

		case SCENE::ROOM:
			gClients[thread_id]->ShowWaitRoomScreen();
			break;
			
		case SCENE::IN_GAME:
			gClients[thread_id]->ShowInGameScreen();
			break;
		}
	}
	gLoop.store(false, std::memory_order_relaxed);
}

int main()
{

	for (int i = 0; i < MAX_TRIAL; i++)
	{
		gClients[i] = std::make_unique<Client>(i);
		if (gClients[i]->Connect(SERVER_IP, SERVER_PORT) == false)
		{
			std::cout << "Connection failed [" << i << "]\n";
			return -1;
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