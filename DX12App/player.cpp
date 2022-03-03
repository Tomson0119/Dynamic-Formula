#include "stdafx.h"
#include "player.h"
#include "camera.h"

Player::Player()
	: GameObject()
{

}

Player::~Player()
{
}

void Player::Walk(float dist, bool updateVelocity)
{
	XMFLOAT3 shift = { 0.0f,0.0f,0.0f };
	shift = Vector3::Add(shift, mLook, dist);
	Move(shift, updateVelocity);
}

void Player::Strafe(float dist, bool updateVelocity)
{
	XMFLOAT3 shift = { 0.0f,0.0f,0.0f };
	shift = Vector3::Add(shift, mRight, dist);
	Move(shift, updateVelocity);
}

void Player::Upward(float dist, bool updateVelocity)
{
	XMFLOAT3 shift = { 0.0f,0.0f,0.0f };
	shift = Vector3::Add(shift, XMFLOAT3(0.0f, 1.0f, 0.0f), dist);
	Move(shift, updateVelocity);
}

void Player::Move(XMFLOAT3& shift, bool updateVelocity)
{
	if (updateVelocity)
		mVelocity = Vector3::Add(mVelocity, shift);
	else
	{
		mPosition = Vector3::Add(mPosition, shift);
		if(mCamera) mCamera->Move(shift.x, shift.y, shift.z);
	}
}

void Player::RotateY(float angle)
{
	if (mCamera)
	{
		switch (mCamera->GetMode())
		{
		case CameraMode::FIRST_PERSON_CAMERA:
			mCamera->RotateY(angle);
			break;

		case CameraMode::THIRD_PERSON_CAMERA:
			mCamera->RotateY(angle);
			break;

		case CameraMode::TOP_DOWN_CAMERA:
			break;
		}
	}
	GameObject::RotateY(angle);
}

void Player::Pitch(float angle)
{
	if (mCamera)
	{
		switch (mCamera->GetMode())
		{
		case CameraMode::FIRST_PERSON_CAMERA:
			XMFLOAT3 look = { 0.0f, GetLook().y, 1.0f };
			look = Vector3::Normalize(look);
			XMFLOAT3 forward = { 0.0f, 0.0f, 1.0f };

			float radian = std::acosf(Vector3::Dot(look, forward));
			float degree = XMConvertToDegrees(radian);

			XMFLOAT3 cross = Vector3::Cross(look, forward);
			bool lookingUp = (cross.x > 0.0f) ? true : false;
			
			if (degree < 32.0f ||
				lookingUp && angle > 0.0f ||
				!lookingUp && angle < 0.0f)  // 각도 제한
				GameObject::Pitch(angle);
			
			break;
		}
	}
}

Camera* Player::ChangeCameraMode(int cameraMode)
{	
	if (mCamera && (int)mCamera->GetMode() == cameraMode)
		return nullptr;  // 같은 모드이면 아무것도 보내지 않는다.

	Camera* newCamera = nullptr;
	switch (cameraMode)
	{
	case (int)CameraMode::FIRST_PERSON_CAMERA:
		newCamera = new FirstPersonCamera();
		break;

	case (int)CameraMode::THIRD_PERSON_CAMERA:
		newCamera = new ThirdPersonCamera();
		break;

	case (int)CameraMode::TOP_DOWN_CAMERA:
		newCamera = new TopDownCamera();
		break;
	}

	if (newCamera)
	{
		newCamera->SetMode(cameraMode);
		newCamera->SetPlayer(this);		
	}

	if (mCamera &&  // 정면을 바라보도록 수정한다.
		mCamera->GetMode() == CameraMode::FIRST_PERSON_CAMERA)
		mLook.y = 0.0f;

	return newCamera;
}

