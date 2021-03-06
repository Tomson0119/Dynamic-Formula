#include "stdafx.h"
#include "gameObject.h"
#include "inGameScene.h"
#include "pipeline.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}

void GameObject::BuildSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	for (const auto& tex : mTextures)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = tex->ShaderResourceView();
		device->CreateShaderResourceView(tex->GetResource(), &srvDesc, cpuHandle);
		cpuHandle.ptr += gCbvSrvUavDescriptorSize;
	}
}

void GameObject::LoadModel(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList, 
	const std::wstring& path,
	bool collider)
{
	std::ifstream in_file{ path, std::ios::binary };
	assert(in_file.is_open());

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> texcoords;
	std::unordered_map<std::string, MatInfo> mats;
	std::shared_ptr<Mesh> new_mesh;

	std::string info;
	std::string objName;

	while (std::getline(in_file, info))
	{
		std::stringstream ss(info);
		std::string type;

		ss >> type;

		if (type == "mtllib")
		{
			std::string mtlname;
			ss >> mtlname;

			std::wstring mtl_path = L"";
			std::wstring::size_type n = path.find('\\');
			if (n != std::wstring::npos)
				mtl_path = path.substr(0, n + 1);

			mtl_path += std::wstring(mtlname.begin(), mtlname.end());

			LoadMaterial(device, cmdList, mats, mtl_path);
		}
		else if (type == "o")
		{
			ss >> objName;
		}

		else if (type == "v")
		{
			XMFLOAT3 pos;
			ss >> pos.x >> pos.y >> pos.z;
			pos.z *= -1.0f;
			positions.push_back(pos);
		}
		else if (type == "vt")
		{
			XMFLOAT2 tex;
			ss >> tex.x >> tex.y;
			//tex.y = 1.0f - tex.y;
			texcoords.push_back(tex);
		}
		else if (type == "vn")
		{
			XMFLOAT3 norm;
			ss >> norm.x >> norm.y >> norm.z;
			norm.z *= -1.0f;
			normals.push_back(norm);
		}
		else if (type == "usemtl")
		{
			std::string mtl_name;
			ss >> mtl_name;

			new_mesh = std::make_shared<Mesh>(objName, mtl_name);

			objName.clear();

			new_mesh->LoadMesh(
				device, cmdList, in_file,
				positions, normals, texcoords, mats[mtl_name], collider);

			mMeshes.push_back(new_mesh);
		}
	}

	const auto& [min_x, max_x] = std::minmax_element(positions.begin(), positions.end(), 
		[](const XMFLOAT3& a, const XMFLOAT3& b) { return (a.x < b.x); });
	const auto& [min_y, max_y] = std::minmax_element(positions.begin(), positions.end(),
		[](const XMFLOAT3& a, const XMFLOAT3& b) {return (a.y < b.y); });
	const auto& [min_z, max_z] = std::minmax_element(positions.begin(), positions.end(),
		[](const XMFLOAT3& a, const XMFLOAT3& b) {return (a.z < b.z); });

	mOOBB.Center = { (min_x->x + max_x->x) / 2, (min_y->y + max_y->y) / 2, (min_z->z + max_z->z) / 2 };
	mOOBB.Extents = { (max_x->x - min_x->x) / 2, (max_y->y - min_y->y) / 2, (max_z->z - min_z->z) / 2 };
}

