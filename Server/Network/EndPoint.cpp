#include "stdafx.h"
#include "EndPoint.h"

EndPoint::EndPoint(const std::string& ip, u_short port)
{
	mAddress.sin_family = AF_INET;
	mAddress.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &mAddress.sin_addr);
}

EndPoint::EndPoint(const sockaddr_in& addr)
{
	mAddress = addr;
}

EndPoint::~EndPoint()
{
}

std::string EndPoint::GetIPAddress() const
{
	return EndPoint::GetIPAddress(mAddress);
}

u_short EndPoint::GetPortNumber() const
{
	return EndPoint::GetPortNumber(mAddress);
}

std::string EndPoint::GetIPAddress(const sockaddr_in& sckaddr)
{
	const int MAX_IPV4_LEN = 256;
	char ip[MAX_IPV4_LEN];
	auto p = inet_ntop(AF_INET, &sckaddr.sin_addr, ip, MAX_IPV4_LEN);

	if (p == nullptr) return "Wrong Socket Name";
	return std::string(ip);
}

u_short EndPoint::GetPortNumber(const sockaddr_in& sckaddr)
{
	return ntohs(sckaddr.sin_port);
}

EndPoint EndPoint::Any(u_short port)
{
	EndPoint ep;
	ep.mAddress.sin_family = AF_INET;
	ep.mAddress.sin_port = htons(port);
	ep.mAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	return ep;
}