void Player::Update(float elapsedTime)
{
	mVelocity = Vector3::Add(mVelocity, mGravity);

	// Velocity 값이 너무 커지지 않게 조정한다.
	float length = sqrtf(mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z);
	if (length > mMaxVelocityXZ)
	{
		mVelocity.x *= (mMaxVelocityXZ / length);
		mVelocity.z *= (mMaxVelocityXZ / length);
	}	

	length = sqrtf(mVelocity.y * mVelocity.y);
	if (length > mMaxVelocityY)
		mVelocity.y *= (mMaxVelocityY / length);

	// 타이머 시간의 흐름에 맞추어 이동한다.
	XMFLOAT3 velocity = Vector3::ScalarProduct(mVelocity, elapsedTime);
	Move(velocity, false);

	if(mPlayerUpdateContext)
		OnPlayerUpdate(elapsedTime);
	if (mCamera && mCameraUpdateContext) {
		OnCameraUpdate(elapsedTime);
		mCamera->UpdateViewMatrix();
	}

	length = Vector3::Length(mVelocity);
	float deceleration = (mFriction * elapsedTime);
	if (deceleration > length) deceleration = length;
	velocity = Vector3::ScalarProduct(mVelocity, -deceleration);
	mVelocity = Vector3::Add(mVelocity, Vector3::Normalize(velocity));

	GameObject::Update(elapsedTime);
}


/////////////////////////////////////////////////////////////////////////////////////
//
TerrainPlayer::TerrainPlayer(void* context)
	: Player()
{
	TerrainObject* terrain = (TerrainObject*)context;
	float xPos = terrain->GetWidth() * 0.5f;
	float zPos = terrain->GetDepth() * 0.5f;
	float yPos = terrain->GetHeight(xPos, zPos) + 30.0f;
	SetPosition(xPos, yPos, zPos);

	SetPlayerContext(terrain);
	SetCameraContext(terrain);
}

TerrainPlayer::~TerrainPlayer()
{
}

Camera* TerrainPlayer::ChangeCameraMode(int cameraMode)
{
	if (mCamera && cameraMode == (int)mCamera->GetMode())
		return nullptr;

	mCamera = Player::ChangeCameraMode(cameraMode);

	switch (mCamera->GetMode())
	{
	case CameraMode::FIRST_PERSON_CAMERA:
		mFriction = 50.0f;
		mGravity = { 0.0f, -9.8f, 0.0f };
		mMaxVelocityXZ = 25.5f;
		mMaxVelocityY = 40.0f;

		mCamera->SetOffset(0.0f, 2.0f, 0.0f);
		mCamera->SetTimeLag(0.0f);
		break;

	case CameraMode::THIRD_PERSON_CAMERA:
		mFriction = 50.0f;
		mGravity = { 0.0f, -9.8f, 0.0f };
		mMaxVelocityXZ = 25.5f;
		mMaxVelocityY = 40.0f;

		mCamera->SetOffset(0.0f, 5.0f, -10.0f);
		mCamera->SetTimeLag(0.25f);
		break;

	case CameraMode::TOP_DOWN_CAMERA:
		mFriction = 50.0f;
		mGravity = { 0.0f, -9.8f, 0.0f };
		mMaxVelocityXZ = 25.5f;
		mMaxVelocityY = 40.0f;

		mCamera->SetOffset(-50.0f, 50.0f, -50.0f);
		mCamera->SetTimeLag(0.25f);
		break;
	}

	mCamera->SetPosition(Vector3::Add(mPosition, mCamera->GetOffset()));
	mCamera->Update(1.0f);

	return mCamera;
}

void TerrainPlayer::OnPlayerUpdate(float elapsedTime)
{
	XMFLOAT3 playerPos = GetPosition();
	TerrainObject* terrain = (TerrainObject*)mPlayerUpdateContext;

	float playerHalfHeight = mOOBB.Extents.y * 0.5f;
	
	float height = terrain->GetHeight(playerPos.x, playerPos.z) + playerHalfHeight - 0.5f;

	if (playerPos.y < height)
	{
		XMFLOAT3 playerVelocity = GetVelocity();
		playerVelocity.y = 0.0f;
		SetVelocity(playerVelocity);
		playerPos.y = height;
		SetPosition(playerPos);
	}
}

