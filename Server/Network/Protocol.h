#pragma once

const short SERVER_PORT = 4000;

const int MAX_NAME_SIZE  = 20;
const int MAX_PWD_SIZE = 40;

const int MAX_PLAYER_SIZE = 10000;
const int MAX_ROOM_CAPACITY = 8;
const int MAX_ROOM_SIZE = MAX_PLAYER_SIZE / MAX_ROOM_CAPACITY + 1;

const int MaxBufferSize = 300;

enum class LOGIN_STAT : char
{
	ACCEPTED = 0,
	INVALID_ID,
	ALREADY_IN,
	BANNED
};

enum class ROOM_STAT : char
{
	GAME_STARTED = 0,
	ROOM_IS_FULL,
	MAX_ROOM_REACHED
};

#pragma pack (push, 1)
namespace CS
{
	const char LOGIN = 1;
	const char ENTER_ROOM = 2;
	const char MOVE = 3;
	const char CHAT = 4;

	struct packet_login {
		unsigned char size;
		char type;
		char name[MAX_NAME_SIZE];
		char pwd[MAX_PWD_SIZE];
	};

	struct packet_enter_room {
		unsigned char size;
		char type;
		bool new_room;
		int room_id;
	};
}

namespace SC
{
	struct PlayerState
	{
		char name[MAX_NAME_SIZE];
		bool empty;
		bool ready;
		char color;
	};

	const char LOGIN_RESULT = 1;
	const char ENTER_ROOM_ACCEPT = 2;
	const char ENTER_ROOM_DENY = 3;
	const char MOVE = 4;
	const char CHAT = 5;

	struct packet_login_result {
		unsigned char size;
		char type;
		char result;
	};

	struct packet_enter_room_accept {
		unsigned char size;
		char type;
		PlayerState player_stats[MAX_ROOM_CAPACITY];
	};

	struct packet_enter_room_deny {
		unsigned char size;
		char type;
		char reason;
	};
}
#pragma pack(pop)