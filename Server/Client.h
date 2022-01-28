#pragma once

enum class CLIENT_STAT : char
{
	EMPTY = 0,
	CONNECTED,
	LOBBY,
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

	void SetState(const CLIENT_STAT& stat) { mState = stat; }
	CLIENT_STAT GetCurrentState() const { return mState; }

public:
	void SendLoginResult(LOGIN_STAT result, bool instSend=true);
	void SendRegisterResult(REGI_STAT result, bool instSend=true);
	void SendAccessRoomAccept(int roomID, bool instSend=true);
	void SendAccessRoomDeny(ROOM_STAT reason, bool instSend=true);

	void SendForceLogout();
	
public:
	int ID;
	std::string Name;	
	std::atomic_int RoomID;
	std::atomic_char PlayerIndex;

	// for stress test
	std::atomic_uint64_t AccessRoomSendTime; 
	std::atomic_uint64_t ReadySendTime;

private:
	WSAOVERLAPPEDEX mRecvOverlapped;
	WSAOVERLAPPEDEX* mSendOverlapped;

	std::atomic<CLIENT_STAT> mState;	

	Socket mSocket;
};