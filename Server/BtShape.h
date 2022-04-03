#pragma once

class VehicleRigidBody;

class BtShapeBase
{
public:
	BtShapeBase() = default;
	virtual ~BtShapeBase() = default;

	virtual void LoadShapeData(std::string_view filename) = 0;
	virtual void BuildCollisionShape() = 0;
	virtual void LoadConvexHullShape(std::string_view filename);

	const btVector3& GetExtents() const { return mExtents; }
	btCollisionShape* GetCollisionShape() const { return mCollisionShape.get(); }
	btCollisionShape* GetCompoundShape() const { return mCompoundShape.get(); }

	static std::ifstream OpenFile(std::string_view filename);

protected:
	btVector3 mExtents;
	std::unique_ptr<btCollisionShape> mCollisionShape;

	std::vector<std::unique_ptr<btConvexHullShape>> mConvexHullShapes;
	std::unique_ptr<btCompoundShape> mCompoundShape;
};

class BtBoxShape : public BtShapeBase
{
public:
	BtBoxShape(std::string_view filename);
	virtual ~BtBoxShape() = default;

	virtual void LoadShapeData(std::string_view filename) override;
	virtual void BuildCollisionShape() override;
};

class BtCarShape : public BtShapeBase
{
public:
	struct WheelInfo
	{
		btVector3 FrontOffset;
		btVector3 BackOffset;
		btVector3 Extents;
		btVector3 DirectionCS0;
		btVector3 AxleCS;
		btScalar  Friction;
		btScalar  SuspensionStiffness;
		btScalar  SuspensionDamping;
		btScalar  SuspensionCompression;
		btScalar  SuspensionRestLength;
		btScalar  RollInfluence;
		btScalar  ConnectionHeight;
	};

public:
	BtCarShape(std::string_view filename);

	virtual ~BtCarShape() = default;

	virtual void LoadShapeData(std::string_view filename) override;
	virtual void BuildCollisionShape() override;

	const WheelInfo& GetWheelInfo() const { return mWheelInfo; }

private:
	WheelInfo mWheelInfo;
};

class BtTerrainShape : public BtShapeBase
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