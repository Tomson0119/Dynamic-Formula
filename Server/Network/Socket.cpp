#include "stdafx.h"
#include "Socket.h"
#include "EndPoint.h"
#include "WSAOverlappedEx.h"

#include <iostream>

WSAInit gWSAInstance;

Socket::Socket()
	: mSckHandle{}
{
	if (!gWSAInstance.Init())
		throw NetException("WSAData Initialize failed");
}

Socket::Socket(SOCKET sck)
	: mSckHandle{ sck }
{
}

Socket::~Socket()
{
	Close();
}

void Socket::Close()
{
	if (mSckHandle)
		closesocket(mSckHandle);
}

void Socket::SetNagleOption(char val)
{
	setsockopt(mSckHandle, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

void Socket::Init(const SocketType& type)
{
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
	if (accept_ex == nullptr) return;

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

int Socket::Send(WSAOVERLAPPEDEX* overlapped)
{
	DWORD bytes = 0;

	if (WSASend(mSckHandle,
		&overlapped->WSABuffer, 1, &bytes, 0,
		&overlapped->Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

int Socket::Recv(WSAOVERLAPPEDEX* overlapped)
{
	DWORD flag = 0;
	DWORD bytes = 0;

	if (WSARecv(mSckHandle,
		&overlapped->WSABuffer, 1, &bytes, &flag,
		&overlapped->Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

int Socket::SendTo(WSAOVERLAPPEDEX* overlapped, const EndPoint& hostEp)
{
	DWORD bytes = 0;

	if (WSASendTo(mSckHandle,
		&overlapped->WSABuffer, 1, &bytes, 0,
		reinterpret_cast<const sockaddr*>(&hostEp.mAddress), sizeof(hostEp.mAddress),
		&overlapped->Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

int Socket::RecvFrom(WSAOVERLAPPEDEX* overlapped, EndPoint& hostEp)
{
	DWORD flag = 0;
	DWORD bytes = 0;

	int ip_len = sizeof(hostEp.mAddress);

	if (WSARecvFrom(mSckHandle,
		&overlapped->WSABuffer, 1, &bytes, &flag,
		reinterpret_cast<sockaddr*>(&hostEp.mAddress), &ip_len,
		&overlapped->Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}