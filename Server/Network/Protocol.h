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
struct packet_header 
{
	uint16_t size;
	char type;
};

namespace CS
{
	const char LOGIN		= 1;
	const char REGISTER		= 2;
	const char OPEN_ROOM	= 3;
	const char ENTER_ROOM	= 4;
	const char REVERT_SCENE = 5;
	const char SWITCH_MAP   = 6;
	const char PRESS_READY  = 7;

	struct packet_login : packet_header
	{
		char name[MAX_NAME_SIZE];
		char pwd[MAX_PWD_SIZE];
	};

	struct packet_register : packet_header
	{
		char name[MAX_NAME_SIZE];
		char pwd[MAX_PWD_SIZE];
	};

	struct packet_open_room : packet_header { };

	struct packet_enter_room : packet_header
	{
		int room_id;
	};

	struct packet_revert_scene : packet_header { };

	struct packet_switch_map : packet_header
	{
		char map_id;
		char room_id;
	};

	struct packet_press_ready : packet_header
	{
		char room_id;
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
	const char UPDATE_PLAYER_INFO = 7;
	const char UPDATE_MAP_INFO    = 8;
	const char REMOVE_PLAYER	  = 9;

	struct packet_force_logout : packet_header { };

	struct packet_login_result : packet_header 
	{
		char result;
	};

	struct packet_register_result : packet_header
	{
		char result;
	};

	struct packet_access_room_accept : packet_header
	{
		int room_id;
	};

	struct packet_access_room_deny : packet_header
	{
		char reason;
	};

	struct packet_room_inside_info : packet_header
	{
		char map_id;
		int room_id;		
		PlayerState player_stats[MAX_ROOM_CAPACITY];
	};

	struct packet_room_outside_info : packet_header
	{
		int room_id;
		unsigned char player_count : 4;
		unsigned char map_id : 1;
		bool game_started : 1;
		bool room_closed : 1;
	};

	struct packet_update_player_info : packet_header
	{
		int room_id;
		char index;
		PlayerState player_info;
	};

	struct packet_update_map_info : packet_header
	{
		int room_id;
		char map_id;
	};

	struct packet_remove_player : packet_header
	{
		int room_id;
		char index;
	};
}
#pragma pack(pop)