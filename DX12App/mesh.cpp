#include "stdafx.h"
#include "mesh.h"
#include "texture.h"

Mesh::Mesh()
{
	mMaterial.Mat.Ambient = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mMaterial.Mat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMaterial.Mat.Specular = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mMaterial.Mat.Emission = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mMaterial.Mat.Exponent = 100.0f;
	mMaterial.Mat.IOR = 1.45f;
	mMaterial.SrvIndex = 0;
}

Mesh::Mesh(const std::string& name)
	: Mesh()
{
	mName = name;
}

void Mesh::CreateResourceInfo(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList,
	UINT vbStride, UINT ibStride,
	D3D12_PRIMITIVE_TOPOLOGY topology,
	const void* vbData, UINT vbCount, 
	const void* ibData, UINT ibCount)
{
	mPrimitiveTopology = topology;

	const UINT vbByteSize = vbCount * vbStride;
	
	mVertexBufferGPU = CreateBufferResource(device, cmdList,
		vbData, vbByteSize, mVertexUploadBuffer);

	mVertexBufferView.BufferLocation = mVertexBufferGPU->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = vbByteSize;
	mVertexBufferView.StrideInBytes = vbStride;

	mVerticesCount = vbCount;
	mVertexStride = vbStride;

	if (ibCount > 0)
	{
		mIndexCount = ibCount;
		mStartIndex = 0;
		mBaseVertex = 0;
		mSlot = 0;

		const UINT ibByteSize = ibCount * ibStride;

		mIndexBufferGPU = CreateBufferResource(device, cmdList,
			ibData, ibByteSize, mIndexUploadBuffer);

		mIndexBufferView.BufferLocation = mIndexBufferGPU->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = ibByteSize;
		mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	}
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList, bool isSO)
{
	cmdList->IASetVertexBuffers(mSlot, 1, &mVertexBufferView);	
	cmdList->IASetPrimitiveTopology(mPrimitiveTopology);

	if (mIndexCount > 0) 
	{
		cmdList->IASetIndexBuffer(&mIndexBufferView);
		cmdList->DrawIndexedInstanced(mIndexCount, 1, mStartIndex, mBaseVertex, 0);
	}
	else
	{
		cmdList->DrawInstanced(mVerticesCount, 1, mBaseVertex, 0);
	}
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList, const BoundingFrustum& viewFrustum, bool isSO)
{
	if (viewFrustum.Intersects(mOOBB))
	{
		cmdList->IASetVertexBuffers(mSlot, 1, &mVertexBufferView);
		cmdList->IASetPrimitiveTopology(mPrimitiveTopology);

		if (mIndexCount > 0)
		{
			cmdList->IASetIndexBuffer(&mIndexBufferView);
			cmdList->DrawIndexedInstanced(mIndexCount, 1, mStartIndex, mBaseVertex, 0);
		}
		else
		{
			cmdList->DrawInstanced(mVerticesCount, 1, mBaseVertex, 0);
		}
	}
}

void Mesh::LoadMesh(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList,
	std::ifstream& file,
	const std::vector<XMFLOAT3>& positions,
	const std::vector<XMFLOAT3>& normals,
	const std::vector<XMFLOAT2>& texcoords,
	const MatInfo& mat)
{
	mMaterial = mat;

	struct UINT3
	{
		UINT vertIndex;
		UINT normIndex;
		UINT texIndex;
	};
	std::vector<std::vector<UINT3>> temp_indices;

	int last_pos = 0;
	std::string info;
	while (std::getline(file, info))
	{		
		std::stringstream ss(info);
		std::string type;

		ss >> type;

		if (type == "f")
		{
			char ignore[2];
			UINT v, vt, vn;

			temp_indices.push_back({});
			while (ss >> v >> ignore[0] >> vt >> ignore[1] >> vn)
			{
				temp_indices.back().push_back({ v - 1, vn - 1, vt - 1 });
			}
			last_pos = (int)file.tellg();
		}
		else if (type == "usemtl") 
		{
			file.seekg(last_pos);
			break;
		}
	}

	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	UINT k = 0;
	for (const std::vector<UINT3>& curr_face : temp_indices)
	{
		for (int i = 0; i < curr_face.size(); i++)
		{
			Vertex v;
			v.Position = positions[curr_face[i].vertIndex];
			v.Normal = normals[curr_face[i].normIndex];
			v.TexCoord = texcoords[curr_face[i].texIndex];
			v.TexCoord.x /= mat.TexScale.x;
			v.TexCoord.x += mat.TexOffset.x;
			v.TexCoord.y /= mat.TexScale.y;
			v.TexCoord.y += mat.TexOffset.y;
			v.TexCoord.y = 1.0f - v.TexCoord.y;
			
			vertices.push_back(v);

			if (i > 0 && indices.size() % 3 == 0)
			{
				indices.push_back(*(indices.end() - 3));
				indices.push_back(*(indices.end() - 2));
			}
			indices.push_back(k++);
		}
	}
	std::reverse(indices.begin(), indices.end());
	Mesh::CreateResourceInfo(device, cmdList, sizeof(Vertex), sizeof(UINT),
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());
}

