#pragma once

#include "NetClient.h"

class Scene;

class NetModule
{
public:
	NetModule();
	~NetModule();

	bool Connect(const char* ip, short port);
	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes);
	void ReadRecvBuffer(WSAOVERLAPPEDEX* over, int bytes);
	bool ProcessPacket(std::byte* packet, char type, int bytes);

	static void NetworkFunc(NetModule& net);

public:
	void SetInterface(Scene* scenePtr) { mScenePtr = scenePtr; }
	NetClient* Client() const { return mNetClient.get(); }

private:
	void Init();

private:
	std::atomic_bool mLoop;

	std::unique_ptr<NetClient> mNetClient;
	std::thread mNetThread;

	IOCP mIOCP;
	Scene* mScenePtr;
};
