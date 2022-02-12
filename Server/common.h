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

#include <deque>
#include <chrono>
#include <concurrent_priority_queue.h>

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#define USE_DATABASE
#define DEBUG_PACKET_TRANSFER

namespace BulletHelper
{
	const float PI = 3.1415926535f;
	inline btRigidBody* CreateRigidBody(btScalar mass, const btTransform& startTransform, btCollisionShape* shape, std::shared_ptr<btDiscreteDynamicsWorld> pbtDynamicsWorld)
	{
		btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic && shape)
			shape->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

		btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

		btRigidBody* body = new btRigidBody(cInfo);

		pbtDynamicsWorld->addRigidBody(body);
		return body;
	}

	inline float RadianToEuler(float fRadian)
	{
		return fRadian / 360 * (float)(2 * PI);
	}
}