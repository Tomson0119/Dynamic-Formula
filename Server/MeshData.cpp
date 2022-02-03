#include "common.h"
#include "MeshData.h"

#include <fstream>

MeshData::MeshData()
	: mPosition{ 0.0f,0.0f,0.0f },
	  mBtRigidBody{ nullptr }
{
}

void CarMesh::Load(std::string_view filepath)
{
	std::ifstream in_file{ filepath.data(), std::ios::binary };

	Vector3 vehicleExtents{}, wheelExtents{};
	in_file >> vehicleExtents.x >> vehicleExtents.y >> vehicleExtents.z;
	in_file >> wheelExtents.x >> wheelExtents.y >> wheelExtents.z;

	/*btCollisionShape* chassisShape = new btBoxShape(
		btVector3(vehicleExtents.x, vehicleExtents.y, vehicleExtents.z));*/

	btTransform btCarTransform{};
	btCarTransform.setIdentity();
	btCarTransform.setOrigin(btVector3(mPosition.x, mPosition.y, mPosition.z));

	//mBtRigidBody = BulletHelper::CreateRigidBody()
}