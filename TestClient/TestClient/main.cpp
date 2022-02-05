#include "Client.h"
#include <random>
#include <conio.h>

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
		gClients[id]->LoginResult = "Login succeeded.\n\n";
		gClients[id]->PushScene(SCENE::LOBBY);
	}
	else 
	{
		gClients[id]->LoginResult = "Login failed (reason: ";
		switch (pck->result)
		{
		case (char)LOGIN_STAT::INVALID_IDPWD:
			gClients[id]->LoginResult += "Invaild ID or Password.)\n\n";
			break;
		}
	}
}

void HandleRegisterResultPacket(SC::packet_register_result* pck, int id)
{
	if (pck->result == (char)REGI_STAT::ACCEPTED)
		gClients[id]->LoginResult = "Register succeeded.\n\n";
	else 
	{
		gClients[id]->LoginResult = "Register failed(reason: ";
		switch (pck->result)
		{
		case (char)REGI_STAT::INVALID_IDPWD:
			gClients[id]->LoginResult += "Invalid ID or Password.)\n\n";
			break;

		case (char)REGI_STAT::ALREADY_EXIST:
			gClients[id]->LoginResult += "ID is already exists.)\n\n";
			break;
		}
	}
}

void HandleWaitPlayersInfo(SC::packet_room_inside_info* pck, int id)
{
	if (gClients[id]->RoomID != pck->room_id)
	{
		std::cout << "Invalid room id\n";
		return;
	}
	gClients[id]->PlayerIdx = pck->player_idx;
	gClients[id]->AdminIdx = pck->admin_idx;
	gClients[id]->UpdateWaitRoomInfo(pck);
}

void HandleAccessRoomDenyPacket(SC::packet_access_room_deny* pck, int id)
{
	gClients[id]->EnterRoomResult = "Room access denied(reason: ";
	switch (pck->reason)
	{
	case (char)ROOM_STAT::ROOM_IS_CLOSED:
		gClients[id]->EnterRoomResult += "Room is closed.)\n\n";
		break;

	case (char)ROOM_STAT::ROOM_IS_FULL:
		gClients[id]->EnterRoomResult += "Room is full.)\n\n";
		break;

	case (char)ROOM_STAT::GAME_STARTED:
		gClients[id]->EnterRoomResult += "Game already started.)\n\n";
		break;

	case (char)ROOM_STAT::MAX_ROOM_REACHED:
		gClients[id]->EnterRoomResult += "Max room has reached.)\n\n";
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
		case SC::ROOM_INSIDE_INFO:
		{
			SC::packet_room_inside_info* pck = reinterpret_cast<SC::packet_room_inside_info*>(packet);
			HandleWaitPlayersInfo(pck, id);
			break;
		}
		case SC::ACCESS_ROOM_ACCEPT:
		{
			SC::packet_access_room_accept* pck = reinterpret_cast<SC::packet_access_room_accept*>(packet);
			gClients[id]->EnterRoomResult = "Room entered(ID: " + std::to_string(pck->room_id) + ").\n";
			gClients[id]->RoomID = pck->room_id;
			gClients[id]->PushScene(SCENE::ROOM);			
			break;
		}
		case SC::ACCESS_ROOM_DENY:
		{
			SC::packet_access_room_deny* pck = reinterpret_cast<SC::packet_access_room_deny*>(packet);
			HandleAccessRoomDenyPacket(pck, id);
			break;
		}
		case SC::ROOM_OUTSIDE_INFO:
		{
			SC::packet_room_outside_info* pck = reinterpret_cast<SC::packet_room_outside_info*>(packet);

			if (pck->room_closed)
				gClients[id]->EraseRoom(pck->room_id);
			else
				gClients[id]->InsertRoom(pck);
			break;
		}
		case SC::UPDATE_PLAYER_INFO:
		{
			SC::packet_update_player_info* pck = reinterpret_cast<SC::packet_update_player_info*>(packet);
			if (pck->room_id == gClients[id]->RoomID)
			{
				gClients[id]->AdminIdx = pck->admin_idx;
				gClients[id]->UpdatePlayer(pck->player_idx, pck->player_info);
			}
			break;
		}
		case SC::UPDATE_MAP_INFO:
		{
			SC::packet_update_map_info* pck = reinterpret_cast<SC::packet_update_map_info*>(packet);
			if (gClients[id]->RoomID == pck->room_id)
			{
				gClients[id]->UpdateMap(pck->map_id);
			}
			break;
		}
		case SC::REMOVE_PLAYER:
		{
			SC::packet_remove_player* pck = reinterpret_cast<SC::packet_remove_player*>(packet);
			if (gClients[id]->RoomID == pck->room_id)
			{
				gClients[id]->AdminIdx = pck->admin_idx;
				gClients[id]->RemovePlayer(pck->player_idx);
			}
			break;
		}
		case SC::GAME_START_RESULT:
		{
			SC::packet_game_start_result* pck = reinterpret_cast<SC::packet_game_start_result*>(packet);
			if (gClients[id]->RoomID == pck->room_id)
			{
				if (pck->succeeded) {
					gClients[id]->PushScene(SCENE::IN_GAME);
					std::cout << "Game Started! (Input any value)\n";
				}
				else
					gClients[id]->GameStartResult = "Game Start failed: players are not all ready.\n";
			}
			break;
		}
		case SC::FORCE_LOGOUT:
		{
			while (gClients[id]->GetCurrentScene() != SCENE::LOGIN)
				gClients[id]->PopScene();
			std::cout << "Force Logout!! Returning to login screen..(Input any value)\n";
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
		gClients[thread_id]->LoginResult = "";
		gClients[thread_id]->EnterRoomResult = "";
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