MaterialConstants Mesh::GetMaterialConstant() const
{
	MaterialConstants matCnst{};
	matCnst.Mat = mMaterial.Mat;
	matCnst.TexTransform = mMaterial.TexTransform;
	return matCnst;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//
BoxMesh::BoxMesh(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	float width, float height, float depth)
	: Mesh()
{
	float hx = width * 0.5f, hy = height * 0.5f, hz = depth * 0.5f;

	mOOBB.Center = { 0.0f,0.0f,0.0f };
	mOOBB.Extents = { hx,hy,hz };
	mOOBB.Orientation = { 0.0f,0.0f,0.0f,1.0f };

	std::array<Vertex, 24> vertices =
	{
		// Front
		Vertex(-hx, +hy, -hz, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f),  // 0
		Vertex(+hx, +hy, -hz, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f),  // 1
		Vertex(+hx, -hy, -hz, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 1.0f),  // 2
		Vertex(-hx, -hy, -hz, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f),  // 3

		// Back
		Vertex(+hx, +hy, +hz, 0.0f, 0.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f),  // 4
		Vertex(-hx, +hy, +hz, 0.0f, 0.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),  // 5
		Vertex(-hx, -hy, +hz, 0.0f, 0.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),  // 6
		Vertex(+hx, -hy, +hz, 0.0f, 0.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),  // 7

		// Left
		Vertex(-hx, +hy, +hz, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),  // 8
		Vertex(-hx, +hy, -hz, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),  // 9
		Vertex(-hx, -hy, -hz, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),  // 10
		Vertex(-hx, -hy, +hz, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f),  // 11

		// Right
		Vertex(+hx, +hy, -hz, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f),  // 12
		Vertex(+hx, +hy, +hz, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f, +1.0f, 1.0f, 0.0f),  // 13
		Vertex(+hx, -hy, +hz, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f, +1.0f, 1.0f, 1.0f),  // 14
		Vertex(+hx, -hy, -hz, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f),  // 15

		// Top
		Vertex(-hx, +hy, +hz, 0.0f, +1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f),  // 16
		Vertex(+hx, +hy, +hz, 0.0f, +1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f),  // 17
		Vertex(+hx, +hy, -hz, 0.0f, +1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 1.0f, 1.0f),  // 18
		Vertex(-hx, +hy, -hz, 0.0f, +1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f),  // 19

		// Bottom
		Vertex(-hx, -hy, -hz, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f),  // 20
		Vertex(+hx, -hy, -hz, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),  // 21
		Vertex(+hx, -hy, +hz, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),  // 22
		Vertex(-hx, -hy, +hz, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),  // 23
	};

	std::array<UINT, 36> indices =
	{
		// Front
		0, 1, 2, 0, 2, 3,
		// Back
		4, 5, 6, 4, 6, 7,
		// Left
		8, 9, 10, 8, 10, 11,
		// Right
		12, 13, 14, 12, 14, 15,
		// Top
		16, 17, 18, 16, 18, 19,
		// Bottom
		20, 21, 22, 20, 22, 23
	};

	Mesh::CreateResourceInfo(device, cmdList, sizeof(Vertex), sizeof(UINT),
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//
GridMesh::GridMesh(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList, 
	float width, float height, float u, float v)
{
	float hw = 0.5f * width;
	float hh = 0.5f * height;

	std::array<Vertex, 4> vertices = {
		Vertex(-hw, +hh, 0.0f, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(+hw, +hh, 0.0f, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, width/u, 0.0f),
		Vertex(+hw, -hh, 0.0f, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, width/u, height/v),
		Vertex(-hw, -hh, 0.0f, 0.0f, 0.0f, -1.0f, +1.0f, 0.0f, 0.0f, 0.0f, height/v)
	};

	std::array<UINT, 6> indices = { 0, 1, 2, 0, 2, 3 };

	Mesh::CreateResourceInfo(device, cmdList, sizeof(Vertex), sizeof(UINT),
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//
SphereMesh::SphereMesh(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList, 
	float radius, int sliceCount, int stackCount)
{
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	// Top
	vertices.push_back(Vertex(0.0f, +radius, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));

	// Side
	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f * XM_PI / sliceCount;

	for (int i = 1; i < stackCount; ++i)
	{
		float phi = i * phiStep;

		for (int j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			Vertex v;

			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);
			
			v.Normal = Vector3::Normalize(v.Position);

			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);
			v.TangentU = Vector3::Normalize(v.TangentU);

			v.TexCoord.x = theta / XM_2PI;
			v.TexCoord.y = phi / XM_PI;

			vertices.push_back(v);
		}
	}

	// Bottom
	vertices.push_back(Vertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));

	// Top
	for (int i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back((UINT)i + 1);
		indices.push_back((UINT)i);
	}

	// Side
	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; ++i)
	{
		for (int j = 0; j < sliceCount; ++j)
		{
			indices.push_back((UINT)(baseIndex + i * ringVertexCount + j));
			indices.push_back((UINT)(baseIndex + i * ringVertexCount + j + 1));
			indices.push_back((UINT)(baseIndex + (i + 1) * ringVertexCount + j));

			indices.push_back((UINT)(baseIndex + (i + 1) * ringVertexCount + j));
			indices.push_back((UINT)(baseIndex + i * ringVertexCount + j + 1));
			indices.push_back((UINT)(baseIndex + (i + 1) * ringVertexCount + j + 1));
		}
	}

	// Bottom
	int southPoleIndex = (int)vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (int i = 0; i < sliceCount; ++i)
	{
		indices.push_back((UINT)southPoleIndex);
		indices.push_back((UINT)(baseIndex + i));
		indices.push_back((UINT)(baseIndex + i + 1));
	}

	Mesh::CreateResourceInfo(device, cmdList, sizeof(Vertex), sizeof(UINT),
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
HeightMapGridMesh::HeightMapGridMesh(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	int xStart, int zStart,
	int width, int depth,
	const XMFLOAT3& scale,
	HeightMapImage* context)
	: Mesh(), mWidth(width), mDepth(depth), mScale(scale)
{
	const UINT verticesCount = width * depth;
	const UINT indicesCount = ((width * 2) * (depth - 1)) + (depth - 1 - 1);

	std::vector<TerrainVertex> vertices(verticesCount);
	std::vector<UINT> indices(indicesCount);

	int heightmapWidth = context->GetWidth();
	int heightmapDepth = context->GetDepth();

	size_t k = 0;
	for (int z = zStart; z < (zStart+depth); z++)
	{
		for (int x = xStart; x < (xStart+width); x++)
		{
			vertices[k].Position = XMFLOAT3((x * mScale.x), GetHeight(x,z,context), (z * mScale.z));
			vertices[k].Normal = context->GetNormal(x, z);
			vertices[k].TexCoord0 = XMFLOAT2((float)x / float(heightmapWidth-1), float(heightmapDepth-1-z) / float(heightmapDepth-1));
			vertices[k++].TexCoord1 = XMFLOAT2((float)x / float(mScale.x*0.5f), (float)z/float(mScale.z*0.5f));
		}
	}		
	
	k = 0;
	for (int z = 0; z < depth - 1; z++)
	{
		if (!(z & 1))
		{
			for (int x = 0; x < width; x++)
			{
				if ((x == 0) && (z > 0))
					indices[k++] = (UINT)(x + (z * width));
				indices[k++] = (UINT)(x + (z * width));
				indices[k++] = (UINT)((x + (z * width)) + width);
			}
		}
		else
		{
			for (int x = width - 1; x >= 0; x--)
			{
				if (x == (width - 1))
					indices[k++] = (UINT)(x + (z * width));
				indices[k++] = (UINT)(x + (z * width));
				indices[k++] = (UINT)((x + (z * width)) + width);
			}
		}
	}

	Mesh::CreateResourceInfo(device, cmdList, sizeof(TerrainVertex), sizeof(UINT),
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());
}

float HeightMapGridMesh::GetHeight(int x, int z, HeightMapImage* context) const
{
	XMFLOAT3 scale = context->GetScale();
	int heightmapWidth = context->GetWidth();
	float height = context->GetPixelValue(x + (z * heightmapWidth));
	return height * scale.y;
}

XMFLOAT4 HeightMapGridMesh::GetColor(int x, int z, HeightMapImage* context) const
{
	XMFLOAT3 litDir = Vector3::Normalize(XMFLOAT3(-1.0f, 1.0f, 1.0f));
	XMFLOAT3 scale = context->GetScale();
	XMFLOAT4 incidentLitColor = XMFLOAT4(0.9f, 0.8f, 0.4f, 1.0f);
	
	float reflect = Vector3::Dot(context->GetNormal(x, z), litDir);
	reflect += Vector3::Dot(context->GetNormal(x + 1, z), litDir);
	reflect += Vector3::Dot(context->GetNormal(x + 1, z + 1), litDir);
	reflect += Vector3::Dot(context->GetNormal(x, z + 1), litDir);

	reflect = (reflect / 4.0f) + 0.05f;
	reflect = Math::ClampFloat(reflect, 0.25f, 1.0f);
	
	return Vector4::Multiply(reflect, incidentLitColor);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//
HeightMapPatchListMesh::HeightMapPatchListMesh(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList,
	int xStart, int zStart, 
	int width, int depth, 
	const XMFLOAT3& scale, 
	HeightMapImage* context,
	std::shared_ptr<BulletWrapper> physics)
	: Mesh(), mWidth(width), mDepth(depth), mScale(scale)
{
	const UINT verticesCount = 25;

	mVertices.resize(25);

	const int TessFactor = 20;

	int increasement = 22;

	int heightmapWidth = context->GetWidth();
	int heightmapDepth = context->GetDepth();

	size_t k = 0;
	for (int z = (zStart+depth-1); z >= zStart; z-= increasement)
	{
		for (int x = xStart; x < (xStart + width); x+=increasement)
		{
			mVertices[k].Position = XMFLOAT3((x * mScale.x), GetHeight(x, z, context), (z * mScale.z));

			if (mVertices[k].Position.y > mMaxHeight)
				mMaxHeight = mVertices[k].Position.y;

			if (mVertices[k].Position.y < mMinHeight)
				mMinHeight = mVertices[k].Position.y;

			mVertices[k].Normal = context->GetNormal(x, z);
			mVertices[k].TexCoord0 = XMFLOAT2((float)x / float(heightmapWidth - 1), float(heightmapDepth - 1 - z) / float(heightmapDepth - 1));
			mVertices[k++].TexCoord1 = XMFLOAT2((float)x / float(mScale.x * 0.5f), (float)z / float(mScale.z * 0.5f));
		}
	}

	const auto& [min_x, max_x] = std::minmax_element(mVertices.begin(), mVertices.end(),
		[](const TerrainVertex& a, const TerrainVertex& b) { return (a.Position.x < b.Position.x); });
	const auto& [min_y, max_y] = std::minmax_element(mVertices.begin(), mVertices.end(),
		[](const TerrainVertex& a, const TerrainVertex& b) {return (a.Position.y < b.Position.y); });
	const auto& [min_z, max_z] = std::minmax_element(mVertices.begin(), mVertices.end(),
		[](const TerrainVertex& a, const TerrainVertex& b) {return (a.Position.z < b.Position.z); });

	mOOBB.Center = { (min_x->Position.x + max_x->Position.x) / 2, (min_y->Position.y + max_y->Position.y) / 2, (min_z->Position.z + max_z->Position.z) / 2 };
	mOOBB.Extents = { (max_x->Position.x - min_x->Position.x) / 2, (max_y->Position.y - min_y->Position.y) / 2, (max_z->Position.z - min_z->Position.z) / 2 };

	//BuildHeightmapData(TessFactor, vertices, context);
	
	/*auto TerrainShape = new btHeightfieldTerrainShape(TessFactor + 1, TessFactor + 1, mHeightmapData, mMinHeight, mMaxHeight, 1, false);

	double xScale = (mWidth - 1)  * mScale.x / TessFactor;

	double yScale = (mDepth - 1) * mScale.z / TessFactor;

	TerrainShape->setLocalScaling(btVector3(xScale, mScale.y, yScale));

	btTransform btTerrainTransform;
	btTerrainTransform.setIdentity();
	btTerrainTransform.setOrigin(btVector3(mOOBB.Center.x, (mMaxHeight + mMinHeight) * mScale.y / 2, mOOBB.Center.z));

	mBtRigidBody = physics->CreateRigidBody(0.0f, btTerrainTransform, TerrainShape);

	btVector3 aabbMin;
	btVector3 aabbMax;
	mBtRigidBody->getAabb(aabbMin, aabbMax);*/

	Mesh::CreateResourceInfo(device, cmdList, sizeof(TerrainVertex), 0,
		D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST, mVertices.data(), (UINT)mVertices.size(), nullptr, 0);
}

HeightMapPatchListMesh::~HeightMapPatchListMesh()
{
	//delete mHeightmapData;
}

float HeightMapPatchListMesh::GetHeight(int x, int z, HeightMapImage* context) const
{
	float height = context->GetHeight(x * mScale.x, z * mScale.z, mScale);
	return height;
}

//void HeightMapPatchListMesh::BuildHeightmapData(const int& TessFactor, const std::vector<TerrainVertex>& TerrainVertices, HeightMapImage* context)
//{
//	mHeightmapData = new float[(TessFactor) * (TessFactor)];
//
//	auto CubicBezierSum = [](const std::vector<TerrainVertex>& patch, XMFLOAT2 t) {
//
//		// 4차 베지어 곡선 계수
//		std::array<float, 5> uB, vB;
//		float txInv{ 1.0f - t.x };
//		uB[0] = txInv * txInv * txInv * txInv;
//		uB[1] = 4.0f * t.x * txInv * txInv * txInv;
//		uB[2] = 6.0f * t.x * t.x * txInv * txInv;
//		uB[3] = 4.0f * t.x * t.x * t.x * txInv;
//		uB[4] = t.x * t.x * t.x * t.x;
//
//		float tyInv{ 1.0f - t.y };
//		vB[0] = tyInv * tyInv * tyInv * tyInv;
//		vB[1] = 4.0f * t.y * tyInv * tyInv * tyInv;
//		vB[2] = 6.0f * t.y * t.y * tyInv * tyInv;
//		vB[3] = 4.0f * t.y * t.y * t.y * tyInv;
//		vB[4] = t.y * t.y * t.y * t.y;
//
//		// 4차 베지에 곡면 계산
//		XMFLOAT3 sum{ 0.0f, 0.0f, 0.0f };
//		for (int i = 0; i < 5; ++i)
//		{
//			XMFLOAT3 subSum{ 0.0f, 0.0f, 0.0f };
//			for (int j = 0; j < 5; ++j)
//			{
//				XMFLOAT3 temp{ Vector3::Multiply(uB[j], patch[(i * 5) + j].Position) };
//				subSum = Vector3::Add(subSum, temp);
//			}
//			subSum = Vector3::Multiply(vB[i], subSum);
//			sum = Vector3::Add(sum, subSum);
//		}
//		return sum;
//	};
//
//	float debugTess[20][20];
//
//	for (int i = 0; i < TessFactor; ++i)
//	{
//		for (int j = 0; j < TessFactor; ++j)
//		{
//			XMFLOAT2 uv{ (float)i / (TessFactor - 1), 1.0f - ((float)j / (TessFactor - 1)) };
//			XMFLOAT3 posOnBazier{ CubicBezierSum(TerrainVertices, uv) };
//
//			mHeightmapData[i + (int)(j * (TessFactor))] = posOnBazier.y;
//		}
//	}
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
ParticleMesh::ParticleMesh(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const XMFLOAT3& position,
	const XMFLOAT2& size,
	const XMFLOAT3& direction,
	float lifeTime,
	float speed,
	int maxParticle)
	: mUploadBufferFilledSize(NULL), mMaxParticle(maxParticle), mStart(true)
{
	BillboardVertex vertex;
	vertex.Position = position;
	vertex.Size = size;
	vertex.Direction = direction;
	vertex.Age = XMFLOAT2(0.0f, lifeTime);
	vertex.Speed = speed;
	vertex.Type = 0;

	Mesh::CreateResourceInfo(device, cmdList,
		sizeof(BillboardVertex), 0, D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		reinterpret_cast<const void*>(&vertex), 1, nullptr, 0);

	CreateStreamOutputBuffer(device, cmdList);
}

void ParticleMesh::PrepareBufferViews(ID3D12GraphicsCommandList* cmdList, bool isSO)
{
	if (isSO)
	{
		*mUploadBufferFilledSize = 0;

		cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
			mDefaultBuffer.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_DEST));

		cmdList->CopyResource(mDefaultBuffer.Get(), mUploadBuffer.Get());

		cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
			mDefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT));
	}
	else
	{
		mVertexBufferView.BufferLocation = mRenderBuffer->GetGPUVirtualAddress();
		mVertexBufferView.StrideInBytes = mVertexStride;
		mVertexBufferView.SizeInBytes = mVerticesCount * mVertexStride;
	}
}

void ParticleMesh::Draw(ID3D12GraphicsCommandList* cmdList, bool isSO)
{
	if (isSO)
	{
		D3D12_STREAM_OUTPUT_BUFFER_VIEW soViews[] = { mSOBufferView };
		cmdList->SOSetTargets(0, 1, soViews);

		cmdList->BeginQuery(mQueryHeap.Get(), D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);
		Mesh::Draw(cmdList);
		cmdList->EndQuery(mQueryHeap.Get(), D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);
		
		cmdList->ResolveQueryData(mQueryHeap.Get(), 
			D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0,
			0, 1, mQueryBuffer.Get(), 0);

		CopyToRenderBuffer(cmdList);

		D3D12_RANGE d3dReadRange = { 0, 0 };
		mQueryBuffer->Map(0, &d3dReadRange, (void**)&mQueryStatistics);
		UINT64 count = mQueryStatistics->NumPrimitivesWritten;
		if (count > 0) mVerticesCount = (UINT)count;
		mQueryBuffer->Unmap(0, NULL);
	}
	else
	{
		cmdList->SOSetTargets(0, 0, NULL);
		Mesh::Draw(cmdList);
	}
}

void ParticleMesh::CreateStreamOutputBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	ThrowIfFailed(device->CreateCommittedResource(
		&Extension::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&Extension::BufferResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, mMaxParticle * mVertexStride),
		D3D12_RESOURCE_STATE_STREAM_OUT, NULL, IID_PPV_ARGS(&mStreamOutputBuffer)));

	ThrowIfFailed(device->CreateCommittedResource(
		&Extension::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&Extension::BufferResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, mMaxParticle * mVertexStride),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL, IID_PPV_ARGS(&mRenderBuffer)));

	ThrowIfFailed(device->CreateCommittedResource(
		&Extension::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&Extension::BufferResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, sizeof(UINT64)),
		D3D12_RESOURCE_STATE_STREAM_OUT, NULL, IID_PPV_ARGS(&mDefaultBuffer)));
	
	ThrowIfFailed(device->CreateCommittedResource(
		&Extension::HeapProperties(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&Extension::BufferResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, sizeof(UINT64)),
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&mUploadBuffer)));
	mUploadBuffer->Map(0, NULL, (void**)&mUploadBufferFilledSize);

	mSOBufferView.BufferLocation = mStreamOutputBuffer->GetGPUVirtualAddress();
	mSOBufferView.SizeInBytes = mMaxParticle * mVertexStride;
	mSOBufferView.BufferFilledSizeLocation = mDefaultBuffer->GetGPUVirtualAddress();
	
	D3D12_QUERY_HEAP_DESC queryDesc{};
	queryDesc.Type = D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
	queryDesc.Count = 1;
	queryDesc.NodeMask = 0;

	ThrowIfFailed(device->CreateQueryHeap(&queryDesc, IID_PPV_ARGS(&mQueryHeap)));
	ThrowIfFailed(device->CreateCommittedResource(
		&Extension::HeapProperties(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&Extension::BufferResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, sizeof(D3D12_QUERY_DATA_SO_STATISTICS)),
		D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&mQueryBuffer)));
}

void ParticleMesh::CopyToRenderBuffer(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mStreamOutputBuffer.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE));
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mRenderBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST));

	cmdList->CopyResource(mRenderBuffer.Get(), mStreamOutputBuffer.Get());

	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mStreamOutputBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT));
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mRenderBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}