void TerrainPlayer::OnCameraUpdate(float elapsedTime)
{
	XMFLOAT3 cameraPos = mCamera->GetPosition();
	TerrainObject* terrain = (TerrainObject*)mCameraUpdateContext;

	float height = terrain->GetHeight(cameraPos.x, cameraPos.z) + 5.0f;
	
	if (cameraPos.y <= height)
	{
		cameraPos.y = height;
		mCamera->SetPosition(cameraPos);
	}
	if (mCamera->GetMode() == CameraMode::THIRD_PERSON_CAMERA)
		mCamera->LookAt(mCamera->GetPosition(), GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
}

PhysicsPlayer::PhysicsPlayer(UINT netID) : Player(), mNetID(netID), mRemoveFlag{ false }
{
	mViewPort = { 0.0f, 0.0f, (float)mCubeMapSize, (float)mCubeMapSize, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (LONG)mCubeMapSize, (LONG)mCubeMapSize };

	mMotionBlurOn = false;
	mCubemapOn = true;

	for (std::unique_ptr<Camera>& camera : mCameras)
	{
		camera = std::make_unique<Camera>();
		camera->SetLens(0.5f * Math::PI, 1.0f, 0.1f, 100.0f);
	}
}

PhysicsPlayer::~PhysicsPlayer()
{
}

void PhysicsPlayer::OnPreciseKeyInput(float Elapsed)
{
	mCurrentSpeed = mVehicle->getCurrentSpeedKmHour();
	
	mEngineForce = 0.0f;
	if (mBoosterLeft > 0.0f)
	{
		mMaxSpeed = 1500.0f;
		mBoosterLeft -= Elapsed;
	}
	
	if(mBoosterLeft < 0.0f)
	{
		mMaxSpeed = 1000.0f;
		mBoosterLeft = 0.0f;
	}

	if (mVehicleSteering > 0)
	{
		mVehicleSteering -= mSteeringIncrement * Elapsed;
		if (mVehicleSteering < 0)
		{
			mVehicleSteering = 0;
		}
	}

	else if (mVehicleSteering < 0)
	{
		mVehicleSteering += mSteeringIncrement * Elapsed;
		if (mVehicleSteering > 0)
		{
			mVehicleSteering = 0;
		}
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		mVehicleSteering -= mSteeringIncrement * 2 * Elapsed;
		if (mVehicleSteering < -mSteeringClamp)
			mVehicleSteering = -mSteeringClamp;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		mVehicleSteering += mSteeringIncrement * 2 * Elapsed;
		if (mVehicleSteering > mSteeringClamp)
			mVehicleSteering = mSteeringClamp;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		if (mCurrentSpeed < 0.0f)
			mEngineForce = mMaxEngineForce * 1.5f;
		else if (mMaxSpeed > mCurrentSpeed)
			mEngineForce = mMaxEngineForce;
		else
			mEngineForce = 0.0f;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		if (mCurrentSpeed > 0.0f)
			mEngineForce = -mMaxEngineForce * 1.5f;
		else if (-mMaxSpeed < mCurrentSpeed)
			mEngineForce = -mMaxEngineForce;
		else
			mEngineForce = 0.0f;
	}
	if (GetAsyncKeyState('Z') & 0x8000)
	{
		if (mBoosterLeft == 0.0f)
			mBoosterLeft = mBoosterTime;
	}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		for (int i = 2; i < 4; ++i)
		{
			mVehicle->getWheelInfo(i).m_frictionSlip = 4.0f;
		}

		float Epsilon = 30.0f;

		XMFLOAT3 forward = Vector3::btVectorToXM(mVehicle->getForwardVector());
		float angle = acos(Vector3::Dot(mLook, forward) / (Vector3::Length(mLook) * Vector3::Length(forward)));

		if (Epsilon < angle && mDriftGauge < 100.0f)
		{
			mDriftGauge += Elapsed * 10.0f;
		}
		if (mDriftGauge > 100.0f)
		{
			mDriftGauge = 0.0f;
			if(mItemNum < 2)
				mItemNum++;
		}
	}
	else
	{
		for (int i = 2; i < 4; ++i)
		{
			mVehicle->getWheelInfo(i).m_frictionSlip = 25.0f;
		}
	}

	if (mBoosterLeft && mMaxSpeed < mCurrentSpeed)
		mEngineForce = mBoosterEngineForce;

	for (int i = 0; i < 2; ++i)
	{
		mVehicle->applyEngineForce(mEngineForce, i);
	}
	
	int wheelIndex = 0;
	mVehicle->setSteeringValue(mVehicleSteering, wheelIndex);
	wheelIndex = 1;
	mVehicle->setSteeringValue(mVehicleSteering, wheelIndex);

}


