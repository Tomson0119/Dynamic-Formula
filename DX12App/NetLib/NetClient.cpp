#include "../stdafx.h"
#include "NetClient.h"

NetClient::NetClient()
	: mIsConnected{ false },
	  mServerEp{},
	  mSenderEp{},
	  mTCPSendOverlapped{ nullptr },
	  mTCPRecvOverlapped{ OP::RECV },
	  mUDPSendOverlapped{ nullptr },
	  mUDPRecvOverlapped{ OP::RECV }
{
	mTCPSocket.Init(SocketType::TCP);
	mTCPSocket.SetNagleOption(1);

	mUDPSocket.Init(SocketType::UDP);
}

NetClient::~NetClient()
{
	Disconnect();
}

bool NetClient::Connect(const std::string& ip, u_short port)
{
	mServerEp = EndPoint(ip, port);
	mIsConnected = mTCPSocket.Connect(mServerEp);
	return mIsConnected;
}

void NetClient::Disconnect()
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
	if (mIsConnected)
	{
		OutputDebugStringA("Disconnecting...\n");
		mTCPSocket.Close();
		mUDPSocket.Close();
		mIsConnected = false;
	}
}

void NetClient::BindUDPSocket(u_short port)
{
	auto ep = EndPoint::Any(port);
	mUDPSocket.Bind(ep);
	RecvMsg(true);
}

void NetClient::PushPacket(std::byte* pck, int bytes, bool udp)
{
	if (mIsConnected == false) return;
	if (udp)
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

void NetClient::SendMsg(std::byte* pck, int bytes, bool udp)
{
	PushPacket(pck, bytes, udp);
	SendMsg(udp);
}

void NetClient::SendMsg(bool udp)
{
	if (mIsConnected == false) return;
	if (udp && mUDPSendOverlapped)
	{
		mUDPSocket.SendTo(*mUDPSendOverlapped, mServerEp);
		mUDPSendOverlapped = nullptr;
	}
	else if (udp == false && mTCPSendOverlapped)
	{
		mTCPSocket.Send(*mTCPSendOverlapped);
		mTCPSendOverlapped = nullptr;
	}
}

void NetClient::RecvMsg(bool udp)
{
	if (udp)
	{
		mUDPRecvOverlapped.NetBuffer.Clear();
		mUDPRecvOverlapped.Reset(OP::RECV);
		mUDPSocket.RecvFrom(mUDPRecvOverlapped, mSenderEp);
	}
	else
	{
		mTCPRecvOverlapped.Reset(OP::RECV);
		mTCPSocket.Recv(mTCPRecvOverlapped);
	}
}

void NetClient::RequestLogin(const std::string& name, const std::string& pwd)
{
#ifdef _DEBUG
	OutputDebugStringW(L"Requesting login\n");
#endif
	CS::packet_login pck{};
	pck.size = sizeof(CS::packet_login);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::LOGIN);
	strncpy_s(pck.name, name.c_str(), MAX_NAME_SIZE - 1);
	strncpy_s(pck.pwd, pwd.c_str(), MAX_PWD_SIZE - 1);
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::RequestRegister(const std::string& name, const std::string& pwd)
{
#ifdef _DEBUG
	OutputDebugStringW(L"Requesting register\n");
#endif
	CS::packet_register pck{};
	pck.size = sizeof(CS::packet_register);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::REGISTER);
	strncpy_s(pck.name, name.c_str(), MAX_NAME_SIZE - 1);
	strncpy_s(pck.pwd, pwd.c_str(), MAX_PWD_SIZE - 1);
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::RequestNewRoom()
{
#ifdef _DEBUG
	OutputDebugStringW(L"Requesting new room.\n");
#endif
	CS::packet_open_room pck{};
	pck.size = sizeof(CS::packet_open_room);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::OPEN_ROOM);
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::RequestEnterRoom(int roomID)
{
#ifdef _DEBUG
	std::wstring debugWStr = L"Requesting enter room (room id: " + std::to_wstring(roomID) + L")\n";
	OutputDebugStringW(debugWStr.c_str());
#endif
	CS::packet_enter_room pck{};
	pck.size = sizeof(CS::packet_enter_room);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::ENTER_ROOM);
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::InquireRoomList(int pageNum)
{
#ifdef _DEBUG
	OutputDebugStringA(("Inquiring room list (page num: " + std::to_string(pageNum) + ")\n").c_str());
#endif
	CS::packet_inquire_room pck{};
	pck.size = sizeof(CS::packet_inquire_room);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::INQUIRE_ROOM);
	pck.page_num = pageNum;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::RevertScene()
{
#ifdef _DEBUG
	OutputDebugStringW(L"Sending revert scene.\n");
#endif
	CS::packet_revert_scene pck{};
	pck.size = sizeof(CS::packet_revert_scene);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::REVERT_SCENE);
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::SwitchMap(int roomID)
{
#ifdef _DEBUG
	OutputDebugStringW(L"Sending switch map.\n");
#endif
	CS::packet_switch_map pck{};
	pck.size = sizeof(CS::packet_switch_map);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::SWITCH_MAP);
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::ToggleReady(int roomID)
{
	CS::packet_press_ready pck{};
	pck.size = sizeof(CS::packet_press_ready);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::PRESS_READY);
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::SendLoadSequenceDone(int roomID)
{
#ifdef _DEBUG
	OutputDebugStringW(L"Sending Loading done packet.\n");
#endif

	CS::packet_load_done pck{};
	pck.size = sizeof(CS::packet_load_done);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::LOAD_DONE);
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::SendKeyInput(int roomID, int key, bool pressed)
{
	CS::packet_key_input pck{};
	pck.size = sizeof(CS::packet_key_input);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::KEY_INPUT);
	pck.key = (uint8_t)key;
	pck.pressed = pressed;
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::SendMeasureRTTPacket(uint64_t s_send_time)
{
	CS::packet_measure_rtt pck{};
	pck.size = sizeof(CS::packet_measure_rtt);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::MEASURE_RTT);
	pck.s_send_time = s_send_time;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}

void NetClient::SendUDPConnectionPacket()
{
#ifdef _DEBUG
	OutputDebugStringW(L"Sending UDP connection packet.\n");
#endif
	CS::packet_udp_connection pck{};
	pck.size = sizeof(CS::packet_udp_connection);
	pck.type = static_cast<uint8_t>(CS::PCK_TYPE::UDP_CONNECT);
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size, true);
}
