#pragma once

const char* const SERVER_IP = "127.0.0.1";
const short SERVER_PORT = 4000;
const short CLIENT_PORT = 4001;

const int MAX_NAME_SIZE = 20;
const int MAX_PWD_SIZE  = 20;

const int MAX_PLAYER_SIZE = 100;
const int MAX_ROOM_CAPACITY = 8;
const int MAX_ROOM_SIZE = MAX_PLAYER_SIZE;

const int ROOM_NUM_PER_PAGE = 6;

const int MaxBufferSize = 1024;
const int MAX_THREAD_COUNT = 6;


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
	uint8_t type;
};

struct vec3
{
	int x;
	int z;
	unsigned short y : 14;
};

struct quat3
{
	uint8_t max_idx : 2;
	short elem1;
	short elem2;
	short elem3;
};

namespace CS
{
	enum class PCK_TYPE : uint8_t
	{
		LOGIN,
		REGISTER,
		INQUIRE_ROOM,
		OPEN_ROOM,
		ENTER_ROOM,
		REVERT_SCENE,
		SWITCH_MAP,
		PRESS_READY,
		LOAD_DONE,
		KEY_INPUT,
		MEASURE_RTT
	};

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

	struct packet_inquire_room : packet_header
	{
		int page_num;
	};

	struct packet_open_room : packet_header { };

	struct packet_enter_room : packet_header
	{
		int room_id;
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
	};

	struct packet_measure_rtt : packet_header
	{
		uint64_t s_send_time;
	};
}

namespace SC
{
	struct player_info
	{
		char name[MAX_NAME_SIZE];
		uint8_t color : 4;
		bool empty : 1;
		bool ready : 1;
	};

	struct room_info
	{
		int room_id;
		uint8_t player_count : 4;
		uint8_t map_id : 1;
		bool game_started : 1;
		bool room_opened : 1;
	};

	enum class PCK_TYPE : uint8_t
	{
		FORCE_LOGOUT,
		LOGIN_RESULT,
		REGISTER_RESULT,
		ACCESS_ROOM_ACCEPT,
		ACCESS_ROOM_DENY,
		ROOM_INSIDE_INFO,
		ROOM_OUTSIDE_INFO,
		UPDATE_PLAYER_INFO,
		UPDATE_MAP_INFO,
		REMOVE_PLAYER,
		GAME_START_FAIL,
		GAME_START_SUCCESS,
		READY_SIGNAL,
		START_SIGNAL,
		PLAYER_TRANSFORM ,
		MISSILE_LAUNCHED,
		MISSILE_TRANSFORM,
		UI_INFO,
		REMOVE_MISSILE,
		INVINCIBLE_ON,
		SPAWN_TRANSFORM,
		WARNING_MESSAGE,
		INGAME_INFO,
		GAME_END,
		ITEM_COUNT,
		MEASURE_RTT
	};

	struct packet_force_logout : packet_header { };

	struct packet_login_result : packet_header 
	{
		char result;
		short port; // temporary, for test in localhost
	};

	struct packet_register_result : packet_header
	{
		char result;
	};

	struct packet_access_room_accept : packet_header
	{
		int room_id;
		bool game_started;
	};

	struct packet_access_room_deny : packet_header
	{
		char reason;
	};

	struct packet_room_inside_info : packet_header
	{
		uint8_t admin_idx : 3;
		uint8_t player_idx : 3;
		uint8_t map_id : 1;
		player_info player_stats[MAX_ROOM_CAPACITY];
	};

	struct packet_room_outside_info : packet_header
	{
		room_info rooms[ROOM_NUM_PER_PAGE];
	};

	struct packet_update_player_info : packet_header
	{
		uint8_t admin_idx : 3;
		uint8_t player_idx : 3;
		player_info player_info;
	};

	struct packet_update_map_info : packet_header
	{
		uint8_t map_id;
	};

	struct packet_remove_player : packet_header
	{
		uint8_t admin_idx : 3;
		uint8_t player_idx : 3;
	};

	struct packet_game_start_fail : packet_header
	{
	};

	struct packet_game_start_success : packet_header
	{
		vec3 positions[MAX_ROOM_CAPACITY];
		quat3 quaternions[MAX_ROOM_CAPACITY];
	};

	struct packet_ready_signal : packet_header
	{
	};

	struct packet_start_signal : packet_header 
	{
		int running_time_sec;
		int delay_time_msec;
	};

	struct packet_player_transform : packet_header
	{
		uint8_t player_idx : 3;
		vec3 position;
		quat3 quaternion;
		int linear_vel[3];
	};

	struct packet_missile_launched : packet_header
	{
		uint8_t missile_idx : 3;
		vec3 position;
		quat3 quaternion;
	};

	struct packet_missile_transform : packet_header
	{
		uint8_t missile_idx : 3;
		int pos_x;
		int pos_z;
		int linear_vel_x;
		int linear_vel_z;
	};

	struct packet_ui_info : packet_header
	{
		uint8_t player_idx;
		int gauge;
		int speed;
	};

	struct packet_remove_missile : packet_header
	{
		uint8_t missile_idx;
	};

	struct packet_invincible_on : packet_header
	{
		uint8_t player_idx;
		int duration;
	};

	struct packet_spawn_transform : packet_header
	{
		uint8_t player_idx;
		vec3 position;
		quat3 quaternion;
	};

	struct packet_warning_message : packet_header { };

	struct packet_ingame_info : packet_header
	{
		uint8_t player_idx : 3;
		uint8_t rank : 4;
		uint8_t lap_count;
		int point;
	};

	struct packet_game_end : packet_header
	{
		uint8_t rank[MAX_ROOM_CAPACITY];
		uint8_t lap_count[MAX_ROOM_CAPACITY];
		uint8_t hit_count[MAX_ROOM_CAPACITY];
		int point[MAX_ROOM_CAPACITY];
	};

	struct packet_item_count : packet_header 
	{
		uint8_t player_idx : 3;
		uint8_t item_count : 2;
	};

	struct packet_measure_rtt : packet_header
	{
		uint64_t s_send_time;
		uint64_t latency;
	};
}
#pragma pack(pop)