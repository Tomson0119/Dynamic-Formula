#include "stdafx.h"
#include "Socket.h"
#include "EndPoint.h"
#include "WSAOverlappedEx.h"

#include <iostream>

WSAInit Socket::gWSAInstance;

Socket::Socket()
	: mSckHandle{},
	  mSckType{} 
{
	if (!gWSAInstance.Init())
		throw NetException("WSAData Initialize failed");
}

Socket::Socket(SOCKET sck)
	: mSckHandle{ sck },
	  mSckType{}
{
}

Socket::~Socket()
{
	Close();
}

void Socket::Close()
{
	if (mSckHandle) closesocket(mSckHandle);
}

void Socket::SetNagleOption(char val)
{
	setsockopt(mSckHandle, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

void Socket::Init(const SocketType& type)
{
	mSckType = type;
	if (type == SocketType::TCP) 
	{
		mSckHandle = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	}
	else if (type == SocketType::UDP)
	{
		mSckHandle = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED);
	}
	if (mSckHandle == INVALID_SOCKET)
		throw NetException("Socket creation failed");
}

void Socket::Bind(const EndPoint& ep)
{
	if (bind(mSckHandle, reinterpret_cast<const sockaddr*>(&ep.mAddress), sizeof(ep.mAddress)) != 0)
		throw NetException("Bind failed");
}

void Socket::Listen()
{
	if (listen(mSckHandle, SOMAXCONN) != 0)
		throw NetException("Listen failed");
}

void Socket::AsyncAccept(WSAOVERLAPPEDEX* accept_ex)
{
	if (mSckType != SocketType::TCP || accept_ex == nullptr) return;

	SOCKET sck = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (sck == INVALID_SOCKET)
		throw NetException("Socket is invalid");

	accept_ex->Reset(OP::ACCEPT, reinterpret_cast<std::byte*>(&sck), sizeof(sck));
	
	if (AcceptEx(mSckHandle, sck, accept_ex->NetBuffer.BufStartPtr() + sizeof(SOCKET), 0, sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16, NULL, &accept_ex->Overlapped) == FALSE)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
			throw NetException("AcceptEx failed");
	}
}

bool Socket::Connect(const EndPoint& ep)
{
	if (WSAConnect(mSckHandle,
		reinterpret_cast<const sockaddr*>(&ep.mAddress), 
		sizeof(ep.mAddress), 0, 0, 0, 0) != 0)
	{
		return false;
	}
	return true;
}

int Socket::Send(WSAOVERLAPPEDEX& over)
{
	DWORD bytes = 0;

	// TCP Send는 Thread safe하지 않기 때문에 lock을 건다.
	std::unique_lock<std::mutex> lock{ mSendMut };

	if (WSASend(mSckHandle,
		&over.WSABuffer, 1, &bytes, 0,
		&over.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

int Socket::Recv(WSAOVERLAPPEDEX& over)
{
	DWORD flag = 0;
	DWORD bytes = 0;

	if (WSARecv(mSckHandle,
		&over.WSABuffer, 1, &bytes, &flag,
		&over.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

int Socket::SendTo(WSAOVERLAPPEDEX& over, const EndPoint& hostEp)
{
	DWORD bytes = 0;

	if (WSASendTo(mSckHandle,
		&over.WSABuffer, 1, &bytes, 0,
		reinterpret_cast<const sockaddr*>(&hostEp.mAddress), sizeof(hostEp.mAddress),
		&over.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

int Socket::RecvFrom(WSAOVERLAPPEDEX& over, EndPoint& hostEp)
{
	DWORD flag = 0;
	DWORD bytes = 0;

	int ipLen = sizeof(hostEp.mAddress);

	if (WSARecvFrom(mSckHandle,
		&over.WSABuffer, 1, &bytes, &flag,
		reinterpret_cast<sockaddr*>(&hostEp.mAddress), &ipLen,
		&over.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

std::string Socket::GetIPAddress() const 
{
	auto sckName = Socket::GetSocketName(mSckHandle);
	if (sckName.has_value())
	{
		return EndPoint::GetIPAddress(sckName.value());
	}
	return "Wrong Socket Handle";
}

u_short Socket::GetPortNumber() const
{
	auto sckName = Socket::GetSocketName(mSckHandle);
	if (sckName.has_value())
	{
		return EndPoint::GetPortNumber(sckName.value());
	}
	return -1;
}

std::optional<sockaddr_in> Socket::GetSocketName(SOCKET handle)
{
	sockaddr_in sckAddr;
	int len = sizeof(sckAddr);

	if (getsockname(handle, reinterpret_cast<sockaddr*>(&sckAddr), &len) < 0)
		return std::nullopt;
	
	return sckAddr;
}