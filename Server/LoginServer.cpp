#include "common.h"
#include "LoginServer.h"
#include "InGameServer.h"
#include "Client.h"
#include "MemoryPool.h"

std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> gClients;
IOCP LoginServer::msIOCP;

LoginServer::LoginServer(const EndPoint& ep)
{
	std::signal(SIGINT, SignalHandler);

	for (int i = 0; i < mDBHandlers.size(); i++)
	{
	#ifdef USE_DATABASE
		if (mDBHandlers[i].ConnectToDB(L"sql_server"))
			mDBHandlers[i].ResetAllHost();
		else std::cout << "failed to connect to DB\n";
	#endif
	}

	mLobby.Init(this);

	mUDPReceiver.Bind(ep);
	
	for (int i = 0; i < gClients.size(); i++)
		gClients[i] = std::make_unique<Client>(i, mUDPReceiver.GetSocket());

	mListenSck.Init(SocketType::TCP);
	mListenSck.Bind(ep);

	// Pre-allocate memory pools.
	MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().Init(10 * MAX_THREAD_COUNT);
}

LoginServer::~LoginServer()
{
	for (int i = 0; i < gClients.size(); i++)
	{
		if (gClients[i]->GetCurrentState() != CLIENT_STAT::EMPTY)
		{
			Disconnect(i);
		}
	}
}

void LoginServer::Run()
{
	mListenSck.Listen();
	msIOCP.RegisterDevice(mListenSck.GetSocket(), 0);

	mUDPReceiver.RecvMsg();
	auto udpSck = mUDPReceiver.GetSocket();
	msIOCP.RegisterDevice(udpSck->GetSocket(), MAX_PLAYER_SIZE);

	WSAOVERLAPPEDEX acceptEx;
	mListenSck.AsyncAccept(&acceptEx);

	for (int i = 0; i < mThreads.size(); i++)
	{
		mThreads[i] = std::thread{ NetworkThreadFunc, std::ref(*this) };
		mThreadIdMap.insert({ mThreads[i].get_id(), i });
	}

	// Assign main thread id as 0
	mThreadIdMap.insert({ std::this_thread::get_id(), 0 });

	std::cout << "Listening to clients...\n";
	for (std::thread& thrd : mThreads) thrd.join();
}

void LoginServer::NetworkThreadFunc(LoginServer& server)
{
	bool loop = true;
	CompletionInfo info{};
	while (loop)
	{
		try {
			server.msIOCP.GetCompletionInfo(info);

			int client_id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);

			if (over_ex == nullptr)
			{
				loop = false;
				continue;
			}
			if (info.success == FALSE)
			{
				if(client_id < MAX_PLAYER_SIZE)
					server.Disconnect(client_id);
				if (over_ex && over_ex->Operation == OP::SEND)
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

void LoginServer::HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes)
{
	switch (over->Operation)
	{
	case OP::RECV:
	{
		if (id < MAX_PLAYER_SIZE && bytes == 0)
		{
			Disconnect(id);
			break;
		}

		over->NetBuffer.ShiftWritePtr(bytes);
		ReadRecvBuffer(over, id, bytes);

		if (id < MAX_PLAYER_SIZE)
		{
			Client* client = gClients[id].get();
			client->RecvMsg();
		}
		else
		{
			mUDPReceiver.RecvMsg();
		}
		break;
	}
	case OP::SEND:
	{
		if (id < MAX_PLAYER_SIZE && bytes != over->WSABuffer.len)
		{
			//Disconnect(id);
		}
		delete over;
		break;
	}
	case OP::ACCEPT:
	{
		SOCKET clientSck = *reinterpret_cast<SOCKET*>(over->NetBuffer.BufStartPtr());

		sockaddr_in* remote = reinterpret_cast<sockaddr_in*>(
			over->NetBuffer.BufStartPtr() + sizeof(SOCKET) + sizeof(sockaddr_in) + 16);

		int i = GetAvailableID();
		if (i == -1) 
			std::cout << "Session's full\n";
		else
			AcceptNewClient(i, clientSck, remote);

		mListenSck.AsyncAccept(over);
		break;
	}
	case OP::PHYSICS:
	{
		mLobby.GetInGameServer().RunPhysicsSimulation(id);
		break;
	}}
}

void LoginServer::AcceptLogin(const char* name, int id)
{
	if (gClients[id]->ChangeState(CLIENT_STAT::CONNECTED, CLIENT_STAT::LOBBY) == false)
	{
		Disconnect(id);
		return;
	}
	gClients[id]->Name = name;
	gClients[id]->SendLoginResult(LOGIN_STAT::ACCEPTED, false);
	
	mLobby.TakeOverNewPlayer(id);
}

void LoginServer::Logout(int id)
{
	mLobby.RevertScene(id, true);
	gClients[id]->SetState(CLIENT_STAT::CONNECTED);

#ifdef USE_DATABASE
	int thread_id = mThreadIdMap[std::this_thread::get_id()];
	mDBHandlers[thread_id].SaveUserInfo(id);
#endif
}

void LoginServer::Disconnect(int id)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Disconnect.\n";
#endif
	Logout(id);
	gClients[id]->Disconnect();
}

void LoginServer::AcceptNewClient(int id, SOCKET sck, sockaddr_in* remote)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Accepted client.\n";
#endif
	gClients[id]->AssignAcceptedID(id, sck, remote);
	
	const EndPoint& hostEp = gClients[id]->GetHostEp();
	std::cout << "IP: " << hostEp.GetIPAddress() << "\n";
	std::cout << "Port: " << hostEp.GetPortNumber() << "\n";

	mUDPReceiver.AssignId(gClients[id]->GetHostEp(), id);

	msIOCP.RegisterDevice(sck, id);
	gClients[id]->RecvMsg();
}

