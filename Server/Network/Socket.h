#pragma once

#include "WSAInit.h"
#include "WSAOverlappedEx.h"
#include "EndPoint.h"

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

	int Send(WSAOVERLAPPEDEX& over);
	int Recv(WSAOVERLAPPEDEX& over);

	int SendTo(WSAOVERLAPPEDEX& over, const EndPoint& hostEp);
	int RecvFrom(WSAOVERLAPPEDEX& over, EndPoint& hostEp);

	std::string GetIPAddress() const;
	u_short GetPortNumber() const;

	static std::optional<sockaddr_in> GetSocketName(SOCKET handle);

public:
	void SetSocket(SOCKET sck) { mSckHandle = sck; }
	SOCKET GetSocket() const { return mSckHandle; }

private:
	static WSAInit gWSAInstance;

	SOCKET mSckHandle;
	SocketType mSckType;

	std::mutex mSendMut;
};