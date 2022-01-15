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
		: Operation(op), WSABuffer{}
	{
		Reset(op);
	}

	WSAOVERLAPPEDEX(OP op, std::byte* data, int bytes)
		: Operation(op), WSABuffer{}
	{
		Reset(op, data, bytes);
	}

	void Reset(OP op)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		MsgQueue.Clear();
		WSABuffer.buf = reinterpret_cast<char*>(MsgQueue.GetBuffer());
		WSABuffer.len = MaxBufferSize;
	}

	void Reset(OP op, std::byte* data, int bytes)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		MsgQueue.Clear();
		MsgQueue.Push(data, bytes);
		WSABuffer.buf = reinterpret_cast<char*>(MsgQueue.GetBuffer());
		WSABuffer.len = (ULONG)bytes;
	}

	void PushMsg(std::byte* data, int bytes)
	{
		if (Operation == OP::SEND)
		{
			MsgQueue.Push(data, bytes);
			WSABuffer.len = (ULONG)MsgQueue.GetTotalMsgSize();
		}
	}
};