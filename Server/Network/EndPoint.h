#pragma once

class EndPoint
{
public:
	EndPoint() = default;
	EndPoint(const std::string& ip, u_short port);
	EndPoint(const sockaddr_in& addr);
	~EndPoint();

	std::string GetIPAddress() const;
	u_short GetPortNumber() const;

	static std::string GetIPAddress(const sockaddr_in& sckaddr);
	static u_short GetPortNumber(const sockaddr_in& sckaddr);

	static EndPoint Any(u_short port);

public:
	static const short PORT_ANY = 0;
	sockaddr_in mAddress;
};