void GameObject::LoadMaterial(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList, 
	std::unordered_map<std::string, MatInfo>& mats, 
	const std::wstring& path)
{
	std::ifstream mtl_file{ path, std::ios::binary };

	std::string info;
	std::string mat_name;

	std::unordered_map<std::string, int> tex_names;
	XMFLOAT2 tex_offset = { 0.0f,0.0f };
	XMFLOAT2 tex_scale = { 1.0f,1.0f };
	int tex_index = 0;

	while (std::getline(mtl_file, info))
	{
		std::stringstream ss(info);

		std::string type;
		ss >> type;
		
		if (type == "newmtl")
		{
			ss >> mat_name;
			mats[mat_name].Mat = Material();
		}
		else if (type == "Ns")
		{
			ss >> mats[mat_name].Mat.Exponent;
		}
		else if (type == "Ka")
		{
			float ambient;
			ss >> ambient;
			mats[mat_name].Mat.Ambient = { ambient,ambient,ambient };
		}
		else if (type == "Kd")
		{
			XMFLOAT4& diffuse = mats[mat_name].Mat.Diffuse;
			ss >> diffuse.x >> diffuse.y >> diffuse.z;
		}
		else if (type == "Ks")
		{
			XMFLOAT3& specular = mats[mat_name].Mat.Specular;
			ss >> specular.x >> specular.y >> specular.z;

		}
		else if (type == "Ke")
		{
			XMFLOAT3& emission = mats[mat_name].Mat.Emission;
			ss >> emission.x >> emission.y >> emission.z;
		}
		else if (type == "Ni")
		{
			ss >> mats[mat_name].Mat.IOR;
		}
		else if (type == "d")
		{
			ss >> mats[mat_name].Mat.Diffuse.w;
		}
		else if (type == "map_Kd")
		{
			std::string option;
			float ignore0;
			
			while (ss.eof() == false)
			{
				ss >> option;
				if (ss.fail()) {
					ss >> ignore0;
					continue;
				}

				if (option == "-s")
				{
					ss >> tex_scale.x >> tex_scale.y;
				}
				else if (option == "-o")
				{
					ss >> tex_offset.x >> tex_offset.y;
				}
			}

			mats[mat_name].TexOffset = tex_offset;
			mats[mat_name].TexScale = tex_scale;			
			
			std::string::size_type beg = option.rfind('\\');
			std::string::size_type end = option.rfind('.');
			option = option.substr(beg + 1, end - beg - 1);

			auto p = tex_names.find(option);
			if (p == tex_names.end())
			{
				tex_names[option] = tex_index;
				mats[mat_name].SrvIndex = tex_index++;

				std::wstring tex_path = L"Resources\\" + std::wstring(option.begin(), option.end()) + L".dds";
				LoadTexture(device, cmdList, tex_path);
			}
			else
			{
				mats[mat_name].SrvIndex = p->second;
			}
		}
	}
}

void GameObject::LoadTexture(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* cmdList, 
	const std::wstring& path, D3D12_SRV_DIMENSION dimension)
{
	auto tex = std::make_unique<Texture>();
	tex->LoadTextureFromDDS(device, cmdList, path);
	tex->SetDimension(dimension);
	mTextures.push_back(std::move(tex));
}

// Scale?? ?????? ?? ?????? ??!
bool GameObject::LoadConvexHullShape(const std::wstring& path, const std::shared_ptr<BulletWrapper>& physics)
{
	std::ifstream in_file{ path, std::ios::binary };

	if (!in_file.is_open())
		return false;

	std::vector<XMFLOAT3> positions;

	mBtCollisionShape = new btCompoundShape();

	std::string info;

	while (std::getline(in_file, info))
	{
		std::stringstream ss(info);
		std::string type;

		ss >> type;

		if (type == "v")
		{
			XMFLOAT3 pos;
			ss >> pos.x >> pos.y >> pos.z;
			pos.z *= -1.0f;

			positions.push_back(Vector3::Multiply(mScaling, pos));
		}
		else if (type == "s")
		{
			btConvexHullShape* convexHull = new btConvexHullShape();

			for (int i = 0; i < positions.size(); ++i)
				convexHull->addPoint(btVector3(positions[i].x, positions[i].y, positions[i].z));

			positions.clear();

			btTransform localTransform;
			localTransform.setIdentity();
			localTransform.setOrigin(btVector3(0, 0, 0));

			physics->AddShape(convexHull);
			mBtCollisionShape->addChildShape(localTransform, convexHull);
		}
	}

	return true;
}

