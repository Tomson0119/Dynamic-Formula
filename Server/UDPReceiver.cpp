#include "common.h"
#include "UDPReceiver.h"

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
	//mHostIdMap.insert({ ep, id });
}

void UDPReceiver::RecvMsg()
{
	mUDPRecvOverlapped.NetBuffer.Clear();
	mUDPRecvOverlapped.Reset(OP::RECV);
	mUDPSocket->RecvFrom(mUDPRecvOverlapped, mSenderEp);
}