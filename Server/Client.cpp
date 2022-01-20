#include "common.h"
#include "Client.h"
#include "WSAOverlappedEx.h"


Client::Client(int id)
	: ID(id),
	  mRecvOverlapped{},
	  mSendOverlapped{},
	  mState{ CLIENT_STAT::EMPTY }
{
	mSocket.Init();
}

Client::~Client()
{
}

void Client::Disconnect()
{	
	mState = CLIENT_STAT::EMPTY;
	mSocket.Close();
}

void Client::AssignAcceptedID(int id, SOCKET sck)
{
	ID = id;
	mSocket.SetSocket(sck);
}

void Client::PushPacket(std::byte* pck, int bytes)
{
	if (mSendOverlapped == nullptr)
		mSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
	else
		mSendOverlapped->PushMsg(pck, bytes);
}

void Client::SendMsg()
{
	if (mSendOverlapped)
	{
		mSocket.Send(mSendOverlapped);
		mSendOverlapped = nullptr;
	}
}

void Client::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	mSocket.Recv(&mRecvOverlapped);
}

bool Client::ChangeState(CLIENT_STAT expected, const CLIENT_STAT& desired)
{
	return mState.compare_exchange_strong(expected, desired);
}

void Client::SendLoginResultPacket(LOGIN_STAT result)
{
	std::cout << "[" << ID << "] Send login result packet\n";
	SC::packet_login_result pck{};
	pck.size = sizeof(SC::packet_login_result);
	pck.type = SC::LOGIN_RESULT;
	pck.result = (char)result;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::SendMsg();
}

void Client::SendRegisterResultPacket(REGI_STAT result)
{
	std::cout << "[" << ID << "] Send login result packet\n";
	SC::packet_register_result pck{};
	pck.size = sizeof(SC::packet_register_result);
	pck.type = SC::REGISTER_RESULT;
	pck.result = (char)result;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::SendMsg();
}

void Client::SendAccessRoomDenyPacket(ROOM_STAT reason, int players)
{
	SC::packet_access_room_deny pck{};
	pck.size = sizeof(SC::packet_access_room_deny);
	pck.type = SC::ACCESS_ROOM_DENY;
	pck.reason = (char)reason;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	Client::SendMsg();
}
