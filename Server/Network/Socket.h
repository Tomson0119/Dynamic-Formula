#pragma once

#include "WSAInit.h"
#include "WSAOverlappedEx.h"

extern WSAInit gWSAInstance;

class EndPoint;

class Socket
{
public:
	Socket();
	Socket(SOCKET sck);
	virtual ~Socket();

	void Close();
	void SetNagleOption(char val);

	void Init();
	void Bind(const EndPoint& ep);
	void Listen();
	
	void AsyncAccept(WSAOVERLAPPEDEX* accpet_ex);
	bool Connect(const EndPoint& ep);

	int Send(WSAOVERLAPPEDEX* overlapped);
	int Recv(WSAOVERLAPPEDEX* overlapped);

	void SetSocket(SOCKET sck) { mSckHandle = sck; }
	SOCKET GetSocket() const { return mSckHandle; }

private:
	SOCKET mSckHandle;
};