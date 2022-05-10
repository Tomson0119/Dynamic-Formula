#include "common.h"
#include "Client.h"
#include "WSAOverlappedEx.h"


Client::Client(int id, Socket* udpSck)
	: ID(id), mUDPSocketPtr{ udpSck },
	  mHostEp{},
	  mState{ CLIENT_STAT::EMPTY },
	  RoomID(-1), PlayerIndex(-1),
	  mIsConnected{ false },
	  mLatency{ 0 }
{
	mTCPSocket.Init(SocketType::TCP);
	mTCPSocket.SetNagleOption(1);
}

Client::~Client()
{
}

void Client::Disconnect()
{	
	/*if (mTCPSendOverlapped)
	{
		delete mTCPSendOverlapped;
		mTCPSendOverlapped = nullptr;
	}
	if (mUDPSendOverlapped)
	{
		delete mUDPSendOverlapped;
		mUDPSendOverlapped = nullptr;
	}*/

	mState = CLIENT_STAT::EMPTY;
	mTCPSocket.Close();
	mIsConnected = false;
}

void Client::AssignAcceptedID(int id, SOCKET sck, sockaddr_in* addr)
{
	ID = id;
	mTCPSocket.SetSocket(sck);
	mIsConnected = true;
	
	addr->sin_port = htons(CLIENT_PORT); // test	
	SetHostEp(EndPoint(*addr));
}

void Client::PushPacket(std::byte* pck, int bytes, bool udp)
{
	if (mIsConnected == false) return;
	if (udp) mUDPSocketPtr->PushPacket(pck, bytes);
	else mTCPSocket.PushPacket(pck, bytes);
}

void Client::SendMsg(bool udp)
{
	if (mIsConnected == false) return;
	if (udp) mUDPSocketPtr->SendTo(mHostEp);
	else mTCPSocket.Send();
}

void Client::RecvMsg(bool udp)
{	
	if (udp) mUDPSocketPtr->RecvFrom(mHostEp);
	else mTCPSocket.Recv();
}

void Client::SetLatency(uint64_t sendTime)
{
	using namespace std::chrono;

	auto duration = Clock::now().time_since_epoch();
	auto now = duration_cast<milliseconds>(duration).count();
	mLatency = (now - sendTime) / 2;
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
	// TODO: Shouldn't send port in real life enviroment.
	//pck.port = ntohs(mHostEp.mAddress.sin_port);
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

void Client::ReturnSendTimeBack(uint64_t sendTime)
{
	SC::packet_transfer_time pck{};
	pck.size = sizeof(SC::packet_transfer_time);
	pck.type = SC::TRANSFER_TIME;
	pck.c_send_time = sendTime;

	auto duration = Clock::now().time_since_epoch();
	pck.s_send_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	PushPacket(reinterpret_cast<std::byte*>(&pck), pck.size);
	SendMsg();
}
