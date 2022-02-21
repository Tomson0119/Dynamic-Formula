#pragma once

#include "stdafx.h"
#include "Socket.h"
#include "Protocol.h"
#include "BufferQueue.h"

enum class OP : char
{
	RECV,
	SEND,
	ACCEPT,
	DISCONNECT,
	PHYSICS,
	BROADCAST
};

struct WSAOVERLAPPEDEX
{
	WSAOVERLAPPED Overlapped;
	WSABUF WSABuffer;
	OP Operation;
	BufferQueue NetBuffer;

	WSAOVERLAPPEDEX(OP op=OP::RECV)
		: Operation(op), WSABuffer{}
	{
		Reset(op);
	}

	WSAOVERLAPPEDEX(OP op, std::byte* data, int bytes)
		: Operation(op), WSABuffer{}
	{
		Reset(op, data, bytes);
	}

	void Reset(OP op, std::byte* data=nullptr, int bytes=0)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));

		if (bytes > 0 && data)
		{
			NetBuffer.Clear();
			NetBuffer.Push(data, bytes);
			WSABuffer.buf = reinterpret_cast<char*>(NetBuffer.BufStartPtr());
			WSABuffer.len = (ULONG)bytes;
		}
		else {
			WSABuffer.buf = reinterpret_cast<char*>(NetBuffer.BufWritePtr());
			WSABuffer.len = (ULONG)NetBuffer.GetLeftBufLen();
		}
	}

	void PushMsg(std::byte* data, int bytes)
	{
		if (Operation == OP::SEND)
		{
			NetBuffer.Push(data, bytes);
			WSABuffer.len = (ULONG)NetBuffer.GetFilledBufLen();
		}
	}
};