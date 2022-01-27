#include "common.h"
#include "LoginServer.h"
#include "Client.h"
#include "InGameRoom.h"

std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> gClients;

LoginServer::LoginServer(const EndPoint& ep)
	: mLoop(true)
{
	for (int i = 0; i < MAX_THREADS; i++)
	{
		if (mDBHandlers[i].ConnectToDB(L"sql_server"))
			mDBHandlers[i].ResetAllHost();
		else std::cout << "failed to connect to DB\n";
	}

	mLobby.Init(this);
	
	for (int i = 0; i < gClients.size(); i++)
		gClients[i] = std::make_unique<Client>(i);	

	mListenSck.Init();
	mListenSck.Bind(ep);
}

LoginServer::~LoginServer()
{
}

void LoginServer::Run()
{
	mListenSck.Listen();
	mIOCP.RegisterDevice(mListenSck.GetSocket(), 0);
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
	CompletionInfo info{};
	while (server.mLoop)
	{
		try {
			server.mIOCP.GetCompletionInfo(info);

			int client_id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);
			
			if (over_ex == nullptr || info.success == FALSE)
			{
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
			Disconnect(id);
		delete over;
		break;
	}
	case OP::ACCEPT:
	{
		SOCKET clientSck = *reinterpret_cast<SOCKET*>(over->NetBuffer.BufStartPtr());
		
		int i = GetAvailableID();
		if (i == -1) 
			std::cout << "Session's full\n";
		else {
			AcceptNewClient(i, clientSck);
		}
		mListenSck.AsyncAccept(over);
		break;
	}
	}
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
	
	mLobby.IncreasePlayerCount();
	mLobby.SendExistingRoomList(id);
}

void LoginServer::Logout(int id)
{
	mLobby.TryRemovePlayer(gClients[id]->RoomID, id);
	mLobby.DecreasePlayerCount();

	int thread_id = mThreadIDs[std::this_thread::get_id()];
	mDBHandlers[thread_id].SaveUserInfo(id);
	gClients[id]->SetState(CLIENT_STAT::CONNECTED);
}

void LoginServer::Disconnect(int id)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Disconnect.\n";
#endif
	Logout(id);
	gClients[id]->Disconnect();
}

void LoginServer::AcceptNewClient(int id, SOCKET sck)
{
#ifdef DEBUG_PACKET_TRANSFER
	std::cout << "[" << id << "] Accepted client.\n";
#endif
	gClients[id]->AssignAcceptedID(id, sck);
	mIOCP.RegisterDevice(sck, id);
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
		
		int thread_id = mThreadIDs[std::this_thread::get_id()];
		int	conn_id = mDBHandlers[thread_id].SearchIdAndPwd(pck->name, pck->pwd, id);
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
			break;
		}

		int thread_id = mThreadIDs[std::this_thread::get_id()];
		if (mDBHandlers[thread_id].RegisterIdAndPwd(pck->name, pck->pwd))
			gClients[id]->SendRegisterResult(REGI_STAT::ACCEPTED);
		else
			gClients[id]->SendRegisterResult(REGI_STAT::ALREADY_EXIST);
		break;
	}
	case CS::REVERT_SCENE:
	{
#ifdef DEBUG_PACKET_TRANSFER
		std::cout << "[" << id << "] Received revert scene.\n";
#endif
		auto currentState = gClients[id]->GetCurrentState();
		if (currentState == CLIENT_STAT::LOBBY)
			Logout(id);
		else if (currentState == CLIENT_STAT::IN_ROOM)
		{
			mLobby.TryRemovePlayer(gClients[id]->RoomID, id);
			mLobby.SendExistingRoomList(id);
		}
		break;
	}
	default:
		return mLobby.ProcessPacket(packet, type, id, bytes);
	}
	return true;
}