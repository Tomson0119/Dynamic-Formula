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
	BtBoxShape(std::string_view filename);
	virtual ~BtBoxShape() = default;

	virtual void LoadShapeData(std::string_view filename) override;
	virtual void BuildCollisionShape() override;
};


class BtTerrainShape : public BtCollisionShape
{
public:
	BtTerrainShape(std::string_view filename);
	virtual ~BtTerrainShape();

	virtual void LoadShapeData(std::string_view filename) override;
	virtual void BuildCollisionShape() override;

	const btVector3& GetOriginPosition() const { return mOriginPosition; }

private:
	int mMapRow;
	int mMapCol;
	float* mHeightMapData;

	btVector3 mTerrainScale;
	btVector3 mOriginPosition;
};