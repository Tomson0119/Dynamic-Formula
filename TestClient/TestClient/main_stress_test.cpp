#include "Client.h"
#include <random>

const std::string SERVER_IP = "127.0.0.1";

const int MAX_TRIAL = 10000;
const int MAX_NET_THREADS = 4;
const uint64_t PING_LIMIT = 1000;

HANDLE hHandle = GetStdHandle(STD_OUTPUT_HANDLE);

std::array<std::unique_ptr<Client>, MAX_TRIAL> gClients;
std::array<std::chrono::system_clock::time_point, MAX_TRIAL> gPings;

std::atomic_bool gLoop = true;
IOCP gIOCP;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution type_gen{ 0, 2 };
std::uniform_int_distribution room_gen{ 0, MAX_ROOM_SIZE-1 };

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
				//std::cout << "[" << id << "] Login accepted.\n";
				gClients[id]->PushScene(SCENE::LOBBY);
				gClients[id]->LoginRequestFlag = false;
			}
			break;
		}
		case SC::REGISTER_RESULT:
		{
			SC::packet_register_result* pck = reinterpret_cast<SC::packet_register_result*>(packet);
			if (pck->result == (char)REGI_STAT::ACCEPTED)
				std::cout << "[" << id << "] Register accepted.\n";
			else if (pck->result == (char)REGI_STAT::INVALID_IDPWD)
				std::cout << "[" << id << "] Reigster failed: Invalid id and password.\n";
			else
				std::cout << "[" << id << "] Register failed: ID already exists.\n";
			break;
		}
		case SC::ROOM_INSIDE_INFO:
		{
			SC::packet_room_inside_info* pck = reinterpret_cast<SC::packet_room_inside_info*>(packet);
			gClients[id]->UpdateWaitRoomInfo(pck);
			break;
		}
		case SC::ACCESS_ROOM_ACCEPT:
		{
			SC::packet_access_room_accept* pck = reinterpret_cast<SC::packet_access_room_accept*>(packet);
			gClients[id]->CalculateAccessRoomPing(pck->send_time);
			gClients[id]->EnterRoomResult = "Room entered(ID: " + std::to_string(pck->room_id) + ").\n";
			gClients[id]->RoomID = pck->room_id;
			gClients[id]->PushScene(SCENE::ROOM);
			break;
		}
		case SC::ACCESS_ROOM_DENY:
		{
			SC::packet_access_room_deny* pck = reinterpret_cast<SC::packet_access_room_deny*>(packet);
			gClients[id]->CalculateAccessRoomPing(pck->send_time);
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
				if (gClients[id]->PlayerIdx == pck->player_idx)
					gClients[id]->CalculateReadyPing(pck->send_time);
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
				if (gClients[id]->PlayerIdx == gClients[id]->AdminIdx)
					gClients[id]->CalculateReadyPing(pck->send_time);
				if (pck->succeeded)
					gClients[id]->PushScene(SCENE::IN_GAME);
			}
			break;
		}
		case SC::FORCE_LOGOUT:
		{
			while (gClients[id]->GetCurrentScene() != SCENE::LOGIN)
				gClients[id]->PopScene();
			break;
		}
		default:
			std::cout << "\n\nInvalid packet\n";
			gClients[id]->Disconnect();
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
			gClients[client_id]->Disconnect();
			break;
		}
	}
}

bool ConnectToServer(int id)
{
	if (gClients[id]->Connect(SERVER_IP, SERVER_PORT))
	{
		gIOCP.RegisterDevice(gClients[id]->GetSocket(), id);
		gClients[id]->Recv();
		return true;
	}
	else {
		std::cout << "[" << id << "] Failed to connect to server.\n";
		return false;
	}
}

bool DoLobbyTask(int id)
{
	int choice = type_gen(gen);
	switch (choice)
	{
	case 0:
		gClients[id]->ClearRoomList();
		gClients[id]->RevertScene();
		gClients[id]->PopScene();
		break;

	case 1:
		gClients[id]->RequestNewRoom();
		break;

	case 2:
	{
		int room_count = gClients[id]->GetRoomCount();
		if (room_count <= 0) break;

		int idx = room_gen(gen) % room_count;
		int room_id = gClients[id]->GetRoomIdByIndex(idx);
		if (room_id >= 0) {
			gClients[id]->RequestEnterRoom(room_id);
		}
		break;
	}
	default:
		return false;
	}
	return true;
}

