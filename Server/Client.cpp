#include "common.h"
#include "Client.h"
#include "WSAOverlappedEx.h"


Client::Client(int id)
	: ID(id),
	  mRecvOverlapped{},
	  mSendOverlapped{},
	  mState{ CLIENT_STAT::EMPTY },
	  RoomID(-1), PlayerIndex(-1)
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

void Client::SendLoginResult(LOGIN_STAT result, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << ID << "] Send login result packet\n";
#endif
	SC::packet_login_result pck{};
	pck.size = sizeof(SC::packet_login_result);
	pck.type = SC::LOGIN_RESULT;
	pck.result = (char)result;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) SendMsg();
}

void Client::SendRegisterResult(REGI_STAT result, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << ID << "] Send login result packet\n";
#endif
	SC::packet_register_result pck{};
	pck.size = sizeof(SC::packet_register_result);
	pck.type = SC::REGISTER_RESULT;
	pck.result = (char)result;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) SendMsg();
}

void Client::SendAccessRoomAccept(int roomID, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << ID << "] Send access room accept packet\n";
#endif
	SC::packet_access_room_accept pck{};
	pck.size = sizeof(SC::packet_access_room_accept);
	pck.type = SC::ACCESS_ROOM_ACCEPT;
	pck.room_id = roomID;
	pck.send_time = AccessRoomSendTime;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) SendMsg();
}

void Client::SendAccessRoomDeny(ROOM_STAT reason, bool instSend)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << ID << "] Send access room deny packet\n";
#endif
	SC::packet_access_room_deny pck{};
	pck.size = sizeof(SC::packet_access_room_deny);
	pck.type = SC::ACCESS_ROOM_DENY;
	pck.reason = (char)reason;
	pck.send_time = AccessRoomSendTime;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if(instSend) SendMsg();
}

void Client::SendForceLogout()
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << ID << "] Send force logout packet\n";
#endif
	SC::packet_force_logout pck{};
	pck.size = sizeof(SC::packet_force_logout);
	pck.type = SC::FORCE_LOGOUT;
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	SendMsg();
}