#pragma once

#include "common.h"

class Client
{
public:
	Client(int id);
	~Client();

	bool Connect(const std::string& addr, short port);
	void Disconnect();

	void PushPacket(std::byte* pck, int bytes);

	void Send();
	void Send(std::byte* msg, int bytes);

	void Recv();

	SOCKET GetSocket() const { return m_socket.GetSocket(); }

public:
	void RequestLogin();
	void RequestNewRoom();
	void RequestEnterRoom(int room_id);

public:
	int ID;

private:
	Socket m_socket;

	WSAOVERLAPPEDEX* m_sendOverlapped;
	WSAOVERLAPPEDEX m_recvOverlapped;
};
