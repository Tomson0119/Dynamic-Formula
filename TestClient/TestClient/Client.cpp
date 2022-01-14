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

void Client::Disconnect()
{
	m_socket.Close();
}

void Client::Send(std::byte* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	m_socket.Send(*send_over);
}
