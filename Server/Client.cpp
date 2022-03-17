#include "common.h"
#include "Client.h"
#include "WSAOverlappedEx.h"


Client::Client(int id, Socket* udpSck)
	: ID(id), mUDPSocketPtr{ udpSck },
	  mHostEp{},
	  mTCPRecvOverlapped{},
	  mTCPSendOverlapped{},
	  mUDPSendOverlapped{},
	  mState{ CLIENT_STAT::EMPTY },
	  RoomID(-1), PlayerIndex(-1),
	  mTransferTime{ 0 }
{
	mTCPSocket.Init(SocketType::TCP);
	mTCPSocket.SetNagleOption(1);
}

Client::~Client()
{
}

void Client::Disconnect()
{	
	mState = CLIENT_STAT::EMPTY;
	mTCPSocket.Close();
}

void Client::AssignAcceptedID(int id, SOCKET sck, sockaddr_in* addr)
{
	ID = id;
	mTCPSocket.SetSocket(sck);
	
	addr->sin_port = htons(CLIENT_PORT + (short)id); // test	
	SetHostEp(EndPoint(*addr));
}

void Client::PushPacket(std::byte* pck, int bytes, bool udp)
{
	if (udp == false)
	{
		if (mTCPSendOverlapped == nullptr)
			mTCPSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
		else
			mTCPSendOverlapped->PushMsg(pck, bytes);
	}
	else
	{
		if (mUDPSendOverlapped == nullptr)
			mUDPSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
		else
			mUDPSendOverlapped->PushMsg(pck, bytes);
	}
}

void Client::SendMsg(bool udp)
{
	if (udp == false && mTCPSendOverlapped)
	{
		mTCPSocket.Send(mTCPSendOverlapped);
		mTCPSendOverlapped = nullptr;
	}
	else if (udp == true && mUDPSendOverlapped)
	{
		mUDPSocketPtr->SendTo(mUDPSendOverlapped, mHostEp);
		mUDPSendOverlapped = nullptr;
	}
}

void Client::RecvMsg()
{	
	mTCPRecvOverlapped.Reset(OP::RECV);
	mTCPSocket.Recv(&mTCPRecvOverlapped);	
}

void Client::SetTransferTime(uint64_t sendTime)
{
	auto duration = Clock::now().time_since_epoch();
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	mTransferTime = now - sendTime;
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
	pck.port = ntohs(mHostEp.mAddress.sin_port);
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
	std::cout << "[" << ID << "] Send access room accept packet. (room id: " << roomID << ")\n";
#endif
	SC::packet_access_room_accept pck{};
	pck.size = sizeof(SC::packet_access_room_accept);
	pck.type = SC::ACCESS_ROOM_ACCEPT;
	pck.room_id = roomID;
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

void Client::SendTransferTime(bool instSend)
{
	SC::packet_transfer_time pck{};
	pck.size = sizeof(SC::packet_transfer_time);
	pck.type = SC::TRANSFER_TIME;
	pck.recv_time = mTransferTime;
	
	auto duration = Clock::now().time_since_epoch();
	pck.send_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	if (instSend) SendMsg();
}
