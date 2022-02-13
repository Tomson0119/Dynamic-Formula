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

#include <csignal>
#include <deque>
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