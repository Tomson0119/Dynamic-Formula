#include "Client.h"

Client::Client(int id)
	: m_sendOverlapped{ nullptr }, ID(id), RoomID(-1)
{
	LoginResult = "";
	EnterRoomResult = "";
	
	ClearPlayerList();

	m_socket.Init();
}

Client::~Client()
{
}

bool Client::Connect(const std::string& addr, short port)
{
	return m_socket.Connect(EndPoint(addr, port));
}

void Client::Disconnect()
{
	m_socket.Close();
}

void Client::PushPacket(std::byte* pck, int bytes)
{
	if (m_sendOverlapped == nullptr)
		m_sendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
	else
	{
		m_sendOverlapped->PushMsg(pck, bytes);
	}
}

void Client::Send()
{
	if (m_sendOverlapped != nullptr) {
		m_socket.Send(m_sendOverlapped);
		m_sendOverlapped = nullptr;
	}
}

void Client::Send(std::byte* msg, int bytes)
{
	PushPacket(msg, bytes);
	Send();	
}

void Client::Recv()
{
	m_recvOverlapped.Reset(OP::RECV);
	m_socket.Recv(&m_recvOverlapped);
}

void Client::PushScene(SCENE newScene)
{
	std::lock_guard<std::mutex> lck(mSceneStackLock);
	mSceneStack.push(newScene);
}

void Client::PopScene()
{
	std::lock_guard<std::mutex> lck(mSceneStackLock);
	mSceneStack.pop();
}

SCENE Client::GetCurrentScene()
{
	std::lock_guard<std::mutex> lck(mSceneStackLock);
	return mSceneStack.top();
}

void Client::PrintRoomList()
{
	int k = 0;
	for (const auto& [_, room] : mRoomList)
	{
		std::cout << "--------------- " << (k++) + 1 << " ---------------\n";
		std::cout << "Room ID: " << room.ID << "\n";
		std::cout << "Player Counts: " << (int)room.PlayerCount << "\n";
		std::cout << "Map: " << (int)room.MapID << "\n";
		std::cout << "Game started: " << std::boolalpha << room.GameStarted << "\n";
	}
	std::cout << "---------------------------------\n";
}

void Client::ClearRoomList()
{
	mRoomList.clear();
}

void Client::InsertRoom(SC::packet_room_update_info* packet)
{
	std::lock_guard<std::mutex> lck(mRoomListLock);
	Room newRoom{};
	newRoom.ID = packet->room_id;
	newRoom.MapID = packet->map_id;
	newRoom.PlayerCount = packet->player_count;
	newRoom.GameStarted = packet->game_started;
	newRoom.Closed = packet->room_closed;
	mRoomList.insert({ newRoom.ID, newRoom });

	system("cls");
	PrintLobbyInterface();
}

void Client::UpdateWaitPlayersInfo(SC::packet_wait_players_info* info)
{
	if (RoomID == info->room_id)
	{
		mMapIdx = info->map_id;
		for (int i = 0; i < MAX_ROOM_CAPACITY; i++)
		{
			mPlayerList[i].Name = info->player_stats[i].name;
			mPlayerList[i].Color = info->player_stats[i].color;
			mPlayerList[i].Empty = info->player_stats[i].empty;
			mPlayerList[i].Ready = info->player_stats[i].ready;
		}
	}

	system("cls");
	PrintWaitRoomInterface();
}

void Client::EraseRoom(int room_id)
{
	std::lock_guard<std::mutex> lck(mRoomListLock);
	mRoomList.erase(room_id);

	system("cls");
	PrintLobbyInterface();
}

void Client::PrintWaitPlayers()
{
	int i = 1;
	for (const PlayerInfo& info : mPlayerList)
	{
		if (info.Empty == false) {
			std::cout << "---------- " << i << " ----------\n";
			std::cout << "Name: " << info.Name << "\n";
			std::cout << "Color: " << (int)info.Color << "\n";
			std::cout << "Ready: " << std::boolalpha << info.Ready << "\n";
		}
	}
	std::cout << "-----------------------\n";
}

void Client::ClearPlayerList()
{
	for (PlayerInfo& info : mPlayerList)
		info.Empty = true;
}

void Client::ShowLoginScreen()
{
	std::cout << LoginResult;
	std::cout << "[Login Screen]\n";
	std::cout << "1. Login.\n";
	std::cout << "2. Register.\n";
	std::cout << " : ";

	int choice = -1;
	std::cin >> choice;
	std::cin.clear();

	std::string user_id, user_pwd;
	std::cout << "ID: ";
	std::cin >> user_id;
	std::cout << "PWD: ";
	std::cin >> user_pwd;

	if (GetCurrentScene() != SCENE::LOGIN)
		return;

	switch (choice)
	{
	case 1:
		RequestLogin(user_id, user_pwd);
		break;

	case 2:
		RequestRegister(user_id, user_pwd);
		break;
	}
}

