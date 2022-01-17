#pragma once

enum class State : char
{
	EMPTY,
	LOGIN,
};

class Session
{
public:
	Session(int id);
	virtual ~Session();

	void Disconnect();
	void AssignAcceptedID(int id, SOCKET sck);
	
	void PushPacket(std::byte* pck, int bytes);

	void SendMsg();
	void RecvMsg();

	bool ChangeState(State expected, const State& desired);

public:
	void SendLoginResultPacket(LOGIN_STAT result);
	void SendEnterRoomDenyPacket(ROOM_STAT reason, int players);

public:
	int ID;
	std::string Name;

private:
	WSAOVERLAPPEDEX mRecvOverlapped;
	WSAOVERLAPPEDEX* mSendOverlapped;

	std::atomic<State> mState;

	Socket mSocket;
};