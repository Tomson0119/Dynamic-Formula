#pragma once

class NetClient
{	
public:
	NetClient();
	~NetClient();

public:
	bool Connect(const std::string& ip, u_short port);
	void Disconnect();

	void BindUDPSocket(u_short port);

	void PushPacket(std::byte* pck, int bytes, bool udp=false);
	void SendMsg(std::byte* pck, int bytes, bool udp=false);
	
	void SendMsg(bool udp=false);
	void RecvMsg(bool udp=false);
	
public:
	void RequestLogin(const std::string& name, const std::string& pwd);
	void RequestRegister(const std::string& name, const std::string& pwd);
	void RequestNewRoom();
	void RequestEnterRoom(int roomID);
	void InquireRoomList(int pageNum);

	void RevertScene();
	void SwitchMap(int roomID);
	void ToggleReady(int roomID);

	void SendLoadSequenceDone(int roomID);
	void SendKeyInput(int roomID, int key, bool pressed);

	void SendMeasureRTTPacket(uint64_t s_send_time);

public:
	SOCKET GetTCPSocket() const { return mTCPSocket.GetSocket(); }
	SOCKET GetUDPSocket() const { return mUDPSocket.GetSocket(); }

	u_short GetTCPSckPort() const { return mTCPSocket.GetPortNumber(); }
	bool IsConnected() const { return mIsConnected; }

private:
	Socket mTCPSocket;
	Socket mUDPSocket;

	WSAOVERLAPPEDEX* mTCPSendOverlapped;
	WSAOVERLAPPEDEX mTCPRecvOverlapped;
	
	WSAOVERLAPPEDEX* mUDPSendOverlapped;
	WSAOVERLAPPEDEX mUDPRecvOverlapped;

	EndPoint mServerEp;
	EndPoint mSenderEp;

	std::atomic_bool mIsConnected;
};