void Client::PrintLobbyInterface()
{
	std::cout << "[Lobby Screen]\n";
	PrintRoomList();
	std::cout << "0. Back to Loggin scene.\n";
	std::cout << "1. Request New Room.\n";
	std::cout << "2. Request Enter Room.\n";
	std::cout << " : ";
}

void Client::ShowLobbyScreen()
{
	std::cout << LoginResult << EnterRoomResult;
	PrintLobbyInterface();

	int choice = -1;
	std::cin >> choice;
	std::cin.clear();

	if (GetCurrentScene() != SCENE::LOBBY)
		return;

	switch (choice)
	{
	case 0:
		ClearRoomList();
		RevertScene();
		PopScene();
		return;
			
	case 1:
		RequestNewRoom();
		break;

	case 2:
	{
		int n;
		std::cout << "Room number: ";
		std::cin >> n;
		std::cin.clear();

		RequestEnterRoom(n);
		break;
	}
	}
}

void Client::PrintWaitRoomInterface()
{
	std::cout << "[Wait Room Screen]  (ID: " << RoomID << ")\n";
	PrintWaitPlayers();
	std::cout << "0. Back to Lobby\n";
	std::cout << "1. Switch to next map(Only Admin).\n";
	std::cout << "2. Click Ready(Start) button.\n";
	std::cout << " : ";
}

void Client::ShowWaitRoomScreen()
{
	std::cout << EnterRoomResult;
	//PrintWaitRoomInterface();
	
	int choice = -1;
	std::cin >> choice;
	std::cin.clear();

	if (GetCurrentScene() != SCENE::ROOM)
		return;

	switch (choice)
	{
	case 0:
		ClearRoomList();
		ClearPlayerList();
		RevertScene();
		PopScene();
		return;

	case 1:
		if (Admin()) SwitchMap();
		break;

	case 2:
		SetOrUnsetReady();
		break;

	default:
		break;
	}
}

void Client::ShowInGameScreen()
{
	std::cout << "[In Game Screen]\n";
	std::cout << "0. Back to Room.\n";
	std::cout << " : ";
	
	int choice = -1;
	std::cin >> choice;
	std::cin.clear();

	if (GetCurrentScene() != SCENE::IN_GAME)
		return;

	switch (choice)
	{
	case 0:
		RevertScene();
		PopScene();
		return;
	}
}

void Client::RequestLogin(const std::string& name, const std::string& pwd)
{
	std::cout << "[" << ID << "] Requesting login.\n";
	CS::packet_login pck{};
	pck.size = sizeof(CS::packet_login);
	pck.type = CS::LOGIN;	
	strncpy_s(pck.name, name.c_str(), MAX_NAME_SIZE-1);
	strncpy_s(pck.pwd, pwd.c_str(), MAX_PWD_SIZE-1);
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::Send();
}

void Client::RequestRegister(const std::string& name, const std::string& pwd)
{
	std::cout << "[" << ID << "] Requesting register.\n";
	CS::packet_register pck{};
	pck.size = sizeof(CS::packet_register);
	pck.type = CS::REGISTER;
	strncpy_s(pck.name, name.c_str(), MAX_NAME_SIZE - 1);
	strncpy_s(pck.pwd, pwd.c_str(), MAX_PWD_SIZE - 1);
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::Send();
}

void Client::RequestNewRoom()
{
	std::cout << "[" << ID << "] Requesting new room.\n";
	CS::packet_open_room pck{};
	pck.size = sizeof(CS::packet_open_room);
	pck.type = CS::OPEN_ROOM;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::Send();
}

void Client::RequestEnterRoom(int room_id)
{
	std::cout << "[" << ID << "] Requesting enter room (" << room_id << ").\n";
	CS::packet_enter_room pck{};
	pck.size = sizeof(CS::packet_enter_room);
	pck.type = CS::ENTER_ROOM;
	pck.room_id = room_id;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::Send();
}

void Client::RevertScene()
{
	CS::packet_revert_scene pck{};
	pck.size = sizeof(CS::packet_revert_scene);
	pck.type = CS::REVERT_SCENE;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::Send();
}

void Client::SwitchMap()
{
}

void Client::SetOrUnsetReady()
{
}
