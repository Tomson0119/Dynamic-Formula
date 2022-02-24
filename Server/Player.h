#pragma once

#include "RigidBody.h"
#include "InGameServer.h"

class BtCarShape;

class Player
{
public:
	Player();
	~Player() = default;

	void SetVehicleConstant(std::shared_ptr<InGameServer::VehicleConstant> constantPtr);

	void CreateVehicleRigidBody(
		btScalar mass,
		btDiscreteDynamicsWorld* physicsWorld, 
		BtCarShape* shape);

	void UpdatePlayerRigidBody(float elapsed, btDiscreteDynamicsWorld* physicsWorld);
	void SetDeletionFlag() { mVehicleRigidBody.SetUpdateFlag(RigidBody::UPDATE_FLAG::DELETION); }

	void RemoveRigidBody(btDiscreteDynamicsWorld* physicsWorld);
	void UpdateTransformVectors();

	void ClearVehicleComponent();

	void UpdateVehicleComponent(float elapsed);
	void UpdateSteering(float elapsed);
	void UpdateEngineForce();

	void ToggleKeyValue(uint8_t key, bool pressed);

public:
	void SetPosition(float x, float y, float z);
	const btVector3& GetPosition() const { return mPosition; }
	const btVector4& GetQuaternion() const { return mQuaternion; }

public:
	std::atomic_bool Empty;
	std::atomic_bool Ready;
	std::atomic_int ID;
	char Color;
	char Name[MAX_NAME_SIZE];

	std::atomic_bool LoadDone;

private:
	std::map<int, std::atomic_bool> mKeyMap;

	btVector4 mQuaternion;
	btVector3 mPosition;


	VehicleRigidBody mVehicleRigidBody;
	std::shared_ptr<InGameServer::VehicleConstant> mConstantPtr;
};