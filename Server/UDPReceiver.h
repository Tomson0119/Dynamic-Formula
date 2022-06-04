#pragma once
#include <map>

class UDPReceiver
{
public:
	UDPReceiver();
	virtual ~UDPReceiver() = default;

public:
	void Bind(const EndPoint& ep);
	void AssignId(const EndPoint& ep, int id);
	void RecvMsg();

public:
	Socket* GetSocket() const { return mUDPSocket.get(); }

private:
	//std::map<EndPoint, int> mHostIdMap;

	std::unique_ptr<Socket> mUDPSocket;
	WSAOVERLAPPEDEX mUDPRecvOverlapped;
	EndPoint mSenderEp;
};