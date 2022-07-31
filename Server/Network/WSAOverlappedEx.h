#pragma once

#include "stdafx.h"
#include "Socket.h"
#include "Protocol.h"
#include "BufferQueue.h"
#include "MemoryPoolManager.h"

enum class OP : char
{
	RECV,
	SEND,
	ACCEPT,
	PHYSICS
};

// TODO: WSAOVERLAPPEDEX�� ũ�Ⱑ �ʹ� ũ�� ������
// BufferQueue�� �и��ϴ� ���� �ٶ����ϴ�.
// Push �Լ��� multiple threads���� ������ �� �ֱ� ������
// Thread-safe�ϵ��� �����ϴ� ���� �ʿ��ϴ�.
//		1. Send thread ���
//		2. Push �� concurrent_queue ���.
//		3. Ȥ�� mutex lock.
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
		else 
		{
			WSABuffer.buf = reinterpret_cast<char*>(NetBuffer.BufWritePtr());
			WSABuffer.len = (ULONG)NetBuffer.GetLeftBufLen();
		}
	}

	void PushMsg(std::byte* data, int bytes)
	{
		if (Operation == OP::SEND)
		{
			// Multi thread���� Push�� ������ �� �����Ƿ� lock.
			std::unique_lock<std::mutex> lock{ PushMut };

			NetBuffer.Push(data, bytes);
			WSABuffer.len = (ULONG)NetBuffer.GetFilledBufLen();
		}
	}

	void* operator new(size_t size)
	{
		if (MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().GetPoolSize() > 0)
		{
			auto p = MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().Allocate();
			if (p) return p;
		}
		return ::operator new(size);
	}

	void operator delete(void* ptr)
	{
		if (MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().GetPoolSize() > 0)
			MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().Deallocate(ptr);
		else
			::operator delete(ptr);
	}

private:
	std::mutex PushMut;
};