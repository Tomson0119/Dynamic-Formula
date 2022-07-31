#pragma once

#include "BtShapeInterface.h"
#include "BtCollisionShape.h"

class BtCompoundShape : public BtShapeInterface
{
public:
	virtual ~BtCompoundShape() = default;
	virtual void LoadShapeData(std::string_view filename) { }
	virtual void BuildCompoundShape(std::string_view filename);

public:
	btCollisionShape& GetCompoundShape() const { return *mCompoundShape.get(); }

protected:
	std::vector<std::unique_ptr<btConvexHullShape>> mConvexHullShapes;
	std::unique_ptr<btCompoundShape> mCompoundShape;
};


class BtCarShape : public BtCompoundShape
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
	BtCarShape(std::string_view dataPath, std::string_view shapePath);
	virtual ~BtCarShape() = default;

	virtual void LoadShapeData(std::string_view filename) override;
	virtual void BuildCompoundShape(std::string_view filename) override;

	const WheelInfo& GetWheelInfo() const { return mWheelInfo; }

private:
	WheelInfo mWheelInfo;
};


class BtMissileShape : public BtCompoundShape
{
public:
	BtMissileShape(std::string_view shapePath);
	virtual ~BtMissileShape() = default;
};


class BtMeshShape
{
public:
	BtMeshShape() = default;
	BtMeshShape(BtMeshShape&& other) noexcept;
	virtual ~BtMeshShape(); 

public:
	void SetLocalScale(const btVector3& localScale) { mMeshShape->setLocalScaling(localScale); }
	void LoadMesh(std::ifstream& file, const std::vector<btVector3>& positions);
	void BuildMeshShape(const std::vector<btVector3>& vertices, const std::vector<uint32_t>& indices);
	btBvhTriangleMeshShape* GetMeshShape() const { return mMeshShape.get(); }

private:
	std::unique_ptr<btTriangleIndexVertexArray> mTriangleVertexArray;
	std::unique_ptr<btBvhTriangleMeshShape> mMeshShape;
};


class BtMapShape : public BtCompoundShape
{
public:
	BtMapShape(std::string_view filename);
	virtual ~BtMapShape() = default;

	virtual void LoadShapeData(std::string_view filename) override { }
	virtual void BuildCompoundShape(std::string_view filename) override;

	void AddCheckpointShape(std::string_view filename);

	const CheckpointShape& GetCheckpointShape() { return mCheckpointShape; }

private:
	void LoadModel(
		std::string_view filename, 
		const btTransform& localTransform,
		const btVector3& localScale);
	void LoadModel(
		std::ifstream& fileStream,
		const btTransform& localTransform,
		const btVector3& localScale);

private:
	std::vector<BtMeshShape> mMeshShapes;
	CheckpointShape mCheckpointShape;
};