//???????? ???? ?? ?????????? ?????? ??
void GameObject::BuildRigidBody(float mass, const std::shared_ptr<BulletWrapper>& physics)
{
	if (mBtCollisionShape)
	{
		btTransform btObjectTransform;
		btObjectTransform.setIdentity();
		btObjectTransform.setOrigin(btVector3(mPosition.x, mPosition.y, mPosition.z));
		btObjectTransform.setRotation(btQuaternion(mQuaternion.x, mQuaternion.y, mQuaternion.z, mQuaternion.w));
		mBtRigidBody = physics->CreateRigidBody(mass, btObjectTransform, mBtCollisionShape);
	}
}

void GameObject::RemoveObject(btDiscreteDynamicsWorld& dynamicsWorld, Pipeline& pipeline)
{
	if (mBtRigidBody)
	{
		delete mBtRigidBody->getMotionState();
		dynamicsWorld.removeRigidBody(mBtRigidBody);
		delete mBtRigidBody;
		mBtRigidBody = nullptr;
	}

	for (int idx = 0; const auto& obj : pipeline.GetRenderObjects())
	{
		if (this == obj.get())
		{
			pipeline.DeleteObject(idx);
			break;
		}
		idx += 1;
	}

	mInterpolator.Clear();
}

void GameObject::Update(float elapsedTime, int64_t clockDelta)
{
	RotateDirectionVectors();

	mOldWorld = mWorld;

	if (mBtRigidBody)
	{
		InterpolateRigidBody(elapsedTime);
		SetWorldByMotionState();
	}
	else
	{
		InterpolateWorldTransform(elapsedTime, clockDelta);
		UpdateTransform();
	}

	//UpdateBoundingBox();
}

void GameObject::Draw(
	ID3D12GraphicsCommandList* cmdList,
	UINT rootMatIndex, UINT rootCbvIndex, UINT rootSrvIndex,
	UINT64 matGPUAddress, UINT64 byteOffset, bool isSO)
{
	cmdList->SetGraphicsRootDescriptorTable(rootCbvIndex, mCbvGPUAddress);

	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle{};
	for (int i = 0; i < mMeshes.size(); i++)
	{
		mMeshes[i]->PrepareBufferViews(cmdList, isSO);

		int srvIndex = mMeshes[i]->GetSrvIndex();

		if (srvIndex >= 0)
		{
			srvGpuHandle = mSrvGPUAddress;
			srvGpuHandle.ptr += srvIndex * gCbvSrvUavDescriptorSize;
			cmdList->SetGraphicsRootDescriptorTable(rootSrvIndex, srvGpuHandle);
		}
		cmdList->SetGraphicsRootConstantBufferView(rootMatIndex, matGPUAddress);
		mMeshes[i]->Draw(cmdList, isSO);

		matGPUAddress += byteOffset;
	}
}
void GameObject::DrawInstanced(ID3D12GraphicsCommandList* cmdList,
	UINT rootMatIndex, UINT rootSBIndex, UINT rootSrvIndex,
	UINT64 matGPUAddress, UINT64 byteOffset, int InstanceCount, bool isSO)
{
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle{};
	for (int i = 0; i < mMeshes.size(); i++)
	{
		mMeshes[i]->PrepareBufferViews(cmdList, isSO);

		int srvIndex = mMeshes[i]->GetSrvIndex();

		if (srvIndex >= 0)
		{
			srvGpuHandle = mSrvGPUAddress;
			srvGpuHandle.ptr += srvIndex * gCbvSrvUavDescriptorSize;
			cmdList->SetGraphicsRootDescriptorTable(rootSrvIndex, srvGpuHandle);
		}
		cmdList->SetGraphicsRootConstantBufferView(rootMatIndex, matGPUAddress);

		mMeshes[i]->DrawInstanced(cmdList, InstanceCount, isSO);

		matGPUAddress += byteOffset;
	}
}

void GameObject::UpdateTransform()
{
	mWorld = Matrix4x4::CalulateWorldTransform(mPosition, mQuaternion, mScaling);
	/*mWorld(0, 0) = mScaling.x * mRight.x;
	mWorld(0, 1) = mRight.y;
	mWorld(0, 2) = mRight.z;

	mWorld(1, 0) = mUp.x;
	mWorld(1, 1) = mScaling.y * mUp.y;
	mWorld(1, 2) = mUp.z;

	mWorld(2, 0) = mLook.x;
	mWorld(2, 1) = mLook.y;
	mWorld(2, 2) = mScaling.z * mLook.z;

	mWorld(3, 0) = mPosition.x;
	mWorld(3, 1) = mPosition.y;
	mWorld(3, 2) = mPosition.z;*/
}

