#pragma once

#include "stdafx.h"
#include "Socket.h"
#include "Protocol.h"
#include "RingBuffer.h"

enum class OP : char
{
	RECV,
	SEND,
	ACCEPT
};

struct WSAOVERLAPPEDEX
{
	WSAOVERLAPPED Overlapped;
	WSABUF WSABuffer;
	OP Operation;

	RingBuffer MsgQueue;

	WSAOVERLAPPEDEX(OP op = OP::RECV)
		: Operation(op), WSABuffer{}, MsgQueue{}
	{
		Reset(op);
	}

	WSAOVERLAPPEDEX(OP op, std::byte* data, int bytes)
		: Operation(op), WSABuffer{}, MsgQueue{}
	{
		Reset(op, data, bytes);
	}

	void Reset(OP op)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		WSABuffer.buf = reinterpret_cast<char*>(MsgQueue.GetBuffer());
		WSABuffer.len = MaxBufferSize;
	}

	void Reset(OP op, std::byte* data, int bytes)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		std::memcpy(MsgQueue.GetBuffer(), data, bytes);
		WSABuffer.buf = reinterpret_cast<char*>(MsgQueue.GetBuffer());
		WSABuffer.len = (ULONG)bytes;
	}
};