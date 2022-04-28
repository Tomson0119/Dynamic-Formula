#include "common.h"
#include "LoginServer.h"
#include "InGameServer.h"
#include "Client.h"

std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> gClients;
IOCP LoginServer::msIOCP;

LoginServer::LoginServer(const EndPoint& ep)
{
	std::signal(SIGINT, SignalHandler);

	for (int i = 0; i < MAX_THREADS; i++)
	{
	#ifdef USE_DATABASE
		if (mDBHandlers[i].ConnectToDB(L"sql_server"))
			mDBHandlers[i].ResetAllHost();
		else std::cout << "failed to connect to DB\n";
	#endif
	}

	mLobby.Init(this);
	
	mUDPSck = std::make_unique<Socket>();
	mUDPSck->Init(SocketType::UDP);
	mUDPSck->Bind(ep);

	for (int i = 0; i < gClients.size(); i++)
		gClients[i] = std::make_unique<Client>(i, mUDPSck.get());

	mListenSck.Init(SocketType::TCP);
	mListenSck.Bind(ep);
}

LoginServer::~LoginServer()
{
	for (int i = 0; i < gClients.size(); i++)
	{
		if (gClients[i]->ID >= 0)
		{
			Logout(i);
			gClients[i]->Disconnect();
		}
	}
}

void LoginServer::Run()
{
	mListenSck.Listen();
	msIOCP.RegisterDevice(mListenSck.GetSocket(), 0);
	msIOCP.RegisterDevice(mUDPSck->GetSocket(), MAX_PLAYER_SIZE);
	std::cout << "Listening to clients...\n";

	WSAOVERLAPPEDEX acceptEx;
	mListenSck.AsyncAccept(&acceptEx);

	for (int i = 0; i < MAX_THREADS; i++)
	{
		mThreads.emplace_back(NetworkThreadFunc, std::ref(*this));
		mThreadIDs[mThreads.back().get_id()] = i;
	}
	for (std::thread& thrd : mThreads)
		thrd.join();
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
		if (bytes == 0)
		{
			Disconnect(id);
			break;
		}
		Client* client = gClients[id].get();
		over->NetBuffer.ShiftWritePtr(bytes);
		ReadRecvBuffer(over, id, bytes);
		client->RecvMsg();
		break;
	}
	case OP::SEND:
	{
		if (bytes != over->WSABuffer.len)
		{
			Disconnect(id);
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
		else {
			AcceptNewClient(i, clientSck, remote);
		}
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
	int thread_id = mThreadIDs[std::this_thread::get_id()];
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
		char type = GetPacketType(packet);

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

bool LoginServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	case CS::LOGIN:
	{
	#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received login packet\n";
	#endif
		CS::packet_login* pck = reinterpret_cast<CS::packet_login*>(packet);
		
	#ifdef USE_DATABASE
		int thread_id = mThreadIDs[std::this_thread::get_id()];
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
	case CS::REGISTER:
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
		int thread_id = mThreadIDs[std::this_thread::get_id()];
		if (mDBHandlers[thread_id].RegisterIdAndPwd(pck->name, pck->pwd))
			gClients[id]->SendRegisterResult(REGI_STAT::ACCEPTED);
		else
			gClients[id]->SendRegisterResult(REGI_STAT::ALREADY_EXIST);
	#endif
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
		for(int i=0;i<MAX_THREADS;i++)
			msIOCP.PostToCompletionQueue(nullptr, -1);
	}
}
