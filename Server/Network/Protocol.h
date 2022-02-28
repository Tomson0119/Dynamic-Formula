#pragma once

const char* const SERVER_IP = "127.0.0.1";
const short SERVER_PORT = 4000;

const int MAX_NAME_SIZE = 20;
const int MAX_PWD_SIZE  = 20;

const int MAX_PLAYER_SIZE = 100;
const int MAX_ROOM_CAPACITY = 8;
const int MAX_ROOM_SIZE = MAX_PLAYER_SIZE / MAX_ROOM_CAPACITY + 1;

const int MaxBufferSize = 1024;

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
	AVAILABLE = 0, 
	ROOM_IS_CLOSED,
	ROOM_IS_FULL,
	GAME_STARTED,
	MAX_ROOM_REACHED,
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
	const char LOAD_DONE	= 8;
	const char KEY_INPUT	= 9;

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
		//uint64_t send_time;
	};

	struct packet_revert_scene : packet_header { };

	struct packet_switch_map : packet_header
	{
		int room_id;
		char map_id;
	};

	struct packet_press_ready : packet_header
	{
		int room_id;
		//uint64_t send_time;
	};

	struct packet_load_done : packet_header
	{
		int room_id;
	};

	struct packet_key_input : packet_header
	{
		int room_id;
		uint8_t key;
		bool pressed;
		uint64_t send_time;
	};
}

namespace SC
{
	struct PlayerInfo
	{
		char name[MAX_NAME_SIZE];
		uint8_t color : 4;
		bool empty : 1;
		bool ready : 1;
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
	const char GAME_START_FAIL	  = 10;
	const char GAME_START_SUCCESS = 11;
	const char START_SIGNAL		  = 12;
	const char TRANSFER_TIME	  = 13;
	const char PLAYER_TRANSFORM	  = 14;

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
		bool game_started;
		//uint64_t send_time;
	};

	struct packet_access_room_deny : packet_header
	{
		char reason;
		//uint64_t send_time;
	};

	struct packet_room_inside_info : packet_header
	{
		int room_id;
		uint8_t admin_idx : 4;
		uint8_t player_idx : 4;
		uint8_t map_id;
		PlayerInfo player_stats[MAX_ROOM_CAPACITY];
	};

	struct packet_room_outside_info : packet_header
	{
		int room_id;
		uint8_t player_count : 4;
		uint8_t map_id : 1;
		bool game_started : 1;
		bool room_closed : 1;
	};

	struct packet_update_player_info : packet_header
	{
		int room_id;
		uint8_t admin_idx : 4;
		uint8_t player_idx : 4;
		PlayerInfo player_info;
		//uint64_t send_time;
	};

	struct packet_update_map_info : packet_header
	{
		int room_id;
		uint8_t map_id;
	};

	struct packet_remove_player : packet_header
	{
		int room_id;
		uint8_t admin_idx : 4;
		uint8_t player_idx : 4;
	};

	struct packet_game_start_fail : packet_header
	{
		int room_id;
		//uint64_t send_time;
	};

	struct packet_game_start_success : packet_header
	{
		int room_id;
		float x[MAX_ROOM_CAPACITY];
		float y[MAX_ROOM_CAPACITY];
		float z[MAX_ROOM_CAPACITY];
	};

	struct packet_start_signal : packet_header
	{
		int world_id;
	};

	struct packet_transfer_time : packet_header
	{
		uint64_t recv_time;
		uint64_t send_time;
	};

	struct packet_player_transform : packet_header
	{
		int world_id;
		int player_idx;
		float position[3];
		float quaternion[4];
	};
}
#pragma pack(pop)