void GameObject::UpdateInverseWorld()
{
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	XMStoreFloat4x4(&mInvWorld, invWorld);
}

void GameObject::RotateDirectionVectors()
{
	XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&mQuaternion));

	mRight = Vector3::TransformNormal(XMFLOAT3(1.0f,0.0f,0.0f), R);
	mUp = Vector3::TransformNormal(XMFLOAT3(0.0f,1.0f,0.0f), R);
	mLook = Vector3::TransformNormal(XMFLOAT3(0.0f,0.0f,1.0f), R);

	mRight = Vector3::Normalize(mRight);
	mUp = Vector3::Normalize(mUp);
	mLook = Vector3::Normalize(mLook);
}

void GameObject::SetWorldByMotionState()
{
	btScalar m[16];
	btTransform btMat{};
	mBtRigidBody->getMotionState()->getWorldTransform(btMat);
	btMat.getOpenGLMatrix(m);

	btQuaternion quaternion = btMat.getRotation();

	mQuaternion = XMFLOAT4(quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w());

	mWorld = Matrix4x4::glMatrixToD3DMatrix(m);
	ResetTransformVectors();
}

void GameObject::ResetTransformVectors()
{
	mPosition.x = mWorld(3, 0);
	mPosition.y = mWorld(3, 1);
	mPosition.z = mWorld(3, 2);

	mLook.x = mWorld(2, 0);
	mLook.y = mWorld(2, 1);
	mLook.z = mWorld(2, 2);

	mUp.x = mWorld(1, 0);
	mUp.y = mWorld(1, 1);
	mUp.z = mWorld(1, 2);

	mRight.x = mWorld(0, 0);
	mRight.y = mWorld(0, 1);
	mRight.z = mWorld(0, 2);
}

void GameObject::UpdateBoundingBox()
{
	mOOBB.Center = { 0.0f, 0.0f, 0.0f };
	mOOBB.Transform(mOOBB, XMLoadFloat4x4(&mWorld));
	XMStoreFloat4(&mOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&mOOBB.Orientation)));
}

void GameObject::Animate(float elapsedTime)
{
	if (!Vector3::Equal(mMoveDirection, Vector3::Zero()))
		Move(mMoveDirection, mMoveSpeed * elapsedTime);

	if (!Vector3::Equal(mRotationAxis, Vector3::Zero()))
		Rotate(mRotationAxis, mRotationSpeed * elapsedTime);
}

void GameObject::UpdateMatConstants(ConstantBuffer<MaterialConstants>* matCnst, int offset)
{
	for (int i = 0; i < mMeshes.size(); i++)
		matCnst->CopyData(offset + i, mMeshes[i]->GetMaterialConstant());
}

void GameObject::SortMeshes()
{
	std::sort(
		mMeshes.begin(), mMeshes.end(),
		[](std::shared_ptr<Mesh> first, std::shared_ptr<Mesh> second)
		{
			return first->mOOBB.Center.z > second->mOOBB.Center.z;
		}
	);
}

