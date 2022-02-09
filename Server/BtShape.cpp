#include "common.h"
#include "BtShape.h"

#include <fstream>


//
//	BtBoxShape
//
BtBoxShape::BtBoxShape(std::string_view filename)
{
	LoadShapeData(filename);
}

void BtBoxShape::LoadShapeData(std::string_view filename)
{
	std::ifstream file{ filename.data(), std::ios::binary };
	
	btScalar x{}, y{}, z{};
	file >> x >> y >> z;

	mExtents.setValue(x, y, z);
}


//
//	BtCarShape
//
BtCarShape::BtCarShape(std::string_view filename)
{
	LoadShapeData(filename);
}

void BtCarShape::LoadShapeData(std::string_view filename)
{
	std::ifstream file{ filename.data(), std::ios::binary };

	btScalar body_x{}, body_y{}, body_z{};
	btScalar wheel_x{}, wheel_y{}, wheel_z{};

	file >> body_x >> body_y >> body_z;
	file >> wheel_x >> wheel_y >> wheel_z;

	mExtents.setValue(body_x, body_y, body_z);
	mWheelExtents.setValue(wheel_x, wheel_y, wheel_z);
}


//
//	BtTerrainShape
//
BtTerrainShape::BtTerrainShape(std::string_view filename)
	: mMapRow{ }, mMapCol{ }, mHeightMapData{ }
{
	LoadShapeData(filename);
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

	std::ofstream testfile{ "test.txt" };
	for (int i = 0; i < mMapCol; i++)
	{
		for (int j = 0; j < mMapRow; j++)
		{
			testfile << mHeightMapData[i * mMapCol + j] << " ";
		}
		testfile << "\n";
	}
}