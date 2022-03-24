#include "EndPoint.h"
#include "stdafx.h"
#include "EndPoint.h"

EndPoint::EndPoint(const std::string& ip, short port)
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

EndPoint EndPoint::Any(short port)
{
	EndPoint ep;
	ep.mAddress.sin_family = AF_INET;
	ep.mAddress.sin_port = htons(port);
	ep.mAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	return ep;
}
