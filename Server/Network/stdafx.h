#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

extern "C" {
#include "include/lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}
#pragma comment (lib, "lua54.lib")

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>
#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <unordered_set>

#include "NetException.h"
#include "Protocol.h"

#define ABS(x) ((x < 0) ? -(x) : (x))

using namespace std::chrono_literals;

inline char GetPacketType(std::byte* pck)
{
	return *reinterpret_cast<char*>(pck + sizeof(uint16_t));
}

inline std::wstring CharToWString(const char* str)
{
	std::string s_str(str);
	int size = MultiByteToWideChar(CP_ACP, 0, &s_str[0], (int)s_str.size(), NULL, 0);
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_ACP, 0, &s_str[0], (int)s_str.size(), &wstr[0], size);
	return wstr;
}