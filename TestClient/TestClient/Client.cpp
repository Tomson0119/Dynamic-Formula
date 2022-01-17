#include "Client.h"

Client::Client(int id)
	: m_sendOverlapped{ nullptr }, ID(id)
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

void Client::RequestLogin()
{
	std::cout << "[" << ID << "] Requesting login.\n";
	CS::packet_login pck{};
	pck.size = sizeof(CS::packet_login);
	pck.type = CS::LOGIN;
	std::string client_id = "Host" + std::to_string(ID);
	std::string client_pwd = "Host" + std::to_string(ID);
	strncpy_s(pck.name, client_id.c_str(), client_id.size());
	strncpy_s(pck.pwd, client_pwd.c_str(), client_pwd.size());
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void Client::RequestNewRoom()
{
	std::cout << "[" << ID << "] Requesting new room.\n";
	CS::packet_enter_room pck{};
	pck.size = sizeof(CS::packet_enter_room);
	pck.type = CS::ENTER_ROOM;
	pck.new_room = true;
	pck.room_id = -1;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void Client::RequestEnterRoom(int room_id)
{
	std::cout << "[" << ID << "] Requesting enter room (" << room_id << ").\n";
	CS::packet_enter_room pck{};
	pck.size = sizeof(CS::packet_enter_room);
	pck.type = CS::ENTER_ROOM;
	pck.new_room = false;
	pck.room_id = room_id;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
}
