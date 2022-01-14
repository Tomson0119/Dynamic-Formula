#pragma once

#include "Protocol.h"

class RingBuffer
{
public:
	RingBuffer();
	~RingBuffer();

	void Clear();

	void Push(std::byte* msg, size_t size);
	void Pop(std::byte* msg, size_t size);

	bool IsEmpty();
	bool IsFull();

	char GetMsgType();
	size_t GetTotalMsgSize();
	size_t PeekNextPacketSize();

	std::byte* GetBuffer() { return m_buffer; }

private:
	int m_readIndex;
	int m_writeIndex;
	size_t m_remainSize;

	std::byte m_buffer[MaxBufferSize];
};