void GameObject::InterpolateRigidBody(float elapsed)
{
	//auto rigid = mBtRigidBody;
	//if (rigid == nullptr) return;

	//if (mPrevOrigin.IsZero())
	//{
	//	auto motionState = rigid->getMotionState();

	//	btTransform transform{};
	//	motionState->getWorldTransform(transform);

	//	// Get current state of position/rotation.
	//	auto& currentOrigin = transform.getOrigin();
	//	auto& currentQuat = transform.getRotation();

	//	mPrevOrigin.SetValue(currentOrigin);
	//	mPrevQuat.SetValue(currentQuat);
	//}

	//const btVector3& prevOrigin = mPrevOrigin.GetBtVector3();
	//const btQuaternion& prevQuat = mPrevQuat.GetBtQuaternion();

	//// Get correction state of extrapolated server postion/rotation.
	//const btVector3& correctOrigin = mCorrectionOrigin.GetBtVector3();
	//const btQuaternion& correctQuat = mCorrectionQuat.GetBtQuaternion();

	//if (BulletMath::IsZero(correctQuat) || updateRate <= 0.0f) return;

	//float progress = mProgress / FIXED_FLOAT_LIMIT;
	//progress = std::min(1.0f, progress + elapsed / updateRate);
	//mProgress = (int)(progress * FIXED_FLOAT_LIMIT);

	//btVector3 nextOrigin = prevOrigin.lerp(correctOrigin, progress);
	//btQuaternion nextQuat = prevQuat.slerp(correctQuat, progress);

	//btTransform nextTransform{};
	//nextTransform.setOrigin(nextOrigin);
	//nextTransform.setRotation(nextQuat);

	//// manually set rigidbody tranform.
	//rigid->setWorldTransform(nextTransform);
}

void GameObject::InterpolateWorldTransform(float elapsed, uint64_t clockDelta)
{
	mPosition = mCorrectionOrigin.GetXMFloat3();
	mQuaternion = mCorrectionQuat.GetXMFloat4();

	//mInterpolator.Interpolate(elapsed, clockDelta, mPosition, mQuaternion);

	//if (updateRate <= 0.0f) return;

	//mProgressMut.lock();
	//mProgress += elapsed;
	//float progress = mProgress / updateRate;
	//mProgressMut.unlock();

	//const XMFLOAT3& prevOrigin = mPrevOrigin.GetXMFloat3();
	//const XMFLOAT4& prevQuat = mPrevQuat.GetXMFloat4();

	//// Get correction state of extrapolated server postion/rotation.
	//const XMFLOAT3& correctOrigin = mCorrectionOrigin.GetXMFloat3();
	//const XMFLOAT4& correctQuat = mCorrectionQuat.GetXMFloat4();

	//mPosition = Vector3::Lerp(prevOrigin, correctOrigin, progress);
	//mQuaternion = Vector4::Slerp(prevQuat, correctQuat, progress);
}

void GameObject::SetPosition(float x, float y, float z)
{
	SetPosition({ x, y, z });
}

void GameObject::SetPosition(const XMFLOAT3& pos)
{
	mPosition = pos;
	mPrevOrigin = pos;
	mCorrectionOrigin = pos;
}

void GameObject::SetDiffuse(const std::string& name, const XMFLOAT4& color)
{
	auto p = std::find_if(mMeshes.begin(), mMeshes.end(),
		[&name](const auto& mesh) { return (mesh->GetMaterialName() == name); });

	if (p != mMeshes.end())
		(*p)->SetMatDiffuse(color);
}

void GameObject::SetLook(XMFLOAT3& look)
{
	mLook = look;
	//GameObject::Update(1.0f);
}

void GameObject::CopyMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes)
{
	for (int i = 0; i < meshes.size(); ++i)
	{
		mMeshes.push_back(std::make_shared<Mesh>(*meshes[i]));
	}
	SetBoudingBoxFromMeshes();
}

void GameObject::SetBoudingBox(BoundingOrientedBox oobb)
{
	mOOBB = oobb;
}

