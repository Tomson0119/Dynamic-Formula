#pragma once

#include "Protocol.h"

class BufferQueue
{
public:
	BufferQueue();
	~BufferQueue();

	void Clear();

	// For Send buffer
	void Push(std::byte* msg, int size);

	// For Recv buffer
	void ShiftWritePtr(int offset);
	
	int GetLeftBufLen() const { return (MaxBufferSize - m_writeIndex); }
	int GetFilledBufLen() const { return (MaxBufferSize - m_remainSize); }
	
	bool Readable();
	bool Empty() const { return (m_readIndex == m_writeIndex && m_remainSize > 0); }
	
	char PeekNextPacketSize();

	std::byte* BufStartPtr() { return m_buffer; }
	std::byte* BufWritePtr() { return m_buffer + m_writeIndex; }
	std::byte* BufReadPtr();

private:
	int m_readIndex;
	int m_writeIndex;
	int m_remainSize;
	
	std::byte m_buffer[MaxBufferSize];
};