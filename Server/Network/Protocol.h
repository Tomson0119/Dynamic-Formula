#pragma once

const short SERVER_PORT = 4000;

const int MAX_NAME_SIZE = 20;
const int MAX_PWD_SIZE  = 20;

const int MAX_PLAYER_SIZE = 10000;
const int MAX_ROOM_CAPACITY = 8;
const int MAX_ROOM_SIZE = MAX_PLAYER_SIZE / MAX_ROOM_CAPACITY + 1;

const int MaxBufferSize = 300;

enum class LOGIN_STAT : char
{	
	INVALID_IDPWD = -2,
	ACCEPTED,
};

enum class REGI_STAT : char
{
	INVALID_IDPWD = 0,
	ALREADY_EXIST,
	ACCEPTED
};

enum class ROOM_STAT : char
{
	GAME_STARTED = 0,
	INVALID_ROOM_ID,
	ROOM_IS_FULL,
	ROOM_IS_CLOSED,
	MAX_ROOM_REACHED,
	AVAILABLE
};

#pragma pack (push, 1)
namespace CS
{
	const char LOGIN		= 1;
	const char REGISTER		= 2;
	const char OPEN_ROOM	= 3;
	const char ENTER_ROOM	= 4;
	const char REVERT_SCENE = 5;

	struct packet_login 
	{
		uint16_t size;
		char type;
		char name[MAX_NAME_SIZE];
		char pwd[MAX_PWD_SIZE];
	};

	struct packet_register 
	{
		uint16_t size;
		char type;
		char name[MAX_NAME_SIZE];
		char pwd[MAX_PWD_SIZE];
	};

	struct packet_open_room 
	{
		uint16_t size;
		char type;
	};

	struct packet_enter_room
	{
		uint16_t size;
		char type;
		int room_id;
	};

	struct packet_revert_scene
	{
		uint16_t size;
		char type;
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

	const char FORCE_LOGOUT		  = 0;
	const char LOGIN_RESULT		  = 1;
	const char REGISTER_RESULT	  = 2;
	const char ACCESS_ROOM_ACCEPT = 3;
	const char ACCESS_ROOM_DENY   = 4;
	const char ROOM_INSIDE_INFO   = 5;
	const char ROOM_OUTSIDE_INFO  = 6;

	struct packet_force_logout
	{
		uint16_t size;
		char type;
	};

	struct packet_login_result 
	{
		uint16_t size;
		char type;
		char result;
	};

	struct packet_register_result 
	{
		uint16_t size;
		char type;
		char result;
	};

	struct packet_access_room_accept
	{
		uint16_t size;
		char type;
		int room_id;
	};

	struct packet_access_room_deny
	{
		uint16_t size;
		char type;
		char reason;
	};

	struct packet_room_inside_info 
	{
		uint16_t size;
		char type;
		char map_id;
		int room_id;		
		PlayerState player_stats[MAX_ROOM_CAPACITY];
	};

	struct packet_room_outside_info 
	{
		uint16_t size;
		char type;
		int room_id;
		unsigned char player_count : 4;
		unsigned char map_id : 1;
		bool game_started : 1;
		bool room_closed : 1;
	};
}
#pragma pack(pop)