Camera* PhysicsPlayer::ChangeCameraMode(int cameraMode)
{
	if (mCamera && cameraMode == (int)mCamera->GetMode())
		return nullptr;

	mCamera = Player::ChangeCameraMode(cameraMode);

	switch (mCamera->GetMode())
	{
	case CameraMode::FIRST_PERSON_CAMERA:
		mFriction = 50.0f;
		mGravity = { 0.0f, -9.8f, 0.0f };
		mMaxVelocityXZ = 25.5f;
		mMaxVelocityY = 40.0f;

		mCamera->SetOffset(0.0f, 2.0f, 0.0f);
		mCamera->SetTimeLag(0.0f);
		break;

	case CameraMode::THIRD_PERSON_CAMERA:
		mFriction = 50.0f;
		mGravity = { 0.0f, -9.8f, 0.0f };
		mMaxVelocityXZ = 25.5f;
		mMaxVelocityY = 40.0f;

		mCamera->SetOffset(0.0f, 20.0f, -50.0f);
		mCamera->SetTimeLag(0.25f);
		break;

	case CameraMode::TOP_DOWN_CAMERA:
		mFriction = 50.0f;
		mGravity = { 0.0f, -9.8f, 0.0f };
		mMaxVelocityXZ = 25.5f;
		mMaxVelocityY = 40.0f;

		mCamera->SetOffset(-50.0f, 50.0f, -50.0f);
		mCamera->SetTimeLag(0.25f);
		break;
	}

	mCamera->SetPosition(Vector3::Add(mPosition, mCamera->GetOffset()));
	mCamera->Update(1.0f);

	return mCamera;
}


void PhysicsPlayer::OnCameraUpdate(float elapsedTime)
{
	XMFLOAT3 cameraPos = mCamera->GetPosition();
	
	mCamera->SetPosition(cameraPos);

	if (mCamera->GetMode() == CameraMode::THIRD_PERSON_CAMERA)
		mCamera->LookAt(mCamera->GetPosition(), XMFLOAT3(GetPosition().x, GetPosition().y, GetPosition().z), XMFLOAT3(0.0f, 1.0f, 0.0f));
}


void PhysicsPlayer::OnPlayerUpdate(float elapsedTime)
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

void PhysicsPlayer::Update(float elapsedTime)
{
	btScalar m[16];
	btTransform btMat;
	mVehicle->getRigidBody()->getMotionState()->getWorldTransform(btMat);
	btMat.getOpenGLMatrix(m);

	mOldWorld = mWorld;
	mWorld = Matrix4x4::glMatrixToD3DMatrix(m);
	UpdateBoundingBox();

	OnPlayerUpdate(elapsedTime);

	if (mCamera) {
		OnCameraUpdate(elapsedTime);
		mCamera->UpdateViewMatrix();
	}

	for (int i = 0; i < 4; ++i)
	{
		btTransform wheelTransform = mVehicle->getWheelTransformWS(i);
		mWheel[i]->UpdateRigidBody(elapsedTime, wheelTransform);
	}

	mLook = Vector3::Normalize(mLook);
	mUp = Vector3::Normalize(Vector3::Cross(mLook, mRight));
	mRight = Vector3::Cross(mUp, mLook);

	if (mBoosterLeft > 0.0f)
	{
		if (mFovCoefficient < 1.1f)
			mFovCoefficient += elapsedTime * 5.0f * (1.1f - mFovCoefficient);
		else
			mFovCoefficient = 1.1f;
	}
	else
	{
		if (mFovCoefficient > 1.0f)
			mFovCoefficient -= elapsedTime * 5.0f * (mFovCoefficient - 1.0f);
		else
			mFovCoefficient = 1.0f;
	}

	if (mCamera)
	{
		mCamera->SetFovCoefficient(mFovCoefficient);
		mCamera->SetLens(mCamera->GetAspect());
	}
}

void PhysicsPlayer::SetMesh(const std::shared_ptr<Mesh>& bodyMesh, const std::shared_ptr<Mesh>& wheelMesh, std::shared_ptr<BulletWrapper> physics)
{
	GameObject::SetMesh(bodyMesh);

	BuildRigidBody(physics);
}

void PhysicsPlayer::SetMesh(const std::shared_ptr<Mesh>& Mesh)
{
	GameObject::SetMesh(Mesh);
}

