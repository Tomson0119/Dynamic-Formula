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
	Client(int id, Socket* udpSck);
	virtual ~Client();

	void Disconnect();
	void AssignAcceptedID(int id, SOCKET sck, sockaddr_in* addr);
	
	void PushPacket(std::byte* pck, int bytes, bool udp=false);

	void SendMsg(bool udp=false);
	void RecvMsg();

public:
	void SetLatency(uint64_t sendTime);
	uint64_t GetLatency() const { return mLatencyMs; }

	bool ChangeState(CLIENT_STAT expected, const CLIENT_STAT& desired);
	void SetState(const CLIENT_STAT& stat) { mState = stat; }
	CLIENT_STAT GetCurrentState() const { return mState; }

	void SetHostEp(const EndPoint& ep) { mHostEp = ep; }
	const EndPoint& GetHostEp() const { return mHostEp; }

	void SetPageNum(int n) { mLobbyPageNum = n; }
	int GetPageNum() const { return mLobbyPageNum; }

public:
	void SendLoginResult(LOGIN_STAT result, bool instSend=true);
	void SendRegisterResult(REGI_STAT result, bool instSend=true);
	void SendAccessRoomAccept(int roomID, bool instSend=true);
	void SendAccessRoomDeny(ROOM_STAT reason, bool instSend=true);
	void SendForceLogout();
	void SendMeasureRTTPacket(bool udp=false, bool instSend=true);
	void SendHolePunchingAck();
	
public:
	int ID;
	std::string Name;
	std::atomic_int RoomID;
	std::atomic_char PlayerIndex;

private:
	std::atomic<CLIENT_STAT> mState;
	std::atomic_int mLobbyPageNum;
	
	std::atomic_bool mIsConnected;
	std::atomic_uint64_t mLatencyMs;

	Socket mTCPSocket;
	Socket* mUDPSocketPtr;

	WSAOVERLAPPEDEX* mTCPSendOverlapped;
	WSAOVERLAPPEDEX mTCPRecvOverlapped;

	WSAOVERLAPPEDEX* mUDPSendOverlapped;

	EndPoint mHostEp;
};