int LoginServer::GetAvailableID()
{
	for (int i = 0; i < gClients.size(); i++)
	{
		if (gClients[i]->ChangeState(CLIENT_STAT::EMPTY, CLIENT_STAT::CONNECTED))
			return i;
	}
	return -1;
}

void LoginServer::ReadRecvBuffer(WSAOVERLAPPEDEX* over, int id, int bytes)
{
	while (over->NetBuffer.Readable())
	{
		std::byte* packet = over->NetBuffer.BufReadPtr();
		CS::PCK_TYPE type = static_cast<CS::PCK_TYPE>(GetPacketType(packet));

		if (packet == nullptr) {
			over->NetBuffer.Clear();
			break;
		}
		if (ProcessPacket(packet, type, id, bytes) == false) {
			over->NetBuffer.Clear();
			break;
		}
	}
}

bool LoginServer::ProcessPacket(std::byte* packet, const CS::PCK_TYPE& type, int id, int bytes)
{
	switch (type)
	{
	case CS::PCK_TYPE::LOGIN:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received login packet\n";
	#endif
		CS::packet_login* pck = reinterpret_cast<CS::packet_login*>(packet);
		
	#ifdef USE_DATABASE
		int thread_id = mThreadIdMap[std::this_thread::get_id()];
		int	conn_id = mDBHandlers[thread_id].SearchIdAndPwd(pck->name, pck->pwd, id);
	#else
		int conn_id = (int)LOGIN_STAT::INVALID_IDPWD;
	#endif

		if(conn_id >= (int)LOGIN_STAT::ACCEPTED)
		{
			if (conn_id >= 0)
			{
				Logout(conn_id);
				gClients[conn_id]->SendForceLogout();
			}
			AcceptLogin(pck->name, id);
		}
		else if (conn_id == (int)LOGIN_STAT::INVALID_IDPWD)
		{
			if (strcmp(pck->name, "GM") == 0) // TEST
			{
				std::string number = std::to_string(id);
				strncat_s(pck->name, number.c_str(), number.size());
				AcceptLogin(pck->name, id);
				break;
			}
			gClients[id]->SendLoginResult(LOGIN_STAT::INVALID_IDPWD);
		}
		break;
	}
	case CS::PCK_TYPE::REGISTER:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received register packet.\n";
	#endif
		CS::packet_register* pck = reinterpret_cast<CS::packet_register*>(packet);

		if (std::string(pck->name).find("GM") != std::string::npos)
		{
			gClients[id]->SendRegisterResult(REGI_STAT::INVALID_IDPWD);
		}
		else
		{
			gClients[id]->SendRegisterResult(REGI_STAT::ACCEPTED);
		}

	#ifdef USE_DATABASE
		int thread_id = mThreadIdMap[std::this_thread::get_id()];
		if (mDBHandlers[thread_id].RegisterIdAndPwd(pck->name, pck->pwd))
			gClients[id]->SendRegisterResult(REGI_STAT::ACCEPTED);
		else
			gClients[id]->SendRegisterResult(REGI_STAT::ALREADY_EXIST);
	#endif
		break;
	}
	case CS::PCK_TYPE::MEASURE_RTT:
	{
		CS::packet_measure_rtt* pck = reinterpret_cast<CS::packet_measure_rtt*>(packet);
		auto id = mUDPReceiver.GetLastReceivedId();
		if (id.has_value())
		{
			gClients[id.value()]->SetLatency(pck->s_send_time);
		}
		break;
	}
	case CS::PCK_TYPE::UDP_CONNECT:
	{
		CS::packet_udp_connection* pck = reinterpret_cast<CS::packet_udp_connection*>(packet);
		mUDPReceiver.PrintLastReceivedEp();
		break;
	}
	default:
		return mLobby.ProcessPacket(packet, type, id, bytes);
	}
	return true;
}

void LoginServer::SignalHandler(int signal)
{
	if (signal == SIGINT)
	{
		std::cout << "Terminating Server..\n";
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
			msIOCP.PostToCompletionQueue(nullptr, -1);
	}
}
