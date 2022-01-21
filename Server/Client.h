#pragma once

enum class CLIENT_STAT : char
{
	EMPTY,
	CONNECTED,
	LOGIN,
	IN_ROOM,
	IN_GAME
};

class Client
{
public:
	Client(int id);
	virtual ~Client();

	void Disconnect();
	void AssignAcceptedID(int id, SOCKET sck);
	
	void PushPacket(std::byte* pck, int bytes);

	void SendMsg();
	void RecvMsg();

	bool ChangeState(CLIENT_STAT expected, const CLIENT_STAT& desired);

	CLIENT_STAT GetCurrentState() const { return mState.load(); }

public:
	void SendLoginResult(LOGIN_STAT result, bool instSend=true);
	void SendRegisterResult(REGI_STAT result, bool instSend=true);
	void SendAccessRoomDeny(ROOM_STAT reason, int players, bool instSend=true);
	
public:
	int ID;
	std::string Name;

private:
	WSAOVERLAPPEDEX mRecvOverlapped;
	WSAOVERLAPPEDEX* mSendOverlapped;

	std::atomic<CLIENT_STAT> mState;

	Socket mSocket;
};