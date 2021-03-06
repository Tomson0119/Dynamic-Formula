#pragma once
#include <unordered_map>

struct EpHash
{
	size_t operator()(const EndPoint& ep) const;
};

class UDPReceiver
{
public:
	UDPReceiver();
	virtual ~UDPReceiver() = default;

public:
	void Bind(const EndPoint& ep);
	void AssignId(const EndPoint& ep, int id);
	void RecvMsg();

	void PrintLastReceivedEp();

	std::optional<int> GetLastReceivedId();

public:
	Socket* GetSocket() const { return mUDPSocket.get(); }

private:
	std::mutex mHostIdMapMut;
	std::unordered_map<EndPoint, int, EpHash> mHostIdMap;

	std::unique_ptr<Socket> mUDPSocket;
	WSAOVERLAPPEDEX mUDPRecvOverlapped;
	EndPoint mSenderEp;
};