#pragma once

#include "common.h"

class Client
{
public:
	Client();
	~Client();

	bool Connect(const std::string& addr, short port);

public:
	bool Loop = true;

private:
	Socket m_socket;
};
