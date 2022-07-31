#include "common.h"
#include "BtCompoundShape.h"
#include "RigidBody.h"


//
//	BtCompoundShape
//
void BtCompoundShape::BuildCompoundShape(std::string_view filename)
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
// BtMissileShape
//
BtMissileShape::BtMissileShape(std::string_view shapePath)
{
	BuildCompoundShape(shapePath);
}


//
// BtMeshShape
// 
BtMeshShape::BtMeshShape(BtMeshShape&& other) noexcept
	: mTriangleVertexArray{ nullptr }
{
	if (other.mMeshShape)
	{
		mMeshShape = std::move(other.mMeshShape);
	}
	if (other.mTriangleVertexArray)
	{
		mTriangleVertexArray = std::move(other.mTriangleVertexArray);
	}
}

BtMeshShape::~BtMeshShape()
{
	if (mTriangleVertexArray)
	{
		btIndexedMesh& mesh = mTriangleVertexArray->getIndexedMeshArray()[0];

		delete[] mesh.m_vertexBase;
		delete[] mesh.m_triangleIndexBase;
	}
}

void BtMeshShape::LoadMesh(std::ifstream& file, const std::vector<btVector3>& positions)
{
	std::vector<std::vector<int>> temp;

	std::streampos last = 0;
	std::string info;
	while (std::getline(file, info))
	{
		std::stringstream ss(info);
		std::string type;
		ss >> type;

		if (type == "f")
		{
			char ignore[2];
			int v, vt, vn;

			temp.emplace_back();
			while (ss >> v >> ignore[0] >> vt >> ignore[1] >> vn)
			{
				temp.back().push_back(v-1);
			}
			last = file.tellg();
		}
		else if (type == "usemtl")
		{
			file.seekg(last);
			break;
		}
	}

	std::vector<btVector3> vertices;
	std::vector<uint32_t> indices;
	uint32_t k = 0;
	for (const std::vector<int>& face : temp)
	{
		for (int i = 0; i < face.size(); i++)
		{
			vertices.push_back(positions[face[i]]);
			if (i > 0 && indices.size() % 3 == 0)
			{
				indices.push_back(*(indices.end() - 3));
				indices.push_back(*(indices.end() - 2));
			}
			indices.push_back(k++);
		}
	}

	BuildMeshShape(vertices, indices);
}

void BtMeshShape::BuildMeshShape(const std::vector<btVector3>& vertices, const std::vector<uint32_t>& indices)
{
	mTriangleVertexArray = std::make_unique<btTriangleIndexVertexArray>();

	btIndexedMesh tempMesh;
	mTriangleVertexArray->addIndexedMesh(tempMesh, PHY_FLOAT);

	btIndexedMesh& mesh = mTriangleVertexArray->getIndexedMeshArray()[0];

	const int32_t VERTICES_PER_TRIANGLE = 3;
	size_t numIndices = indices.size();
	mesh.m_numTriangles = (int)numIndices / VERTICES_PER_TRIANGLE;
	if (numIndices < std::numeric_limits<int16_t>::max())
	{
		mesh.m_triangleIndexBase = new unsigned char[sizeof(int16_t) * (size_t)numIndices];
		mesh.m_indexType = PHY_SHORT;
		mesh.m_triangleIndexStride = VERTICES_PER_TRIANGLE * sizeof(int16_t);
	}
	else
	{
		mesh.m_triangleIndexBase = new unsigned char[sizeof(int32_t) * (size_t)numIndices];
		mesh.m_indexType = PHY_INTEGER;
		mesh.m_triangleIndexStride = VERTICES_PER_TRIANGLE * sizeof(int32_t);
	}
	mesh.m_numVertices = (int)vertices.size();
	mesh.m_vertexBase = new unsigned char[VERTICES_PER_TRIANGLE * sizeof(btScalar) * (size_t)mesh.m_numVertices];
	mesh.m_vertexStride = VERTICES_PER_TRIANGLE * sizeof(btScalar);
	btScalar* vertexData = static_cast<btScalar*>((void*)(mesh.m_vertexBase));
	for (int32_t i = 0; i < mesh.m_numVertices; ++i)
	{
		int32_t j = i * VERTICES_PER_TRIANGLE;
		const btVector3& point = vertices[i];
		vertexData[j] = point.x();
		vertexData[j + 1] = point.y();
		vertexData[j + 2] = point.z();
	}
	if (numIndices < std::numeric_limits<int16_t>::max())
	{
		int16_t* indexBase = static_cast<int16_t*>((void*)(mesh.m_triangleIndexBase));
		for (int32_t i = 0; i < numIndices; ++i) {
			indexBase[i] = (int16_t)indices[i];
		}
	}
	else
	{
		int32_t* indexBase = static_cast<int32_t*>((void*)(mesh.m_triangleIndexBase));
		for (int32_t i = 0; i < numIndices; ++i)
		{
			indexBase[i] = indices[i];
		}
	}

	const bool USE_QUANTIZED_AABB_COMPRESSION = true;
	mMeshShape = std::make_unique<btBvhTriangleMeshShape>(mTriangleVertexArray.get(), USE_QUANTIZED_AABB_COMPRESSION);
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

	mCompoundShape = std::make_unique<btCompoundShape>();

	std::string info;	
	while (std::getline(file, info))
	{
		std::stringstream ss(info);

		std::string objName;
		ss >> objName;

		float x, y, z, w;

		btVector3 pos;
		ss >> x >> y >> z;
		pos.setValue(x, y, z);

		btQuaternion quat;
		ss >> x >> y >> z >> w;
		quat.setValue(x, y, z, w);

		btVector3 scale;
		ss >> x >> y >> z;
		scale.setValue(x, y, z);

		btTransform localTransform = btTransform::getIdentity();
		localTransform.setOrigin(pos);
		localTransform.setRotation(quat);

		std::string objPath1 = "Resource\\Models\\" + objName + ".obj";
		std::string objPath2 = "Resource\\Models\\" + objName + "_Transparent.obj";

		LoadModel(objPath1, localTransform, scale);
		
		std::ifstream transFile{ objPath2, std::ios::binary };
		if(transFile.is_open()) LoadModel(transFile, localTransform, scale);
	}
}

void BtMapShape::AddCheckpointShape(std::string_view filename)
{
	mCheckpointShape.LoadCheckpointShapeData(filename);
}

void BtMapShape::LoadModel(
	std::string_view filename, 
	const btTransform& localTransform,
	const btVector3& localScale)
{
	std::ifstream file = Helper::OpenFile(filename);
	LoadModel(file, localTransform, localScale);
}

void BtMapShape::LoadModel(std::ifstream& fileStream, const btTransform& localTransform, const btVector3& localScale)
{
	std::vector<btVector3> positions;

	std::string info;
	while (std::getline(fileStream, info))
	{
		std::stringstream ss(info);
		std::string type;
		ss >> type;

		if (type == "v")
		{
			float x, y, z;
			ss >> x >> y >> z;
			z *= -1.0f;
			positions.push_back({ x,y,z });
		}
		else if (type == "usemtl")
		{
			mMeshShapes.emplace_back();
			mMeshShapes.back().LoadMesh(fileStream, positions);
			mMeshShapes.back().SetLocalScale(localScale);
			mCompoundShape->addChildShape(localTransform, mMeshShapes.back().GetMeshShape());
		}
	}
}