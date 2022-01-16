#include "stdafx.h"
#include "BufferQueue.h"

BufferQueue::BufferQueue()
	: m_writeIndex(0),
	  m_readIndex(0),
	  m_remainSize(MaxBufferSize)
{
	std::memset(m_buffer, 0, MaxBufferSize);
}

BufferQueue::~BufferQueue()
{
}

void BufferQueue::Clear()
{
	m_writeIndex = 0;
	m_readIndex = 0;
	m_remainSize = MaxBufferSize;
}

void BufferQueue::Push(std::byte* msg, int size)
{
	if (msg != nullptr && size > 0)
	{
		if (m_remainSize < size) {
			std::cout << "[Buffer Overflow] Unable to push data.\n";
			return;
		}
		std::memcpy(m_buffer + m_writeIndex, msg, size);
		m_writeIndex += size;
		m_remainSize -= size;
	}
}

void BufferQueue::ShiftWritePtr(int offset)
{
	if (GetLeftBufLen() < offset) {
		std::cout << "[Buffer Overflow] Unable to shift.\n";
		return;
	}

	if (GetLeftBufLen() == offset) {
		std::memcpy(m_buffer, m_buffer + m_readIndex, GetFilledBufLen() + offset);
		m_readIndex = 0;
		m_writeIndex = GetFilledBufLen();
	}
	m_writeIndex += offset;
	m_remainSize -= offset;
}

bool BufferQueue::Readable()
{
	return (Empty() == false && PeekNextPacketSize() <= GetFilledBufLen());
}

char BufferQueue::PeekNextPacketSize()
{
	return *reinterpret_cast<char*>(m_buffer + m_readIndex);
}

std::byte* BufferQueue::BufReadPtr()
{
	if (Empty()) {
		std::cout << "[Buffer Is Empty]\n";
		return nullptr;
	}

	int size = (int)PeekNextPacketSize();
	if (size <= GetFilledBufLen())
	{
		std::byte* ptr = reinterpret_cast<std::byte*>(m_buffer + m_readIndex);
		m_readIndex += size;
		m_remainSize += size;

		return ptr;
	}
	return nullptr;
}
