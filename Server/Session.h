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
	void SendMsg(uchar* msg, int bytes);
	void RecvMsg();

	bool ChangeState(State expected, const State& desired);

public:
	int ID;

private:
	WSAOVERLAPPEDEX mRecvOverlapped;
	std::atomic<State> mState;

	Socket mSocket;
};