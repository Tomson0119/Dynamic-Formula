#include "common.h"
#include "LobbyServer.h"
#include "Client.h"
#include "InGameRoom.h"

std::array<std::unique_ptr<Client>, MAX_PLAYER_SIZE> LobbyServer::gClients;
std::array<std::unique_ptr<InGameRoom>, MAX_ROOM_SIZE> LobbyServer::gRooms;

LobbyServer::LobbyServer(const EndPoint& ep)
	: mLoop(true), mRoomCount(0)
{
	if (mDBHandler.ConnectToDB(L"sql_server") == false)
		std::cout << "failed to connect to DB\n";

	for (int i = 0; i < gClients.size(); i++)
		gClients[i] = std::make_unique<Client>(i);

	for (int i = 0; i < gRooms.size(); i++)
		gRooms[i] = std::make_unique<InGameRoom>(i, this);

	mListenSck.Init();
	mListenSck.Bind(ep);
}

LobbyServer::~LobbyServer()
{
}

void LobbyServer::Run()
{
	mListenSck.Listen();
	mIOCP.RegisterDevice(mListenSck.GetSocket(), 0);
	std::cout << "Listening to clients...\n";

	WSAOVERLAPPEDEX acceptEx;
	mListenSck.AsyncAccept(&acceptEx);

	for (int i = 0; i < MaxThreads; i++)
		mThreads.emplace_back(NetworkThreadFunc, std::ref(*this));

	for (std::thread& thrd : mThreads)
		thrd.join();
}

void LobbyServer::NetworkThreadFunc(LobbyServer& server)
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

void LobbyServer::HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes)
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

void LobbyServer::Disconnect(int id)
{
	std::cout << "Disconnect [" << id << "]\n";
	mDBHandler.SaveAndDisconnect(id);
	gClients[id]->Disconnect();
}

void LobbyServer::AcceptNewClient(int id, SOCKET sck)
{
	std::cout << "Accepted client [" << id << "]\n";
	gClients[id]->AssignAcceptedID(id, sck);
	mIOCP.RegisterDevice(sck, id);
	gClients[id]->RecvMsg();
}

void LobbyServer::ReadRecvBuffer(WSAOVERLAPPEDEX* over, int id, int bytes)
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

bool LobbyServer::ProcessPacket(std::byte* packet, char type, int id, int bytes)
{
	switch (type)
	{
	case CS::LOGIN:
	{
		std::cout << "[" << id << "] Login packet\n";
		CS::packet_login* pck = reinterpret_cast<CS::packet_login*>(packet);

		int conn_id = (int)LOGIN_STAT::INVALID_IDPWD;
		if (strcmp(pck->name, "GM") == 0) 
		{
			std::string number = std::to_string(id);
			strncat_s(pck->name, number.c_str(), number.size());
			conn_id = (int)LOGIN_STAT::ACCEPTED;
		}
		else
			conn_id = mDBHandler.SearchIdAndPwd(pck->name, pck->pwd, id);

		if(conn_id >= (int)LOGIN_STAT::ACCEPTED)
		{
			if (conn_id >= 0) Disconnect(conn_id);
			ProcessLoginStep(pck->name, id);
		}
		else if (conn_id == (int)LOGIN_STAT::INVALID_IDPWD)
		{
			gClients[id]->SendLoginResult(LOGIN_STAT::INVALID_IDPWD);
		}
		break;
	}
	case CS::REGISTER:
	{
		CS::packet_register* pck = reinterpret_cast<CS::packet_register*>(packet);
		std::cout << "[" << id << "] Register packet.\n";

		if (std::string(pck->name).find("GM") != std::string::npos)
		{
			gClients[id]->SendRegisterResult(REGI_STAT::INVALID_IDPWD);
			break;
		}

		bool succeeded = mDBHandler.RegisterIdAndPwd(pck->name, pck->pwd);
		if (succeeded)
			gClients[id]->SendRegisterResult(REGI_STAT::ACCEPTED);
		else
			gClients[id]->SendRegisterResult(REGI_STAT::ALREADY_EXIST);
		break;
	}
	case CS::OPEN_ROOM:
	{
		CS::packet_open_room* pck = reinterpret_cast<CS::packet_open_room*>(packet);
		std::cout << "[" << id << "] Open room packet\n";

		if (mRoomCount >= MAX_ROOM_SIZE)
			gClients[id]->SendAccessRoomDeny(ROOM_STAT::MAX_ROOM_REACHED, -1);
		else 
		{
			mRoomCount.fetch_add(1);			
			gRooms[mRoomCount - 1]->OpenRoom(id);
		}

		break;
	}
	case CS::ENTER_ROOM:
	{
		CS::packet_enter_room* pck = reinterpret_cast<CS::packet_enter_room*>(packet);
		std::cout << "[" << id << "] Enter room packet\n";

		if (gRooms[pck->room_id]->Full())
			gClients[id]->SendAccessRoomDeny(ROOM_STAT::ROOM_IS_FULL, -1);
		else
			gRooms[pck->room_id]->AddPlayer(id);
		break;
	}
	default:
	{
		std::cout << "Invalid packet...clearing buffer\n";
		return false;
	}
	}
	return true;
}

int LobbyServer::GetAvailableID()
{
	for (int i = 0; i < gClients.size(); i++)
	{
		if (gClients[i]->ChangeState(CLIENT_STAT::EMPTY, CLIENT_STAT::CONNECTED))
			return i;
	}
	return -1;
}

void LobbyServer::ProcessLoginStep(const char* name, int id)
{
	if (gClients[id]->ChangeState(CLIENT_STAT::CONNECTED, CLIENT_STAT::LOGIN) == false)
	{
		std::cout << "Failed to change state\n";
		Disconnect(id);
		return;
	}
	gClients[id]->Name = name;
	gClients[id]->SendLoginResult(LOGIN_STAT::ACCEPTED, false);
	SendCurrentRoomList(id);
}

void LobbyServer::SendCurrentRoomList(int id)
{
	int k = mRoomCount;
	for (int i = 0; i < MAX_ROOM_SIZE && k > 0; i++)
	{
		if (gRooms[i]->Empty() == false)
		{
			gRooms[i]->SendCurrentRoomInfo(id, false);
			k -= 1;
		}
	}
	gClients[id]->SendMsg();
}
