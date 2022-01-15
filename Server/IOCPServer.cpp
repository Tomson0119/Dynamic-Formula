#include "common.h"
#include "IOCPServer.h"

//#define USE_RINGBUFFER

std::array<std::unique_ptr<Session>, MAX_PLAYER> IOCPServer::gClients;

IOCPServer::IOCPServer(const EndPoint& ep)
	: mLoop(true)
{
	if (mDBHandler.ConnectToDB(L"sql_server") == false)
		std::cout << "failed to connect to DB\n";

	for (int i = 0; i < gClients.size(); i++)
	{
		gClients[i] = std::make_unique<Session>(i);
	}

	mListenSck.Init();
	mListenSck.Bind(ep);
}

IOCPServer::~IOCPServer()
{
}

void IOCPServer::Run()
{
	mListenSck.Listen();
	mIOCP.RegisterDevice(mListenSck.GetSocket(), 0);
	std::cout << "Listening to clients...\n";

	WSAOVERLAPPEDEX acceptEx;
	mListenSck.AsyncAccept(acceptEx);

	for (int i = 0; i < MaxThreads; i++)
		mThreads.emplace_back(NetworkThreadFunc, std::ref(*this));

	for (std::thread& thrd : mThreads)
		thrd.join();
}

void IOCPServer::NetworkThreadFunc(IOCPServer& server)
{
	CompletionInfo info{};
	while (server.mLoop)
	{
		try {
			server.mIOCP.GetCompletionInfo(info);

			int client_id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);
			
			if (info.success == FALSE)
			{
				server.Disconnect(client_id);
				if (over_ex->Operation == OP::SEND)
					delete over_ex;
				continue;
			}
			server.HandleCompletionInfo(over_ex, client_id, info.bytes);
		}
		catch (std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}
	}
}

void IOCPServer::HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes)
{
	switch (over->Operation)
	{
	case OP::RECV:
	{
		if (bytes == 0)
		{
			Disconnect(id);
			break;
		}
		Session* client = gClients[id].get();
		
#ifndef USE_RINGBUFFER
		AssemblePacket(id, over, bytes);
#endif
		client->RecvMsg();
		break;
	}
	case OP::SEND:
	{
		if (bytes != over->WSABuffer.len)
			Disconnect(id);
		delete over;
		break;
	}
	case OP::ACCEPT:
	{
		SOCKET clientSck = *reinterpret_cast<SOCKET*>(over->MsgQueue.GetBuffer());
		
		int i = GetAvailableID();
		if (i == -1) 
			std::cout << "Session's full\n";
		else {
			AcceptNewClient(i, clientSck);
		}
		mListenSck.AsyncAccept(*over);
		break;
	}
	}
}

void IOCPServer::AssemblePacket(int client_id, WSAOVERLAPPEDEX* over, int bytes)
{
	std::byte* packet_start = over->MsgQueue.GetBuffer();
	int remain_len = bytes + gClients[client_id]->PrevSize;
	int packet_size = static_cast<int>(packet_start[0]);

	while (packet_size <= remain_len)
	{
		ProcessPackets(client_id, packet_start);
		remain_len -= packet_size;
		packet_start += packet_size;
		if (remain_len > 0) packet_size = static_cast<int>(packet_start[0]);
		else break;
	}

	if (remain_len > 0)
	{
		gClients[client_id]->PrevSize = remain_len;
		std::memcpy(over->MsgQueue.GetBuffer(), packet_start, remain_len);
	}
}

void IOCPServer::Disconnect(int id)
{
	std::cout << "Disconnect [" << id << "]\n";
	gClients[id]->Disconnect();
}

void IOCPServer::AcceptNewClient(int id, SOCKET sck)
{
	std::cout << "Accepted client [" << id << "]\n";
	gClients[id]->AssignAcceptedID(id, sck);
	mIOCP.RegisterDevice(sck, id);
	gClients[id]->RecvMsg();
}

void IOCPServer::ProcessPackets(int id, std::byte* packet)
{
	char type = static_cast<char>(packet[1]);
	std::cout << "[" << id << "] ";
	switch (type)
	{
	case CS::LOGIN:
	{
		CS::login_packet* pck = reinterpret_cast<CS::login_packet*>(packet);
		std::cout << "Login packet: " << pck->name << std::endl;
		break;
	}
	case CS::CHAT:
	{
		CS::chat_packet* pck = reinterpret_cast<CS::chat_packet*>(packet);
		std::cout << "Chat packet: " << pck->message << std::endl;
		break;
	}
	case CS::MOVE:
	{
		CS::move_packet* pck = reinterpret_cast<CS::move_packet*>(packet);
		std::cout << "Move packet: " << (int)pck->direction << " " << pck->move_time << std::endl;
		break;
	}
	default:
		std::cout << "Invalid packet\n";
		break;
	}
}

int IOCPServer::GetAvailableID()
{
	for (int i = 0; i < gClients.size(); i++)
	{
		if (gClients[i]->ChangeState(State::EMPTY, State::LOGIN))
			return i;
	}
	return -1;
}