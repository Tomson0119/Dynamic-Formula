#include "Client.h"

Client::Client(int id)
	: m_sendOverlapped{ nullptr }, ID(id), RoomID(-1)
{
	LoginSuccessFlag = false;
	RecvResultFlag = false;
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
	std::cout << "[Login Screen].\n";
	while (LoginSuccessFlag.load(std::memory_order_acquire) == false)
	{
		int c;
		std::cout << "1. Login.\n";
		std::cout << "2. Register.\n";
		std::cout << " : ";
		std::cin >> c;
		std::cin.clear();

		switch (c)
		{
		case 1:
			RequestLogin();
			break;

		case 2:
			RequestRegister();
			break;

		default:
			continue;
		}

		while (RecvResultFlag.load(std::memory_order_acquire) == false);
		bool b = true;
		RecvResultFlag.compare_exchange_strong(b, false, std::memory_order_release);
	}
	std::cout << "\n";
}

void Client::EnterLobbyScreen()
{
	while (true)
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
			break;
		}
		case 3:
			break;

		default:
			break;
		}

		while (RecvResultFlag.load(std::memory_order_acquire) == false);
		bool b = true;
		RecvResultFlag.compare_exchange_strong(b, false, std::memory_order_release);
	}
}

void Client::RequestLogin()
{
	std::string client_id, client_pwd;
	std::cout << "ID: ";
	std::cin >> client_id;
	std::cout << "PWD: ";
	std::cin >> client_pwd;
	std::cin.clear();

	std::cout << "[" << ID << "] Requesting login.\n";
	CS::packet_login pck{};
	pck.size = sizeof(CS::packet_login);
	pck.type = CS::LOGIN;	
	strncpy_s(pck.name, client_id.c_str(), MAX_NAME_SIZE-1);
	strncpy_s(pck.pwd, client_pwd.c_str(), MAX_PWD_SIZE-1);
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::Send();
}

void Client::RequestRegister()
{
	std::string client_id, client_pwd;
	std::cout << "ID: ";
	std::cin >> client_id;
	std::cout << "PWD: ";
	std::cin >> client_pwd;
	std::cin.clear();

	std::cout << "[" << ID << "] Requesting register.\n";
	CS::packet_register pck{};
	pck.size = sizeof(CS::packet_register);
	pck.type = CS::REGISTER;
	strncpy_s(pck.name, client_id.c_str(), MAX_NAME_SIZE - 1);
	strncpy_s(pck.pwd, client_pwd.c_str(), MAX_PWD_SIZE - 1);
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