#pragma once

class BtShapeBase
{
public:
	BtShapeBase() = default;
	virtual ~BtShapeBase() = default;

	virtual void LoadShapeData(std::string_view filename) = 0;

protected:
	btVector3 mExtents;
	std::unique_ptr<btCollisionShape> mCollisionShape;
};

class BtBoxShape : public BtShapeBase
{
public:
	BtBoxShape(std::string_view filename);
	virtual ~BtBoxShape() = default;

	virtual void LoadShapeData(std::string_view filename) override;
};

class BtCarShape : public BtShapeBase
{
public:
	BtCarShape(std::string_view filename);
	virtual ~BtCarShape() = default;

	virtual void LoadShapeData(std::string_view filename) override;

private:
	btVector3 mWheelExtents;
};

class BtTerrainShape : public BtShapeBase
{
public:
	BtTerrainShape(std::string_view filename);
	virtual ~BtTerrainShape();

	virtual void LoadShapeData(std::string_view filename) override;

private:
	int mMapRow;
	int mMapCol;
	float* mHeightMapData;
	btVector3 mTerrainScale;
};