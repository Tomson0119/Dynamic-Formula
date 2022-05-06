#pragma once

#define BT_NO_SIMD_OPERATOR_OVERLOADS
#define NOMINMAX

#include <stdafx.h>
#include <Socket.h>
#include <EndPoint.h>
#include <NetException.h>
#include <BufferQueue.h>
#include <Protocol.h>
#include <IOCP.h>
#include <MemoryPool.h>

#include <fstream>
#include <csignal>
#include <deque>
#include <format>
#include <numeric>
#include <chrono>
#include <concurrent_priority_queue.h>

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#ifndef _DEBUG
#define _DEBUG
#else
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define USE_DATABASE
#define DEBUG_PACKET_TRANSFER

namespace Math
{
	const double PI = 3.14159265359;
}

namespace Helper
{
	inline void Assert(bool expr, const std::string& message)
	{
		if (expr == false)
		{
			std::cout << message << "\n";
			std::abort();
		}
	}

	inline std::ifstream OpenFile(std::string_view filename)
	{
		std::ifstream file{ filename.data(), std::ios::binary };
		Assert(file.is_open(), "Cannot find file name: \"" + std::string(filename) + "\".");
		return file;
	}
}

inline std::ostream& operator<<(std::ostream& os, const btVector3& vec)
{
	os << "[" << vec.x() << ", " << vec.y() << ", " << vec.z() << "]";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const btVector4& vec)
{
	os << "[" << vec.x() << ", " << vec.y() << ", " << vec.z() << ", " << vec.w() << "]";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const btQuaternion& quat)
{
	os << "[" << quat.x() << ", " << quat.y() << ", " << quat.z() << ", " << quat.w() << "]";
	return os;
}