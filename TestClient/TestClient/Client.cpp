#include "Client.h"

Client::Client()
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
