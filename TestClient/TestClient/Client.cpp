#include "Client.h"

Client::Client(int id)
	: m_sendOverlapped{ nullptr }, ID(id), RoomID(-1),
	  LoginSuccessFlag(false), RecvResultFlag(false),
	  RoomEnteredFlag(false), GameStartFlag(false)
{
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

void Client::EnterLoginScreen()
{
	LoginSuccessFlag = false;

	std::cout << "[Login Screen].\n";
	while (LoginSuccessFlag.load(std::memory_order_acquire) == false)
	{
		// TEST
		std::string user_id = "GM";
		std::string user_pwd = "GM";
		RequestLogin(user_id, user_pwd);

		/*int c;
		std::cout << "1. Login.\n";
		std::cout << "2. Register.\n";
		std::cout << " : ";
		std::cin >> c;
		std::cin.clear();

		std::string user_id, user_pwd;
		std::cout << "ID: ";
		std::cin >> user_id;
		std::cout << "PWD: ";
		std::cin >> user_pwd;
		std::cin.clear();

		switch (c)
		{
		case 1:
			RequestLogin(user_id, user_pwd);
			break;

		case 2:
			RequestRegister(user_id, user_pwd);
			break;

		default:
			continue;
		}*/

		while (RecvResultFlag.load(std::memory_order_acquire) == false);
		bool b = true;
		RecvResultFlag.compare_exchange_strong(b, false, std::memory_order_release);
	}
	std::cout << "\n";
	PushScene(SCENE::LOBBY);
}

void Client::EnterLobbyScreen()
{
	RoomEnteredFlag = false;

	std::cout << "[Lobby Screen]\n";
	while (RoomEnteredFlag == false)
	{
		int choice = 0;

		std::cout << "1. Request New Room.\n";
		std::cout << "2. Request Enter Room.\n";
		std::cout << " : ";
		std::cin >> choice;
		std::cin.clear();		

		switch (choice)
		{
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
		case 3:
			break;

		default:
			continue;
		}

		while (RecvResultFlag.load(std::memory_order_acquire) == false);
		bool b = true;
		RecvResultFlag.compare_exchange_strong(b, false, std::memory_order_release);
	}
	PushScene(SCENE::ROOM);
}

void Client::EnterWaitRoomScreen()
{
	GameStartFlag = false;

	bool b = true;
	std::cout << "[Wait Room Screen]\n";
	while (GameStartFlag == false)
	{
		int choice = -1;
		std::cout << "0. Back to Lobby\n";
		std::cout << "1. Switch to next map(Only Admin).\n";
		std::cout << "2. Click Ready(Start) button.\n";
		std::cout << " : ";
		std::cin >> choice;
		std::cin.clear();
		
		switch (choice)
		{
		case 0:
			BackToLobby();
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

		while (RecvResultFlag.load(std::memory_order_acquire) == false);
		bool b = true;
		RecvResultFlag.compare_exchange_strong(b, false, std::memory_order_release);
	}
	PushScene(SCENE::IN_GAME);
}

void Client::EnterInGameScreen()
{
	std::cout << "[In Game Screen]\n";
	while (true)
	{
		int choice = -1;
		std::cout << "0. Back to Room.\n";
		
		switch (choice)
		{
		case 0:
			PopScene();
			break;
		}
		while (RecvResultFlag.load(std::memory_order_acquire) == false);
		bool b = true;
		RecvResultFlag.compare_exchange_strong(b, false, std::memory_order_release);
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

void Client::BackToLobby()
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
