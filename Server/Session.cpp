#include "common.h"
#include "Session.h"
#include "WSAOverlappedEx.h"


Session::Session(int id)
	: ID(id),
	  mRecvOverlapped{},
	  mState{ State::EMPTY }
{
	mSocket.Init();
}

Session::~Session()
{
}

void Session::Disconnect()
{
	mSocket.Close();
}

void Session::AssignAcceptedID(int id, SOCKET sck)
{
	ID = id;
	mSocket.SetSocket(sck);
}

void Session::SendMsg(uchar* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	mSocket.Send(*send_over);
}

void Session::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	mSocket.Recv(mRecvOverlapped);
}

bool Session::ChangeState(State expected, const State& desired)
{
	return mState.compare_exchange_strong(expected, desired);
}
