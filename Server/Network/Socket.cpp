#include "stdafx.h"
#include "Socket.h"
#include "EndPoint.h"
#include "WSAOverlappedEx.h"

#include <iostream>

WSAInit gWSAInstance;

Socket::Socket()
	: mSckHandle{},
	  mSckType{},
	  mRecvOverlapped{ OP::RECV },
	  mSendOverlapped{ nullptr }
{
	if (!gWSAInstance.Init())
		throw NetException("WSAData Initialize failed");
}

Socket::Socket(SOCKET sck)
	: mSckHandle{ sck },
	  mSckType{},
	  mRecvOverlapped{ OP::RECV },
	  mSendOverlapped{ nullptr }
{
}

Socket::~Socket()
{
	if (mSendOverlapped)
	{
		delete mSendOverlapped;
		mSendOverlapped = nullptr;
	}
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

void Socket::PushPacket(std::byte* pck, int bytes)
{
	if (mSendOverlapped == nullptr)
	{
		mSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
	}
	else
	{
		mSendOverlapped->PushMsg(pck, bytes);
	}
}

int Socket::Send()
{
	if (mSckType != SocketType::TCP || mSendOverlapped == nullptr)
		return -1;

	DWORD bytes = 0;

	if (WSASend(mSckHandle,
		&mSendOverlapped->WSABuffer, 1, &bytes, 0,
		&mSendOverlapped->Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			delete mSendOverlapped;
			mSendOverlapped = nullptr;
			return -1;
		}
	}
	mSendOverlapped = nullptr;
	return (int)bytes;
}

int Socket::Recv()
{
	if (mSckType != SocketType::TCP)
		return -1;

	mRecvOverlapped.Reset(OP::RECV);

	DWORD flag = 0;
	DWORD bytes = 0;

	if (WSARecv(mSckHandle,
		&mRecvOverlapped.WSABuffer, 1, &bytes, &flag,
		&mRecvOverlapped.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}

int Socket::SendTo(const EndPoint& hostEp)
{
	if (mSckType != SocketType::UDP || mSendOverlapped == nullptr)
		return -1;

	DWORD bytes = 0;

	if (WSASendTo(mSckHandle,
		&mSendOverlapped->WSABuffer, 1, &bytes, 0,
		reinterpret_cast<const sockaddr*>(&hostEp.mAddress), sizeof(hostEp.mAddress),
		&mSendOverlapped->Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			delete mSendOverlapped;
			mSendOverlapped = nullptr;
			return -1;
		}
	}
	mSendOverlapped = nullptr;
	return (int)bytes;
}

int Socket::RecvFrom(EndPoint& hostEp)
{
	if (mSckType != SocketType::UDP)
		return -1;

	mRecvOverlapped.NetBuffer.Clear();
	mRecvOverlapped.Reset(OP::RECV);

	DWORD flag = 0;
	DWORD bytes = 0;

	int ip_len = sizeof(hostEp.mAddress);

	if (WSARecvFrom(mSckHandle,
		&mRecvOverlapped.WSABuffer, 1, &bytes, &flag,
		reinterpret_cast<sockaddr*>(&hostEp.mAddress), &ip_len,
		&mRecvOverlapped.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return (int)bytes;
}