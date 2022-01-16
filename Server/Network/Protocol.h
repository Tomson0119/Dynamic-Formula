#pragma once

const short SERVER_PORT = 4000;

const int  MAX_NAME_SIZE = 20;
const int  MAX_CHAT_SIZE = 100;
const int  MAX_PLAYER = 10000;

const int MaxBufferSize = 300;

#pragma pack (push, 1)
namespace CS
{
	const char LOGIN = 1;
	const char MOVE = 2;
	const char CHAT = 3;

	struct login_packet {
		unsigned char size;
		char	type;
		char	name[MAX_NAME_SIZE];
	};

	struct move_packet {
		unsigned char size;
		char	type;
		char	direction;
		int		move_time;
	};

	struct chat_packet {
		unsigned char size;
		char	type;
		char	message[MAX_CHAT_SIZE];
	};
}

namespace SC
{
	const char LOGIN_OK = 1;
	const char MOVE = 2;
	const char CHAT = 3;
	const char LOGIN_FAIL = 4;

	struct packet_login_ok {
		unsigned char size;
		char type;
		int	 id;
	};

	struct packet_login_fail {
		unsigned char size;
		char type;
		char reason;
	};

	struct packet_move {
		unsigned char size;
		char  type;
		int	  id;
		short x, y;
		int	  move_time;
	};

	struct packet_chat {
		unsigned char size;
		char type;
		int  id;
		char message[MAX_CHAT_SIZE];
	};
}
#pragma pack(pop)