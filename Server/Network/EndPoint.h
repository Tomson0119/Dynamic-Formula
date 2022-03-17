#pragma once

class EndPoint
{
public:
	EndPoint() = default;
	EndPoint(const std::string& ip, short port);
	EndPoint(const sockaddr_in& addr);
	~EndPoint();

	static EndPoint Any(short port);

	sockaddr_in mAddress;
};