void PhysicsPlayer::SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex)
{
	ID3D12DescriptorHeap* descHeaps[] = { mSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	auto gpuStart = mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	gpuStart.ptr += gCbvSrvUavDescriptorSize * (1 - mCurrentRenderTarget);
	cmdList->SetGraphicsRootDescriptorTable(srvIndex, gpuStart);
}

void PhysicsPlayer::BuildRigidBody(std::shared_ptr<BulletWrapper> physics)
{
	auto dynamicsWorld = physics->GetDynamicsWorld();

	mOOBB.Extents = { 10.0f, 4.0f, 14.0f };

	XMFLOAT3 vehicleExtents = mOOBB.Extents;
	XMFLOAT3 wheelExtents = mWheel[0]->GetBoundingBox().Extents;

	btTransform btCarTransform;
	btCarTransform.setIdentity();
	btCarTransform.setOrigin(btVector3(mPosition.x, mPosition.y, mPosition.z));

	LoadConvexHullShape(L"Models\\Car_Body_Convex_Hull.obj", physics);

	mBtRigidBody = physics->CreateRigidBody(1000.0f, btCarTransform, mBtCollisionShape);
	mVehicleRayCaster = std::make_shared<btDefaultVehicleRaycaster>(dynamicsWorld.get());
	mVehicle = std::make_shared<btRaycastVehicle>(mTuning, mBtRigidBody, mVehicleRayCaster.get());

	mBtRigidBody->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addVehicle(mVehicle.get());

	mVehicle->setCoordinateSystem(0, 1, 2);

	btVector3 wheelDirectionCS0(0, -1, 0);
	btVector3 wheelAxleCS(-1, 0, 0);

	float wheelWidth = wheelExtents.x;
	float wheelRadius = wheelExtents.z;
	float wheelFriction = 26.0f;
	float suspensionStiffness = 20.f;
	float suspensionDamping = 2.3f;
	float suspensionCompression = 4.4f;
	float rollInfluence = 0.01f;  //1.0f;

	// 앞바퀴
	bool isFrontWheel = true;
	float connectionHeight = -0.9f;

	btVector3 connectionPointCS0(vehicleExtents.x - 2.5f, connectionHeight, vehicleExtents.z - 2.8f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.6, wheelRadius, mTuning, isFrontWheel);

	connectionPointCS0 = btVector3(-vehicleExtents.x + 2.5f, connectionHeight, vehicleExtents.z - 2.8f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.6, wheelRadius, mTuning, isFrontWheel);

	// 뒷바퀴
	isFrontWheel = false;

	connectionPointCS0 = btVector3(vehicleExtents.x - 2.3f, connectionHeight, -vehicleExtents.z + 2.6f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.6, wheelRadius, mTuning, isFrontWheel);

	connectionPointCS0 = btVector3(-vehicleExtents.x + 2.3f, connectionHeight, -vehicleExtents.z + 2.6f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.6, wheelRadius, mTuning, isFrontWheel);

	for (int i = 0; i < mVehicle->getNumWheels(); i++)
	{
		btWheelInfo& wheel = mVehicle->getWheelInfo(i);
		wheel.m_suspensionStiffness = suspensionStiffness;
		wheel.m_wheelsDampingRelaxation = suspensionDamping;
		wheel.m_wheelsDampingCompression = suspensionCompression;
		wheel.m_frictionSlip = wheelFriction;
		wheel.m_rollInfluence = rollInfluence;
	}
}

void PhysicsPlayer::BuildDsvRtvView(ID3D12Device* device)
{
	assert(mCubeMapSize > 0 && "CubeMapSize is not assigned.");

	device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			1,
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
		IID_PPV_ARGS(&mDsvDescriptorHeap));

	device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			RtvCounts,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
		IID_PPV_ARGS(&mRtvDescriptorHeap));

	device->CreateDescriptorHeap(
		&Extension::DescriptorHeapDesc(
			2,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
		IID_PPV_ARGS(&mSrvDescriptorHeap));

	auto rtvHandle = mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvHandle = mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto srvHandle = mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f,0.0f,0.0f,1.0f} };

	for (int i = 0; i < 2; ++i)
	{
		std::unique_ptr<Texture> textureCube = std::make_unique<Texture>();
		textureCube->SetDimension(D3D12_SRV_DIMENSION_TEXTURECUBE);
		textureCube->CreateTexture(
			device, mCubeMapSize, mCubeMapSize, RtvCounts / 2, 1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ, &clearValue);

		mCubeMap[i] = std::move(textureCube);


		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		rtvDesc.Texture2DArray.ArraySize = 1;

		for (int j = i * 6; j < (i + 1) * 6; j++)
		{
			mRtvCPUDescriptorHandles[j] = rtvHandle;
			rtvDesc.Texture2DArray.FirstArraySlice = j % 6;
			device->CreateRenderTargetView(mCubeMap[i]->GetResource(), &rtvDesc, mRtvCPUDescriptorHandles[j]);
			rtvHandle.ptr += gRtvDescriptorSize;
		}
	}
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	mDepthStencilBuffer = CreateTexture2DResource(
		device,
		mCubeMapSize, mCubeMapSize,
		1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue);

	mDsvCPUDescriptorHandle = dsvHandle;
	device->CreateDepthStencilView(mDepthStencilBuffer.Get(), NULL, mDsvCPUDescriptorHandle);
	
	for (int i = 0; i < 2; ++i)
	{
		mSrvCPUDescriptorHandle = srvHandle;
		device->CreateShaderResourceView(mCubeMap[i]->GetResource(), &mCubeMap[i]->ShaderResourceView(), mSrvCPUDescriptorHandle);
		srvHandle.ptr += gCbvSrvUavDescriptorSize;
	}
}

