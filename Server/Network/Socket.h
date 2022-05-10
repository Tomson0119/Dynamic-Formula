#pragma once

#include "WSAInit.h"
#include "WSAOverlappedEx.h"
#include "EndPoint.h"

extern WSAInit gWSAInstance;

enum class SocketType : char
{
	TCP=0,
	UDP
};

class Socket
{
public:
	Socket();
	Socket(SOCKET sck);
	virtual ~Socket();

	void Close();
	void SetNagleOption(char val);

	void Init(const SocketType& type);
	void Bind(const EndPoint& ep);
	void Listen();
	
	void AsyncAccept(WSAOVERLAPPEDEX* accpet_ex);
	bool Connect(const EndPoint& ep);

	void PushPacket(std::byte* pck, int bytes);

	int Send();
	int Recv();

	int SendTo(const EndPoint& hostEp);
	int RecvFrom(EndPoint& hostEp);

public:
	void SetSocket(SOCKET sck) { mSckHandle = sck; }
	SOCKET GetSocket() const { return mSckHandle; }

private:
	SOCKET mSckHandle;
	SocketType mSckType;

	WSAOVERLAPPEDEX mRecvOverlapped;
	WSAOVERLAPPEDEX* mSendOverlapped;
};