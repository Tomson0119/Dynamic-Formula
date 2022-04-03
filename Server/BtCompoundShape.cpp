#include "common.h"
#include "BtCompoundShape.h"
#include "RigidBody.h"


//
//	BtCarShape
//
BtCarShape::BtCarShape(std::string_view dataPath, std::string_view shapePath)
	: mWheelInfo{ }
{
	mWheelInfo.DirectionCS0 = { 0, -1, 0 };
	mWheelInfo.AxleCS = { -1, 0, 0 };

	LoadShapeData(dataPath);
	BuildCompoundShape(shapePath);
}

void BtCarShape::LoadShapeData(std::string_view filename)
{
	std::ifstream file = Helper::OpenFile(filename);

	btScalar x, y, z;
	std::string type;
	while (file >> type)
	{
		if (type == "BodyExtents")
		{
			file >> x >> y >> z;
			mExtents.setValue(x, y, z);
		}
		else if (type == "WheelExtents")
		{
			file >> x >> y >> z;
			mWheelInfo.Extents.setValue(x, y, z);
		}
		else if (type == "FrontOffset")
		{
			file >> x >> y >> z;
			mWheelInfo.FrontOffset.setValue(x, y, z);
		}
		else if (type == "BackOffset")
		{
			file >> x >> y >> z;
			mWheelInfo.BackOffset.setValue(x, y, z);
		}
		else if (type == "Friction")
		{
			file >> x;
			mWheelInfo.Friction = x;
		}
		else if (type == "SuspensionStiffness")
		{
			file >> x;
			mWheelInfo.SuspensionStiffness = x;
		}
		else if (type == "SuspensionDamping")
		{
			file >> x;
			mWheelInfo.SuspensionDamping = x;
		}
		else if (type == "SuspensionCompression")
		{
			file >> x;
			mWheelInfo.SuspensionCompression = x;
		}
		else if (type == "SuspensionRestLength")
		{
			file >> x;
			mWheelInfo.SuspensionRestLength = x;
		}
		else if (type == "RollInfluence")
		{
			file >> x;
			mWheelInfo.RollInfluence = x;
		}
		else if (type == "ConnectionHeight")
		{
			file >> x;
			mWheelInfo.ConnectionHeight = x;
		}
	}
}

void BtCarShape::BuildCompoundShape(std::string_view filename)
{
	std::ifstream in_file = Helper::OpenFile(filename);

	mCompoundShape = std::make_unique<btCompoundShape>();

	std::vector<btVector3> positions;
	std::string info;
	while (std::getline(in_file, info))
	{
		std::stringstream ss(info);
		std::string type;

		ss >> type;

		if (type == "v")
		{
			float x, y, z;
			ss >> x >> y >> z;
			z *= -1.0f;

			positions.push_back(btVector3{ x,y,z });
		}
		else if (type == "s")
		{
			auto convexHull = std::make_unique<btConvexHullShape>();

			for (int i = 0; i < positions.size(); ++i)
				convexHull->addPoint(positions[i]);

			positions.clear();

			btTransform localTransform;
			localTransform.setIdentity();
			localTransform.setOrigin(btVector3(0, 0, 0));

			mCompoundShape->addChildShape(localTransform, convexHull.get());
			mConvexHullShapes.push_back(std::move(convexHull));
		}
	}
}


//
// BtMeshShape
// 
BtMeshShape::BtMeshShape(std::string_view)
{
}

BtMeshShape::~BtMeshShape()
{
}

void BtMeshShape::LoadMesh(std::string_view filename)
{
}

void BtMeshShape::BuildMeshShape(const std::vector<btVector3> position, const std::vector<uint16_t> indices)
{
}


//
// BtMapShape
// 
BtMapShape::BtMapShape(std::string_view filename)
{
	BuildCompoundShape(filename);
}

void BtMapShape::BuildCompoundShape(std::string_view filename)
{
	std::ifstream file = Helper::OpenFile(filename);

	std::string info, objName;
	btVector3 pos;
	btVector4 quat;
	btVector3 scale;
	float x, y, z, w;
	while (std::getline(file, info))
	{
		std::stringstream ss(info);

		ss >> objName;

		ss >> x >> y >> z;
		pos.setValue(x, y, z);

		ss >> x >> y >> z >> w;
		quat.setValue(x, y, z, w);

		ss >> x >> y >> z;
		scale.setValue(x, y, z);

		std::string objPath1 = "Models\\" + objName + ".obj";
		std::string objPath2 = "Models\\" + objName + "_Transparent.obj";
	}
}