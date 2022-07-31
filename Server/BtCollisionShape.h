#pragma once

#include "BtShapeInterface.h"

class BtCollisionShape : public BtShapeInterface
{
public:
	virtual ~BtCollisionShape() = default;
	virtual void LoadShapeData(std::string_view filename) override = 0;
	virtual void BuildCollisionShape() = 0;

public:
	const btVector3 GetExtents() const { return mExtents; }
	btCollisionShape& GetCollisionShape() const { return *mCollisionShape.get(); }

protected:
	std::unique_ptr<btCollisionShape> mCollisionShape;
};


class BtBoxShape : public BtCollisionShape
{
public:
	BtBoxShape() = default;
	BtBoxShape(std::string_view filename);
	BtBoxShape(BtBoxShape&& other) noexcept;
	virtual ~BtBoxShape() = default;

	virtual void LoadShapeData(std::string_view filename) override;
	virtual void LoadShapeData(std::ifstream& file);
	virtual void BuildCollisionShape() override;
};

class CheckpointShape : public BtBoxShape
{
public:
	struct CheckpointInfo
	{
		btVector3 position;
		btQuaternion rotation;
	};

public:
	CheckpointShape() = default;
	CheckpointShape(std::string_view filename);
	virtual ~CheckpointShape() = default;

	void LoadCheckpointShapeData(std::string_view filename);
	const std::vector<CheckpointInfo>& GetInfos() const { return mInfos; }

private:
	std::vector<CheckpointInfo> mInfos;
};


//class BtTerrainShape : public BtCollisionShape
//{
//public:
//	BtTerrainShape(std::string_view filename);
//	virtual ~BtTerrainShape();
//
//	virtual void LoadShapeData(std::string_view filename) override;
//	virtual void BuildCollisionShape() override;
//
//	const btVector3& GetOriginPosition() const { return mOriginPosition; }
//
//private:
//	int mMapRow;
//	int mMapCol;
//	float* mHeightMapData;
//
//	btVector3 mTerrainScale;
//	btVector3 mOriginPosition;
//};