void GameObject::SetBoudingBoxFromMeshes()
{
	float min_x = 0, max_x = 0, min_y = 0, max_y = 0, min_z = 0, max_z = 0;

	for (int i = 0; i < mMeshes.size(); ++i)
	{
		XMFLOAT3 corners[8];
		mMeshes[i]->mOOBB.GetCorners(corners);

		for (int j = 0; j < 8; ++j)
		{
			if (corners[j].x < min_x)
				min_x = corners[j].x;
			
			if(corners[j].x > max_x)
				max_x = corners[j].x;

			if (corners[j].y < min_y)
				min_y = corners[j].y;
			
			if(corners[j].y > max_y)
				max_y = corners[j].y;

			if (corners[j].z < min_z)
				min_z = corners[j].z;
			
			if(corners[j].z > max_z)
				max_z = corners[j].z;
		}
	}

	mOOBB.Center = { (min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2 };
	mOOBB.Extents = { (max_x - min_x) / 2, (max_y - min_y) / 2, (max_z - min_z) / 2 };
}

void GameObject::SetRotation(XMFLOAT3& axis, float speed)
{
	mRotationAxis = axis;
	mRotationSpeed = speed;
}

void GameObject::SetMovement(XMFLOAT3& dir, float speed)
{
	mMoveDirection = dir;
	mMoveSpeed = speed;
}

void GameObject::ChangeUpdateFlag(UPDATE_FLAG expected, const UPDATE_FLAG& desired)
{
	mUpdateFlag.compare_exchange_strong(expected, desired);
}

const XMFLOAT4& GameObject::GetMeshDiffuse(const std::string& name)
{
	auto iter = std::find_if(mMeshes.begin(), mMeshes.end(),
		[&name](const auto& mesh) { return (mesh->GetMaterialName() == name); });
	return (*iter)->GetDiffuse();
}

void GameObject::Move(float dx, float dy, float dz)
{
	mPosition.x += dx;
	mPosition.y += dy;
	mPosition.z += dz;
}

void GameObject::Move(XMFLOAT3& dir, float dist)
{
	XMFLOAT3 shift = { 0.0f,0.0f,0.0f };
	shift = Vector3::Add(shift, dir, dist);
	Move(shift.x, shift.y, shift.z);
}

void GameObject::Strafe(float dist, bool local)
{
	XMFLOAT3 right = (local) ? mRight : XMFLOAT3(1.0f, 0.0f, 0.0f);
	Move(right, dist);
}

void GameObject::Upward(float dist, bool local)
{
	XMFLOAT3 up = (local) ? mUp : XMFLOAT3(0.0f, 1.0f, 0.0f);
	Move(up, dist);
}

void GameObject::Walk(float dist, bool local)
{
	XMFLOAT3 look = (local) ? mLook : XMFLOAT3(0.0f, 0.0f, 1.0f);
	Move(look, dist);
}

void GameObject::Rotate(float pitch, float yaw, float roll)
{
	if (pitch != 0.0f)
	{
		Rotate(mRight, pitch);
	}
	if (yaw != 0.0f)
	{
		Rotate(mUp, yaw);
	}
	if (roll != 0.0f)
	{
		Rotate(mLook, roll);
	}
}

void GameObject::Rotate(const XMFLOAT3& axis, float angle)
{
	XMStoreFloat4(&mQuaternion, 
		XMQuaternionRotationAxis(
			XMLoadFloat3(&axis), 
			XMConvertToRadians(angle)));
}

void GameObject::SetQuaternion(const XMFLOAT4& quaternion)
{
	mQuaternion = quaternion;
	mPrevQuat = quaternion;
	mCorrectionQuat = quaternion;
}

void GameObject::SetQuaternion(float x, float y, float z, float w)
{
	SetQuaternion({ x,y,z,w });
}

void GameObject::RotateY(float angle)
{
	Rotate(mUp, angle);
}

void GameObject::Pitch(float angle)
{
	Rotate(mRight, angle);
}

void GameObject::Scale(float xScale, float yScale, float zScale)
{
	mScaling = { xScale, yScale, zScale };
}

void GameObject::Scale(const XMFLOAT3& scale)
{
	Scale(scale.x, scale.y, scale.z);
}

void GameObject::Scale(float scale)
{
	Scale(scale, scale, scale);
}

ObjectConstants GameObject::GetObjectConstants()
{
	ObjectConstants objCnst = {};
	if (mReflected)
	{
		objCnst.World = Matrix4x4::Transpose(Matrix4x4::Multiply(mWorld, mReflectMatrix));
		objCnst.oldWorld = Matrix4x4::Transpose(Matrix4x4::Multiply(mOldWorld, mReflectMatrix));
	}
	else
	{
		objCnst.World = Matrix4x4::Transpose(mWorld);
		objCnst.oldWorld = Matrix4x4::Transpose(mOldWorld);
	}
	objCnst.cubemapOn = mCubemapOn;
	objCnst.motionBlurOn = mMotionBlurOn;
	objCnst.rimLightOn = mRimLightOn;
	objCnst.invincibleOn = mTransparentOn;

	return objCnst;
}

InstancingInfo GameObject::GetInstancingInfo()
{
	InstancingInfo instInfo = {};
	instInfo.World = Matrix4x4::Transpose(mWorld);
	instInfo.oldWorld = Matrix4x4::Transpose(mOldWorld);

	return instInfo;
}

///////////////////////////////////////////////////////////////////////////////
//
TerrainObject::TerrainObject(int width, int depth, const XMFLOAT3& scale)
	: GameObject(), mWidth(width), mDepth(depth), mTerrainScale(scale)
{
}

TerrainObject::~TerrainObject()
{
	mTerrainRigidBodies.clear();
}

void TerrainObject::BuildHeightMap(const std::wstring& path)
{
	mHeightMapImage = std::make_unique<HeightMapImage>(path, mWidth, mDepth, mTerrainScale);
}

void TerrainObject::BuildTerrainMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<BulletWrapper>& physics, int blockWidth, int blockDepth)
{	
	mBlockWidth = blockWidth;
	mBlockDepth = blockDepth;

	int xBlocks = (mWidth - 1) / (blockWidth - 1);
	int zBlocks = (mDepth - 1) / (blockDepth - 1);

	float maxHeight = -FLT_MAX;
	float minHeight = FLT_MAX;

	for (int z = 0, zStart = 0; z < zBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < xBlocks; x++)
		{
			xStart = x * (blockWidth - 1);
			zStart = z * (blockDepth - 1);
			auto gridMesh = std::make_shared<HeightMapPatchListMesh>(device, cmdList, xStart, zStart, blockWidth, blockDepth, mTerrainScale, mHeightMapImage.get(), physics);
			gridMesh->SetIndex(x, z);

			mTerrainRigidBodies.push_back(gridMesh->GetRigidBody());

			SetMesh(gridMesh);

			auto [gridMin, gridMax] = gridMesh->GetMinMax();
			if (gridMin < minHeight)
				minHeight = gridMin;
			if (gridMax > maxHeight)
				maxHeight = gridMax;
		}
	}
}