void PhysicsPlayer::BuildCameras()
{
	static XMFLOAT3 lookAts[RtvCounts / 2] =
	{
		XMFLOAT3(+1.0f, 0.0f,  0.0f),
		XMFLOAT3(-1.0f, 0.0f,  0.0f),
		XMFLOAT3(0.0f, +1.0f,  0.0f),
		XMFLOAT3(0.0f, -1.0f,  0.0f),
		XMFLOAT3(0.0f,  0.0f, +1.0f),
		XMFLOAT3(0.0f,  0.0f, -1.0f)
	};

	static XMFLOAT3 ups[RtvCounts / 2] =
	{
		XMFLOAT3(0.0f, +1.0f,  0.0f),
		XMFLOAT3(0.0f, +1.0f,  0.0f),
		XMFLOAT3(0.0f,  0.0f, -1.0f),
		XMFLOAT3(0.0f,  0.0f, +1.0f),
		XMFLOAT3(0.0f, +1.0f,  0.0f),
		XMFLOAT3(0.0f, +1.0f,  0.0f)
	};

	XMFLOAT3 pos = GetPosition();
	for (int i = 0; i < RtvCounts / 2; i++)
	{
		mCameras[i]->SetPosition(pos);
		mCameras[i]->LookAt(pos, Vector3::Add(lookAts[i % 6], pos), ups[i % 6]);
		mCameras[i]->UpdateViewMatrix();
	}
}

void PhysicsPlayer::PreDraw(ID3D12GraphicsCommandList* cmdList, InGameScene* scene, const UINT& cubemapIndex)
{
	BuildCameras();

	cmdList->RSSetViewports(1, &mViewPort);
	cmdList->RSSetScissorRects(1, &mScissorRect);

	// resource barrier
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mCubeMap[mCurrentRenderTarget]->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	FLOAT clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	cmdList->ClearRenderTargetView(mRtvCPUDescriptorHandles[cubemapIndex + (mCurrentRenderTarget * 6)], clearValue, 0, NULL);
	cmdList->ClearDepthStencilView(mDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	cmdList->OMSetRenderTargets(1, &mRtvCPUDescriptorHandles[cubemapIndex + (mCurrentRenderTarget * 6)], TRUE, &mDsvCPUDescriptorHandle);

	scene->UpdateCameraConstant(cubemapIndex + 4, mCameras[cubemapIndex].get());
	scene->RenderPipelines(cmdList, mCameras[cubemapIndex].get(), cubemapIndex + 4);

	// resource barrier
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mCubeMap[mCurrentRenderTarget]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

WheelObject::WheelObject() : GameObject()
{
	mMotionBlurOn = false;
}

WheelObject::~WheelObject()
{
}

void WheelObject::UpdateRigidBody(const float& Elapsed, const btTransform& wheelTransform)
{
	btScalar m[16];
	wheelTransform.getOpenGLMatrix(m);
	mOldWorld = mWorld;
	mWorld = Matrix4x4::glMatrixToD3DMatrix(m);

	mPosition.x = mWorld(3, 0);
	mPosition.y = mWorld(3, 1);
	mPosition.z = mWorld(3, 2);

	mLook = XMFLOAT3(mWorld._31, mWorld._32, mWorld._33);
}