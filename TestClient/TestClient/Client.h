#pragma once

#include "common.h"

class Client
{
public:
	Client();
	~Client();

	bool Connect(const std::string& addr, short port);
	void Disconnect();

	void Send(std::byte* msg, int bytes);

	SOCKET GetSocket() const { return m_socket.GetSocket(); }

private:
	Socket m_socket;
};
