#pragma once

class NetClient
{	
public:
	NetClient();
	~NetClient() = default;

public:
	bool Connect(const char* ip, short port);
	void Disconnect();

	void PushPacket(std::byte* pck, int bytes);
	void SendMsg(std::byte* pck, int bytes);
	
	void SendMsg();
	void RecvMsg();
	
public:
	void RequestLogin(const std::string& name, const std::string& pwd);
	void RequestRegister(const std::string& name, const std::string& pwd);
	void RequestNewRoom();
	void RequestEnterRoom(int roomID);

	void RevertScene();
	void SwitchMap(int roomID);
	void ToggleReady(int roomID);

public:
	SOCKET GetSocket() const { return mSocket.GetSocket(); }

private:
	Socket mSocket;
	WSAOVERLAPPEDEX* mSendOverlapped;
	WSAOVERLAPPEDEX mRecvOverlapped;
};