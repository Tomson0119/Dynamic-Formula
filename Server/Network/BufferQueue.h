#pragma once

#include "Protocol.h"

// NOTE: BufferQueue is not thread-safe.
// Should be careful to use it especially Push method.

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
	
	int GetLeftBufLen() const { return (MaxBufferSize - mWriteIndex); }
	int GetFilledBufLen() const { return (MaxBufferSize - mRemainSize); }
	
	bool Readable();
	bool Empty() const { return (mReadIndex == mWriteIndex && mRemainSize > 0); }
	
	uint16_t PeekNextPacketSize();

	std::byte* BufStartPtr() { return mBuffer; }
	std::byte* BufWritePtr() { return mBuffer + mWriteIndex; }
	std::byte* BufReadPtr();

private:
	int mReadIndex;
	int mWriteIndex;
	int mRemainSize;
	
	std::byte mBuffer[MaxBufferSize];
};