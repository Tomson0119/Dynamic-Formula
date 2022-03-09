#include "common.h"
#include "BtShape.h"
#include "RigidBody.h"

#include <fstream>


//
//	BtBoxShape
//
void BtShapeBase::LoadConvexHullShape(std::string_view filename)
{
	std::ifstream in_file{ filename.data(), std::ios::binary };

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
//	BtBoxShape
//
BtBoxShape::BtBoxShape(std::string_view filename)
{
	LoadShapeData(filename);
	BuildCollisionShape();
}

void BtBoxShape::LoadShapeData(std::string_view filename)
{
	std::ifstream file{ filename.data(), std::ios::binary };
	
	btScalar x{}, y{}, z{};
	file >> x >> y >> z;

	mExtents.setValue(x, y, z);
}

void BtBoxShape::BuildCollisionShape()
{
	//mCollisionShape = std::make_unique<btBoxShape>(mExtents);
}


//
//	BtCarShape
//
BtCarShape::BtCarShape(std::string_view filename)
	: mWheelInfo{ }
{
	mWheelInfo.DirectionCS0 = { 0, -1, 0 };
	mWheelInfo.AxleCS = { -1, 0, 0 };

	LoadShapeData(filename);
	BuildCollisionShape();
}

void BtCarShape::LoadShapeData(std::string_view filename)
{
	std::ifstream file{ filename.data(), std::ios::binary };

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

void BtCarShape::BuildCollisionShape()
{
	/*mCollisionShape = std::make_unique<btBoxShape>(mExtents);*/
}

//
//	BtTerrainShape
//
BtTerrainShape::BtTerrainShape(std::string_view filename)
	: mMapRow{ }, mMapCol{ }, mHeightMapData{ }
{
	LoadShapeData(filename);
	BuildCollisionShape();
}

BtTerrainShape::~BtTerrainShape()
{
	if (mHeightMapData) delete[] mHeightMapData;
}

void BtTerrainShape::LoadShapeData(std::string_view filename)
{
	std::ifstream file{ filename.data(), std::ios::binary };

	btScalar scale_x{}, scale_y{}, scale_z{};
	file >> mMapRow >> mMapCol;
	file >> scale_x >> scale_y >> scale_z;

	mTerrainScale.setValue(scale_x, scale_y, scale_z);
	
	mHeightMapData = new float[mMapRow * mMapCol];
	for (int i = 0; i < mMapCol; i++)
	{
		for (int j = 0; j < mMapRow; j++)
		{
			file >> mHeightMapData[i * mMapCol + j];
		}
	}
}

void BtTerrainShape::BuildCollisionShape()
{
	auto minmaxHeight = std::minmax_element(mHeightMapData, mHeightMapData + mMapRow * mMapCol,
		[](float a, float b)
		{
			return (a < b);
		});

	float minHeight = *minmaxHeight.first;
	float maxHeight = *minmaxHeight.second;

	mCollisionShape = std::make_unique<btHeightfieldTerrainShape>(
		mMapRow, mMapCol, mHeightMapData,
		minHeight, maxHeight, 1, false);

	mCollisionShape->setLocalScaling(mTerrainScale);

	mOriginPosition = btVector3(
		mMapRow * mTerrainScale.x() / 2,
		(maxHeight + minHeight) * mTerrainScale.y() / 2,
		mMapCol * mTerrainScale.z() / 2);
}