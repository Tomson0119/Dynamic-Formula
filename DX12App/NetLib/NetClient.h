#pragma once

class NetClient
{	
public:
	NetClient();
	~NetClient() = default;

public:
	bool Connect(const char* ip, short port);
	void Disconnect();

	void BindUDPSocket(short port);

	void PushPacket(std::byte* pck, int bytes);
	void SendMsg(std::byte* pck, int bytes);
	
	void SendMsg();
	void RecvMsg(bool udp=false);
	
public:
	void RequestLogin(const std::string& name, const std::string& pwd);
	void RequestRegister(const std::string& name, const std::string& pwd);
	void RequestNewRoom();
	void RequestEnterRoom(int roomID);

	void RevertScene();
	void SwitchMap(int roomID);
	void ToggleReady(int roomID);

	void SendLoadSequenceDone(int roomID);
	void SendKeyInput(int roomID, int key, bool pressed);

	void ReturnSendTimeBack(uint64_t sendTime);

public:
	SOCKET GetTCPSocket() const { return mTCPSocket.GetSocket(); }
	SOCKET GetUDPSocket() const { return mUDPSocket.GetSocket(); }
	bool IsConnected() const { return mIsConnected; }

private:
	Socket mTCPSocket;
	Socket mUDPSocket;

	EndPoint mServerEp;

	WSAOVERLAPPEDEX* mTCPSendOverlapped;

	WSAOVERLAPPEDEX mTCPRecvOverlapped;
	WSAOVERLAPPEDEX mUDPRecvOverlapped;

	std::atomic_bool mIsConnected;
};