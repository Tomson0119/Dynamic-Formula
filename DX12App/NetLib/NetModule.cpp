#include "../stdafx.h"
#include "../scene.h"
#include "NetModule.h"

NetModule::NetModule()
	: mNetClient{ std::make_unique<NetClient>() },
	  mLoop{ true }, mScenePtr{ nullptr }
{
}

NetModule::~NetModule()
{
	if (mNetThread.joinable()) mNetThread.join();
}

bool NetModule::Connect(const char* ip, short port)
{
	if (mNetClient->Connect(ip, port))
	{
		Init();
		return true;
	}
	return false;
}

void NetModule::NetworkFunc(NetModule& net)
{
	CompletionInfo info{};
	while (net.mLoop)
	{
		try {
			net.mIOCP.GetCompletionInfo(info);

			int client_id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);

			if (over_ex == nullptr || info.success == FALSE)
			{
				net.mNetClient->Disconnect();
				if (over_ex && over_ex->Operation == OP::SEND)
					delete over_ex;
				continue;
			}
			net.HandleCompletionInfo(over_ex, info.bytes);
		}
		catch (std::exception& ex)
		{
			std::string str = ex.what();
			OutputDebugStringA((str + "\n").c_str());
		}
	}
}

void NetModule::HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes)
{
	switch (over->Operation)
	{
	case OP::RECV:
	{
		if (bytes == 0)
		{
			mNetClient->Disconnect();
			break;
		}
		over->NetBuffer.ShiftWritePtr(bytes);
		ReadRecvBuffer(over, bytes);
		mNetClient->RecvMsg();
		break;
	}
	case OP::SEND:
	{
		if (bytes != over->WSABuffer.len)
			mNetClient->Disconnect();
		delete over;
		break;
	}
	}
}

void NetModule::ReadRecvBuffer(WSAOVERLAPPEDEX* over, int bytes)
{
	while (over->NetBuffer.Readable())
	{
		std::byte* packet = over->NetBuffer.BufReadPtr();
		char type = GetPacketType(packet);

		if (packet == nullptr || ProcessPacket(packet, type, bytes) == false) {
			over->NetBuffer.Clear();
			break;
		}
	}
}

bool NetModule::ProcessPacket(std::byte* packet, char type, int bytes)
{
	switch (type)
	{
	case SC::LOGIN_RESULT:
	{
		OutputDebugStringW(L"Login result packet received..\n");
		mScenePtr->SetSceneFlag(true);
		break;
	}
	default:
	{
		OutputDebugStringW(L"Invalid packet type..\n");
		return false;
	}
	}
	return true;
}

void NetModule::Init()
{
	mIOCP.RegisterDevice(mNetClient->GetSocket(), 0);
	mNetClient->RecvMsg();
	mNetThread = std::thread{ NetworkFunc, std::ref(*this) };
}
