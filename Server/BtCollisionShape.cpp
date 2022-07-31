#include "common.h"
#include "BtCollisionShape.h"


//
//	BtBoxShape
//
BtBoxShape::BtBoxShape(std::string_view filename)
{
	LoadShapeData(filename);
	BuildCollisionShape();
}

BtBoxShape::BtBoxShape(BtBoxShape&& other) noexcept
{
	if (other.mCollisionShape)
	{
		mCollisionShape = std::move(mCollisionShape);
	}
}

void BtBoxShape::LoadShapeData(std::string_view filename)
{
	std::ifstream file = Helper::OpenFile(filename);
	LoadShapeData(file);
}

void BtBoxShape::LoadShapeData(std::ifstream& file)
{
	btScalar x{}, y{}, z{};
	file >> x >> y >> z;
	mExtents.setValue(x, y, z);
}

void BtBoxShape::BuildCollisionShape()
{
	mCollisionShape = std::make_unique<btBoxShape>(mExtents);
}


//
//	BtBoxShape
//
CheckpointShape::CheckpointShape(std::string_view filename)
{
	LoadCheckpointShapeData(filename);
}

void CheckpointShape::LoadCheckpointShapeData(std::string_view filename)
{
	std::ifstream file = Helper::OpenFile(filename);

	LoadShapeData(file);
	BuildCollisionShape();

	float px, py, pz;
	float rx, ry, rz, rw;
	while (file >> px >> py >> pz >>
		rx >> ry >> rz >> rw)
	{
		mInfos.emplace_back();
		mInfos.back().position.setValue(px, py, pz);
		mInfos.back().rotation.setValue(rx, ry, rz, rw);
	}
}


//	BtTerrainShape
//
//BtTerrainShape::BtTerrainShape(std::string_view filename)
//	: mMapRow{ }, mMapCol{ }, mHeightMapData{ }
//{
//	LoadShapeData(filename);
//	BuildCollisionShape();
//}
//
//BtTerrainShape::~BtTerrainShape()
//{
//	if (mHeightMapData) delete[] mHeightMapData;
//}
//
//void BtTerrainShape::LoadShapeData(std::string_view filename)
//{
//	std::ifstream file = Helper::OpenFile(filename);
//
//	btScalar scale_x{}, scale_y{}, scale_z{};
//	file >> mMapRow >> mMapCol;
//	file >> scale_x >> scale_y >> scale_z;
//
//	mTerrainScale.setValue(scale_x, scale_y, scale_z);
//
//	mHeightMapData = new float[mMapRow * mMapCol];
//	for (int i = 0; i < mMapCol; i++)
//	{
//		for (int j = 0; j < mMapRow; j++)
//		{
//			file >> mHeightMapData[i * mMapCol + j];
//		}
//	}
//}
//
//void BtTerrainShape::BuildCollisionShape()
//{
//	auto minmaxHeight = std::minmax_element(mHeightMapData, mHeightMapData + mMapRow * mMapCol,
//		[](float a, float b)
//		{
//			return (a < b);
//		});
//
//	float minHeight = *minmaxHeight.first;
//	float maxHeight = *minmaxHeight.second;
//
//	mCollisionShape = std::make_unique<btHeightfieldTerrainShape>(
//		mMapRow, mMapCol, mHeightMapData,
//		minHeight, maxHeight, 1, false);
//
//	mCollisionShape->setLocalScaling(mTerrainScale);
//
//	mOriginPosition = btVector3(
//		mMapRow * mTerrainScale.x() / 2,
//		(maxHeight + minHeight) * mTerrainScale.y() / 2,
//		mMapCol * mTerrainScale.z() / 2);
//}

