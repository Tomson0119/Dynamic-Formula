#include "../stdafx.h"
#include "NetClient.h"

NetClient::NetClient()
	: mIsConnected{ false },
	  mServerEp{},
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

bool NetClient::Connect(const char* ip, short port)
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

void NetClient::BindUDPSocket(short port)
{
	mUDPSocket.Bind(EndPoint::Any(port));
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
		mUDPSocket.RecvFrom(mUDPRecvOverlapped, mServerEp);
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
	pck.type = CS::LOGIN;
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
	pck.type = CS::REGISTER;
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
	pck.type = CS::OPEN_ROOM;
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
	pck.type = CS::ENTER_ROOM;
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::RevertScene()
{
#ifdef _DEBUG
	OutputDebugStringW(L"Sending revert scene.\n");
#endif
	CS::packet_revert_scene pck{};
	pck.size = sizeof(CS::packet_revert_scene);
	pck.type = CS::REVERT_SCENE;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::SwitchMap(int roomID)
{
#ifdef _DEBUG
	OutputDebugStringW(L"Sending switch map.\n");
#endif
	CS::packet_switch_map pck{};
	pck.size = sizeof(CS::packet_switch_map);
	pck.type = CS::SWITCH_MAP;
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::ToggleReady(int roomID)
{
	CS::packet_press_ready pck{};
	pck.size = sizeof(CS::packet_press_ready);
	pck.type = CS::PRESS_READY;
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
	pck.type = CS::LOAD_DONE;
	pck.room_id = roomID;

	auto duration = Clock::now().time_since_epoch();
	pck.send_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::SendKeyInput(int roomID, int key, bool pressed)
{
	CS::packet_key_input pck{};
	pck.size = sizeof(CS::packet_key_input);
	pck.type = CS::KEY_INPUT;
	pck.key = (uint8_t)key;
	pck.pressed = pressed;
	pck.room_id = roomID;

	auto duration = Clock::now().time_since_epoch();
	pck.send_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}

void NetClient::ReturnSendTimeBack(uint64_t sendTime)
{
	CS::packet_transfer_time pck{};
	pck.size = sizeof(CS::packet_transfer_time);
	pck.type = CS::TRANSFER_TIME;
	pck.send_time = sendTime;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}
