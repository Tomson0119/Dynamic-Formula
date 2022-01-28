#pragma once

struct PlayerInfo
{
	bool Empty;
	char Color;
	bool Ready;
	char Name[MAX_NAME_SIZE];
};

class NetClient
{
	using PlayerList = std::array<PlayerInfo, MAX_ROOM_CAPACITY>;
public:
	NetClient();
	~NetClient();

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

private:
	Socket mSocket;
	WSAOVERLAPPEDEX* mSendOverlapped;
	WSAOVERLAPPEDEX mRecvOverlapped;
};