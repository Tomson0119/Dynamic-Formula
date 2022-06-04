#pragma once

class EndPoint
{
public:
	EndPoint() = default;
	EndPoint(const std::string& ip, short port);
	EndPoint(const sockaddr_in& addr);
	~EndPoint();

	std::string GetIPAddress() const;
	short GetPortNumber() const;

	static std::string GetIPAddress(const sockaddr_in& sckaddr);
	static short GetPortNumber(const sockaddr_in& sckaddr);

	static EndPoint Any(short port);

public:
	static const short PORT_ANY = 0;
	sockaddr_in mAddress;
};