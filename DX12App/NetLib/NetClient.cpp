#include "../stdafx.h"
#include "NetClient.h"

NetClient::NetClient()
	: mSendOverlapped{ nullptr }
{
	mSocket.Init();
}

bool NetClient::Connect(const char* ip, short port)
{
	return mSocket.Connect(EndPoint(ip, port));
}

void NetClient::Disconnect()
{
	mSocket.Close();
}

void NetClient::PushPacket(std::byte* pck, int bytes)
{
	if (mSendOverlapped == nullptr)
		mSendOverlapped = new WSAOVERLAPPEDEX(OP::SEND, pck, bytes);
	else
		mSendOverlapped->PushMsg(pck, bytes);
}

void NetClient::SendMsg(std::byte* pck, int bytes)
{
	PushPacket(pck, bytes);
	SendMsg();
}

void NetClient::SendMsg()
{
	if (mSendOverlapped)
	{
		mSocket.Send(mSendOverlapped);
		mSendOverlapped = nullptr;
	}
}

void NetClient::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	mSocket.Recv(&mRecvOverlapped);
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

void NetClient::SendKeyInput(int roomID, int key, bool pressed)
{
#ifdef _DEBUG
	OutputDebugStringW(L"Sending key input.\n");
#endif
	CS::packet_key_input pck{};
	pck.size = sizeof(CS::packet_key_input);
	pck.type = CS::KEY_INPUT;
	pck.key = (uint8_t)key;
	pck.pressed = pressed;
	pck.room_id = roomID;
	SendMsg(reinterpret_cast<std::byte*>(&pck), pck.size);
}
