#pragma once

#include "BtShapeInterface.h"

class BtCompoundShape : public BtShapeInterface
{
public:
	virtual ~BtCompoundShape() = default;
	virtual void LoadShapeData(std::string_view filename) override = 0;
	virtual void BuildCompoundShape(std::string_view filename) = 0;

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


class BtMeshShape
{
public:
	BtMeshShape(std::string_view filename);
	BtMeshShape(BtMeshShape&& other) noexcept;
	virtual ~BtMeshShape(); 

	void LoadModel(std::string_view filename);
	void LoadMesh(std::ifstream& file, const std::vector<btVector3>& positions);
	void BuildMeshShape(const std::vector<btVector3> vertices, const std::vector<uint16_t> indices);

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

private:
	std::vector<BtMeshShape> mMeshShapes;
};
