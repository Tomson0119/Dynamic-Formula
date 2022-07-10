#include "common.h"
#include "UDPReceiver.h"

constexpr bool operator==(const EndPoint& a, const EndPoint& b)
{
	return ((a.GetIPAddress() == b.GetIPAddress()) 
		&& (a.GetPortNumber() == b.GetPortNumber()));
}

size_t EpHash::operator()(const EndPoint& ep) const
{
	size_t key = 0;
	std::string ip = ep.GetIPAddress();
	for (char c : ip)
	{
		key = ((key >> 5) + c);
	}
	key += ep.GetPortNumber();
	key += (key >> 16);
	key ^= (key << 11);
	key += (key >> 6);
	return key;
}


UDPReceiver::UDPReceiver()
	: mSenderEp{ }
{
	mUDPSocket = std::make_unique<Socket>();
	mUDPSocket->Init(SocketType::UDP);
	mUDPRecvOverlapped.Reset(OP::RECV);
}

void UDPReceiver::Bind(const EndPoint& ep)
{
	mUDPSocket->Bind(ep);
}

void UDPReceiver::AssignId(const EndPoint& ep, int id)
{
	std::unique_lock<std::mutex> lock{ mHostIdMapMut };
	mHostIdMap.insert({ ep, id });
}

void UDPReceiver::RecvMsg()
{
	mUDPRecvOverlapped.NetBuffer.Clear();
	mUDPRecvOverlapped.Reset(OP::RECV);
	mUDPSocket->RecvFrom(mUDPRecvOverlapped, mSenderEp);
}

void UDPReceiver::PrintLastReceivedEp()
{
	std::cout << "UDP Sender Endpoint: ("
		<< mSenderEp.GetIPAddress() << ", "
		<< mSenderEp.GetPortNumber() << ")\n";
}

std::optional<int> UDPReceiver::GetLastReceivedId()
{
	std::unique_lock<std::mutex> lock{ mHostIdMapMut };
	if (mHostIdMap.find(mSenderEp) != mHostIdMap.end())
	{
		return mHostIdMap[mSenderEp];
	}
	return std::nullopt;
}