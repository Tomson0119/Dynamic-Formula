#include "common.h"
#include "Session.h"
#include "WSAOverlappedEx.h"


Session::Session(int id)
	: ID(id),
	  mRecvOverlapped{},
	  mSendOverlapped{},
	  mState{ State::EMPTY }
{
	mSocket.Init();
}

Session::~Session()
{
}

void Session::Disconnect()
{
	mState = State::EMPTY;
	mSocket.Close();
}

void Session::AssignAcceptedID(int id, SOCKET sck)
{
	ID = id;
	mSocket.SetSocket(sck);
}

void Session::PushPacket(std::byte* pck, int bytes)
{
	if (mSendOverlapped == nullptr)
		mSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
	else
		mSendOverlapped->PushMsg(pck, bytes);
}

void Session::SendMsg()
{
	if (mSendOverlapped)
	{
		mSocket.Send(mSendOverlapped);
		mSendOverlapped = nullptr;
	}
}

void Session::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	mSocket.Recv(&mRecvOverlapped);
}

bool Session::ChangeState(State expected, const State& desired)
{
	return mState.compare_exchange_strong(expected, desired);
}

void Session::SendLoginResultPacket(LOGIN_STAT result)
{
	std::cout << "[" << ID << "] Send login ok packet\n";
	SC::packet_login_result pck{};
	pck.size = sizeof(SC::packet_login_result);
	pck.type = SC::LOGIN_RESULT;
	pck.result = (char)result;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	//Session::SendMsg();
}

void Session::SendEnterRoomDenyPacket(ROOM_STAT reason, int players)
{
	SC::packet_enter_room_deny pck{};
	pck.size = sizeof(SC::packet_enter_room_deny);
	pck.type = SC::ENTER_ROOM_DENY;
	pck.reason = (char)reason;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	//Session::SendMsg();
}