float TerrainObject::GetHeight(float x, float z) const
{
	assert(mHeightMapImage && "HeightMapImage doesn't exist");
	return mHeightMapImage->GetHeight(x, z, mTerrainScale);
}

XMFLOAT3 TerrainObject::GetNormal(float x, float z) const
{
	assert(mHeightMapImage && "HeightMapImage doesn't exist");
	return mHeightMapImage->GetNormal((int)(x / mTerrainScale.x), (int)(z / mTerrainScale.z));
}

///////////////////////////////////////////////////////////////////////////////
//
Billboard::Billboard(float width, float height)
	: mDurationTime(0)
{
	mWidth = width;
	mHeight = height;
}

Billboard::~Billboard()
{
}

void Billboard::AppendBillboard(const XMFLOAT3& pos)
{
	BillboardVertex vertex = { pos, XMFLOAT2(mWidth, mHeight) };
	
	mVertices.push_back(vertex);
	mIndices.push_back((UINT)mIndices.size());
}

void Billboard::BuildMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	auto mesh = std::make_shared<Mesh>();
	mesh->CreateResourceInfo(device, cmdList,
		sizeof(BillboardVertex), sizeof(UINT), D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		mVertices.data(), (UINT)mVertices.size(), mIndices.data(), (UINT)mIndices.size());
	SetMesh(mesh);
}

void Billboard::SetDurationTime(std::chrono::milliseconds& time)
{
	mCreationTime = std::chrono::steady_clock::now();
	mDurationTime = time;
}

bool Billboard::IsTimeOver(std::chrono::steady_clock::time_point& currentTime)
{
	if (mDurationTime == std::chrono::milliseconds(0)) return false;
	return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mCreationTime) > mDurationTime;
}