bool DoRoomTask(int id)
{
	int choice = type_gen(gen);
	switch (choice)
	{
	case 0:
		gClients[id]->ClearRoomList();
		gClients[id]->ClearPlayerList();
		gClients[id]->RevertScene();
		gClients[id]->PopScene();
		break;

	case 1:
		gClients[id]->SwitchMap();
		break;

	case 2:
		gClients[id]->SetOrUnsetReady();
		break;

	default:
		return false;
	}
	return true;
}

bool DoInGameTask(int id)
{
	return true;
}

bool DoRandomTask(int id)
{
	SCENE currentScene = gClients[id]->GetCurrentScene();
	switch (currentScene)
	{
	case SCENE::LOGIN:
	{
		std::string name = "test" + std::to_string(id);
		std::string pwd = "testpwd" + std::to_string(id);
		gClients[id]->RequestLogin(name, pwd);
		return true;
	}
	case SCENE::LOBBY:
	{
		return DoLobbyTask(id);
	}
	case SCENE::ROOM:
	{
		return DoRoomTask(id);
	}
	case SCENE::IN_GAME:
	{
		return DoInGameTask(id);
	}
	}
	return false;
}

void PrintEveryClientInfo()
{
	int lobbyCount = 0;
	for (int i = 0; i < MAX_TRIAL; i++)
	{
		SCENE scene = gClients[i]->GetCurrentScene();
		std::cout << "[" << i << "] ping: ";
		std::cout << gClients[i]->MaxPing << " , scene: ";
		switch (scene)
		{
		case SCENE::LOGIN:
			std::cout << "LOGIN                                  \n";
			break;

		case SCENE::LOBBY:
			std::cout << "LOBBY,  room count: " << gClients[i]->GetRoomCount() << "                                   \n";
			lobbyCount++;
			break;

		case SCENE::ROOM:
			std::cout << "ROOM  , room id: " << gClients[i]->RoomID << "                                 \n";
			break;

		case SCENE::IN_GAME:
			std::cout << "INGAME                                 \n";
			break;
		}
	}
	std::cout << "Lobby players count: " << lobbyCount << "                \n";
}

uint64_t MaxClientsPing()
{
	uint64_t max_value = 0;
	for (int i = 0; i < MAX_TRIAL; i++)
	{
		if (max_value < gClients[i]->MaxPing)
			max_value = gClients[i]->MaxPing;
	}
	return max_value;
}

void ClientFunc()
{
	int connections = 0;
	int i = 0;
	while (gLoop)
	{
		SetConsoleCursorPosition(hHandle, { 0,0 });
		std::cout << "connections: " << connections << "                        \n";
		//PrintEveryClientInfo();

		if (MaxClientsPing() >= PING_LIMIT)
			goto MaxPing;

		if (connections < MAX_TRIAL)
		{
			if (ConnectToServer(connections++) == false)
				goto ConnectionFail;
		}

		auto beg = std::chrono::system_clock::now();
		for (i = 0; i < connections; i++)
		{
			if (DoRandomTask(i) == false)
				goto TaskFailed;			
		}
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg);
		if (duration < 0ms) duration = 0ms;
		if(duration < 100ms) std::this_thread::sleep_for(100ms - duration);
	}

MaxPing:
	std::cout << "\n\nMax ping has reached..\n";
	return;

ConnectionFail:
	std::cout << "\n\nConnection has failed. (connections: " << connections << "\n";
	return;

TaskFailed:
	std::cout << "\n\nTask failed in client [" << i << "]\n";
	return;
}

int main()
{
	for (int i = 0; i < MAX_TRIAL; i++)
		gClients[i] = std::make_unique<Client>(i);

	std::vector<std::thread> iocp_threads;
	std::thread client_thread{ ClientFunc };
	for (int i = 0; i < MAX_NET_THREADS; i++)
		iocp_threads.push_back(std::thread{ ThreadFunc });

	client_thread.join();
	for (std::thread& thrd : iocp_threads)
		thrd.join();
}