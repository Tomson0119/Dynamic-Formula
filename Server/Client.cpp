#include "common.h"
#include "Client.h"
#include "WSAOverlappedEx.h"


Client::Client(int id, Socket* udpSck)
	: ID{ id }, 
	  RoomID{ -1 }, 
	  PlayerIndex{ -1 },
	  mState{ CLIENT_STAT::EMPTY },
	  mLobbyPageNum{ 0 },
	  mIsConnected{ false },
	  mLatency{ 0 },
	  mUDPSocketPtr{ udpSck },
	  mTCPSendOverlapped{ nullptr },
	  mTCPRecvOverlapped{ OP::RECV },
	  mUDPSendOverlapped{ nullptr },
	  mHostEp{}	  
{
	mTCPSocket.Init(SocketType::TCP);
	mTCPSocket.SetNagleOption(1);
}

Client::~Client()
{
}

void Client::Disconnect()
{	
	if (mTCPSendOverlapped)
	{
		delete mTCPSendOverlapped;
		mTCPSendOverlapped = nullptr;
	}
	if (mUDPSendOverlapped)
	{
		delete mUDPSendOverlapped;
		mUDPSendOverlapped = nullptr;
	}
	mState = CLIENT_STAT::EMPTY;
	mTCPSocket.Close();
	mIsConnected = false;
}

void Client::AssignAcceptedID(int id, SOCKET sck, sockaddr_in* addr)
{
	ID = id;
	mTCPSocket.SetSocket(sck);
	mIsConnected = true;
	
	addr->sin_port = htons(CLIENT_PORT);
	SetHostEp(EndPoint(*addr));
}

void Client::PushPacket(std::byte* pck, int bytes, bool udp)
{
	if (mIsConnected == false) return;
	if (udp && mUDPSocketPtr)
	{
		if (mUDPSendOverlapped == nullptr)
			mUDPSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
		else
			mUDPSendOverlapped->PushMsg(pck, bytes);
	}
	else
	{
		if (mTCPSendOverlapped == nullptr)
			mTCPSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
		else
			mTCPSendOverlapped->PushMsg(pck, bytes);
	}
}

void Client::SendMsg(bool udp)
{
	if (mIsConnected == false) return;
	if (udp && mUDPSocketPtr && mUDPSendOverlapped)
	{
		if (mUDPSocketPtr->SendTo(*mUDPSendOverlapped, mHostEp) < 0)
			std::cout << "Failed to send udp packet.\n";
		mUDPSendOverlapped = nullptr;
	}
	else if(udp == false && mTCPSendOverlapped)
	{
		mTCPSocket.Send(*mTCPSendOverlapped);
		mTCPSendOverlapped = nullptr;
	}
}

void Client::RecvMsg()
{	
	mTCPRecvOverlapped.Reset(OP::RECV);
	mTCPSocket.Recv(mTCPRecvOverlapped);
}

void Client::SetLatency(uint64_t sendTime)
{
	if (sendTime > 0)
	{
		auto now = Clock::now().time_since_epoch().count();
		mLatency = ((now - sendTime) / 2) / 1'000'000;
	}
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
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::LOGIN_RESULT);
	pck.result = (char)result;
	// TODO: Shouldn't send port in real life enviroment.
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
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::REGISTER_RESULT);
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
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::ACCESS_ROOM_ACCEPT);
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
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::ACCESS_ROOM_DENY);
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
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::FORCE_LOGOUT);
	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	SendMsg();
}

void Client::SendMeasureRTTPacket(uint64_t latency)
{
	SC::packet_measure_rtt pck{};
	pck.size = sizeof(SC::packet_measure_rtt);
	pck.type = static_cast<uint8_t>(SC::PCK_TYPE::MEASURE_RTT);

	uint64_t now = Clock::now().time_since_epoch().count();
	pck.s_send_time = now;
	pck.latency = latency;

	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size, true);
	SendMsg(true);
}