MissileObject::MissileObject(const XMFLOAT3& position)
	: GameObject(), mActive{ false } 
{
	SetPosition(position);
}

MissileObject::~MissileObject()
{
}

void MissileObject::SetMesh(const std::shared_ptr<Mesh>& mesh, btVector3 forward, XMFLOAT3 position, std::shared_ptr<BulletWrapper> physics)
{
	GameObject::SetMesh(mesh);

	auto dynamicsWorld = physics->GetDynamicsWorld();

	auto missileExtents = btVector3(mMeshes[0]->mOOBB.Extents.x, mMeshes[0]->mOOBB.Extents.y, mMeshes[0]->mOOBB.Extents.z);
	btCollisionShape* missileShape = new btBoxShape(missileExtents);

	btVector3 bulletPosition = 15 * forward;

	btTransform btMissileTransform;
	btMissileTransform.setIdentity();
	btMissileTransform.setOrigin(btVector3(position.x + bulletPosition.x(), position.y + bulletPosition.y(), position.z + bulletPosition.z()));

	mBtRigidBody = physics->CreateRigidBody(1.0f, btMissileTransform, missileShape);

	mBtRigidBody->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	mBtRigidBody->setLinearVelocity(forward * 1000.0f);
}

void MissileObject::SetCorrectionTransform(SC::packet_missile_transform* pck, uint64_t timePoint, float latency)
{
	mCurrentTime = 0;
	mPrevOrigin = mCorrectionOrigin;

	mCorrectionOrigin.SetValue(
		pck->pos_x / POS_FLOAT_PRECISION,
		pck->pos_z / POS_FLOAT_PRECISION);

	mCorrectionOrigin.Extrapolate(
		pck->linear_vel_x / POS_FLOAT_PRECISION,
		pck->linear_vel_z / POS_FLOAT_PRECISION,
		latency);

	//mInterpolator.Enqueue(timePoint, mCorrectionOrigin.GetXMFloat3(), mCorrectionQuat.GetXMFloat4());
}

void MissileObject::SetActive(bool state)
{
	mActive = state;
}

void MissileObject::Update(float elapsedTime, int64_t clockDelta)
{
	if (mActive)
	{
		GameObject::Update(elapsedTime, clockDelta);
	}
}

void StaticObject::Update(float elapsedTime, int64_t clockDelta)
{
	RotateDirectionVectors();

	mOldWorld = mWorld;

	UpdateTransform();
}

SOParticleObject::SOParticleObject(GameObject& parent) : GameObject(), mParent{ parent }
{
}

void SOParticleObject::Update(float elapsedTime, int64_t clockDelta)
{
	mPosition = mLocalOffset;

	RotateDirectionVectors();
	UpdateTransform();

	mWorld = Matrix4x4::Multiply(mWorld, mParent.GetWorld());
}

void SOParticleObject::Draw(ID3D12GraphicsCommandList* cmdList, UINT rootMatIndex, UINT rootCbvIndex, UINT rootSrvIndex, UINT64 matGPUAddress, UINT64 byteOffset, bool isSO)
{
	cmdList->SetGraphicsRootDescriptorTable(rootCbvIndex, mCbvGPUAddress);
	cmdList->SetGraphicsRoot32BitConstants(8, 1, &mParticleEnable, 4);

	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle{};
	for (int i = 0; i < mMeshes.size(); i++)
	{
		mMeshes[i]->PrepareBufferViews(cmdList, isSO);

		int srvIndex = mMeshes[i]->GetSrvIndex();

		if (srvIndex >= 0)
		{
			srvGpuHandle = mSrvGPUAddress;
			srvGpuHandle.ptr += srvIndex * gCbvSrvUavDescriptorSize;
			cmdList->SetGraphicsRootDescriptorTable(rootSrvIndex, srvGpuHandle);
		}

		mMeshes[i]->Draw(cmdList, isSO);
	}
}

void SOParticleObject::SetLocalOffset(XMFLOAT3 offset)
{
	mLocalOffset = offset;
}