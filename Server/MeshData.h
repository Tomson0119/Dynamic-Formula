#pragma once

struct Vector3
{
	float x;
	float y;
	float z;
};

class MeshData
{
public:
	MeshData();
	virtual ~MeshData() = default;

	virtual void Load(std::string_view filepath) = 0;
	virtual void BuildRigidBody(btDynamicsWorld* dynamicsWorld) = 0;

protected:
	Vector3 mPosition;
	btRigidBody* mBtRigidBody;
};

class CarMesh : public MeshData
{
public:
	CarMesh() = default;
	virtual ~CarMesh() = default;

	virtual void Load(std::string_view filepath) override;
	virtual void BuildRigidBody(btDynamicsWorld* dynamicsWorld) override;

private:
	btRaycastVehicle::btVehicleTuning mTuning;
	std::unique_ptr<btVehicleRaycaster> mVehicleRayCaster;
	std::unique_ptr<btRaycastVehicle> mVehicle;
};

class TerrainMesh : public MeshData
{
public:
	TerrainMesh() = default;
	virtual ~TerrainMesh() = default;

private:
	btScalar* mHeightmapData;
	std::unique_ptr<btHeightfieldTerrainShape> mTerrainShape;
};

class ObjectMesh : public MeshData
{
public:
	ObjectMesh() = default;
	virtual ~ObjectMesh() = default;
};

class MissileMesh : public MeshData
{
public:
	MissileMesh() = default;
	virtual ~MissileMesh() = default;
};