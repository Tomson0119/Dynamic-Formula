#include "stdafx.h"
#include "player.h"
#include "camera.h"
#include "inGameScene.h"

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

void Player::Update(float elapsedTime, float updateRate)
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

	GameObject::Update(elapsedTime, updateRate);
}

const float PhysicsPlayer::TransparentInterval = 0.3f;

PhysicsPlayer::PhysicsPlayer(UINT netID) 
	: Player(), 
	  mNetID(netID),
	  mItemNum(0)
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

	for (int i = 0; i < 2; ++i)
	{
		// Falloff는 전부 미터 단위임에 주의할 것
		mFrontLight[i].light.SetInfo(
			XMFLOAT3(0.6f, 0.6f, 0.6f),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT3(-0.3f, 0.0f, -1.0f),
			0.0f, 100.0f, 100.0f,
			0.0f, SPOT_LIGHT);;

		mFrontLight[i].light.pad0 = 1;

		mFrontLight[i].volumetric.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
		mFrontLight[i].volumetric.Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
		mFrontLight[i].volumetric.innerCosine = cos(6.0f);
		mFrontLight[i].volumetric.outerCosine = cos(7.0f);
		mFrontLight[i].volumetric.Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		mFrontLight[i].volumetric.Range = 30.0f;
		mFrontLight[i].volumetric.Type = SPOT_LIGHT;
		mFrontLight[i].volumetric.VolumetricStrength = 0.7f;
	}
}

PhysicsPlayer::~PhysicsPlayer()
{
}

void PhysicsPlayer::SetSpawnTransform(SC::packet_spawn_transform* pck)
{
	mSpawnFlag = true;
	mSpawnPosition.SetValue(pck->position[0], pck->position[1], pck->position[2]);
	mSpawnRotation.SetValue(
		pck->quaternion[0], pck->quaternion[1],
		pck->quaternion[2], pck->quaternion[3]);
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

void PhysicsPlayer::SetInvincibleOn(int duration)
{
	mInvincibleOnFlag = true;
	mInvincibleInterval = duration;
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

		mCamera->SetOffset(0.0f, 1.6f, -7.5f);
		mCamera->SetTimeLag(0.0f);
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
	XMFLOAT3 look = GetPosition();
	look.y = 0;

	if (mCamera->GetMode() == CameraMode::THIRD_PERSON_CAMERA)
		mCamera->LookAt(mCamera->GetPosition(), look, XMFLOAT3(0.0f, 1.0f, 0.0f));
}

void PhysicsPlayer::OnPreciseKeyInput(float Elapsed)
{
#ifdef STANDALONE
	if (mVehicle)
		mCurrentSpeed = mVehicle->getCurrentSpeedKmHour();

	mEngineForce = 0.0f;
	mBreakingForce = 10.0f;

	bool accel = false;

	if (mBoosterLeft > 0.0f)
	{
		mMaxEngineForce = mBoosterEngineForce;

		accel = true;
		mMaxSpeed = 400.0f;
		mEngineForce = mMaxEngineForce;
		mBoosterLeft -= Elapsed;

		mRimLightOn = true;
	}

	if (mBoosterLeft < 0.0f)
	{
		mMaxEngineForce = mBaseEngineForce;

		mMaxSpeed = 350.0f;
		mBoosterLeft = 0.0f;

		mRimLightOn = false;
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
		accel = true;
		if (mMaxSpeed > mCurrentSpeed)
			mEngineForce = mMaxEngineForce;
		else
		{
			mEngineForce = 0.0f;
		}
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		accel = true;
		if (-mMaxSpeed < mCurrentSpeed)
			mEngineForce = -mMaxEngineForce;
		else
		{
			mEngineForce = 0.0f;
		}
	}
	if (GetAsyncKeyState('Z') & 0x8000/*&&mItemNum>0*/)
	{
		if (mBoosterLeft == 0.0f)
			mBoosterLeft = mBoosterTime;

		//mItemNum-=1;
		//ItemUsingTime 처리
	}
	if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
	{
	}

	// if (GetAsyncKeyState('X') & 0x8000/*&&mItemNum>0*/)
	//{
	//
	//}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{	
		accel = true;
		for (int i = 0; i < 2; ++i)
		{
			if (mVehicle) mVehicle->getWheelInfo(i).m_frictionSlip = 1.5f;
		}

		for (int i = 2; i < 4; ++i)
		{
			if (mVehicle) mVehicle->getWheelInfo(i).m_frictionSlip = 0.0f;
		}

		float DriftLimit = 30.0f / 180.0f;
		float AngleLimit = 50.0f / 180.0f;

		auto camLook = mCamera->GetLook();
		camLook.y = 0.0f;
		camLook = Vector3::Normalize(camLook);

		auto playerLook = mLook;
		playerLook.y = 0.0f;
		playerLook = Vector3::Normalize(playerLook);

		float angle = acos(Vector3::Dot(camLook, playerLook) / (Vector3::Length(camLook) * Vector3::Length(playerLook)));

		if (DriftLimit < angle && mDriftGauge < 1.0f)
		{
			mDriftGauge += Elapsed / 2.0f;
		}

		if (AngleLimit < angle && mDriftGauge < 1.0f)
		{
			mBtRigidBody->setAngularVelocity(btVector3(0, 0, 0));
		}

		if (mDriftGauge > 1.0f)
		{
			mDriftGauge = 0.0f;
			if (mItemNum < 2)
				mItemNum++;
		}
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			if(mVehicle) mVehicle->getWheelInfo(i).m_frictionSlip = mWheelFriction;
		}
	}

	if (mBoosterLeft && mMaxSpeed < mCurrentSpeed)
		mEngineForce = mBoosterEngineForce;

	if (!accel)
	{
		mEngineForce = 0.f;
		mBreakingForce = 15.f;
	}

	for (int i = 0; i < 2; ++i)
	{
		if (mVehicle)
		{
			mVehicle->applyEngineForce(mEngineForce, i);
			mVehicle->setBrake(mBreakingForce, i);
		}
	}

	if (mVehicle)
	{
		int wheelIndex = 0;
		mVehicle->setSteeringValue(mVehicleSteering, wheelIndex);
		wheelIndex = 1;
		mVehicle->setSteeringValue(mVehicleSteering, wheelIndex);
	}
#endif

#ifndef STANDALONE
	if (mBoosterLeft > 0)
	{
		mBoosterLeft -= Elapsed;
	}
	else if(mBoosterLeft < 0)
	{
		mBoosterLeft = 0.0f;
		mRimLightOn = false;
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
#endif
}

void PhysicsPlayer::Update(float elapsedTime, float updateRate)
{
	if (mSpawnFlag)
	{
		mSpawnFlag = false;
		SetPosition(mSpawnPosition.GetXMFloat3());
		SetQuaternion(mSpawnRotation.GetXMFloat4());
		if (mCamera)
		{
			mCamera->SetPosition(mPosition);
			mCamera->Update(elapsedTime);
		}
	}

	GameObject::Update(elapsedTime, updateRate);
	
	for (int i = 0; i < 4; ++i)
		mWheel[i]->SetTransparent(mTransparentOn);

	for (int i = 0; i < 4; ++i)
	{
		if (mVehicle)
		{
			btTransform wheelTransform = mVehicle->getWheelTransformWS(i);
			mWheel[i]->UpdatePosition(elapsedTime, wheelTransform);
		}
		else
		{
			mWheel[i]->Update(elapsedTime, updateRate);
			if(i < 2) mWheel[i]->SetSteeringAngle(mVehicleSteering);
		}
	}

	UpdateFrontLight();

	//btVector3 linearVel = mBtRigidBody->getLinearVelocity();
	//mBtRigidBody->applyCentralImpulse(btVector3(0, -linearVel.length() / 10, 0));

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

	UpdateInvincibleState(elapsedTime);
}

void PhysicsPlayer::UpdateInvincibleState(float elapsed)
{
	if (mInvincibleOnFlag)
	{
		mInvincibleOnFlag = false;
		mInvincible = true;
		mInvincibleDuration = (float)mInvincibleInterval / FIXED_FLOAT_LIMIT;
	}

	if (mInvincible)
	{
		mInvincibleDuration -= elapsed;
		mTransparentTime -= elapsed;
		if (mTransparentTime <= 0.f)
		{
			mTransparentOn = !mTransparentOn;
			mTransparentTime = TransparentInterval;
		}
		if (mInvincibleDuration <= 0.0f)
		{
			mInvincibleDuration = 0.0f;
			mTransparentTime = TransparentInterval;
			mTransparentOn = false;
			mInvincible = false;
		}
	}
}

void PhysicsPlayer::SetCubemapSrv(ID3D12GraphicsCommandList* cmdList, UINT srvIndex)
{
	ID3D12DescriptorHeap* descHeaps[] = { mSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	auto gpuStart = mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	gpuStart.ptr += gCbvSrvUavDescriptorSize * (1 - mCurrentRenderTarget);
	cmdList->SetGraphicsRootDescriptorTable(srvIndex, gpuStart);
}

void PhysicsPlayer::BuildRigidBody(const std::shared_ptr<BulletWrapper>& physics)
{
	auto dynamicsWorld = physics->GetDynamicsWorld();

	mOOBB.Extents = { 10.0f / 6.0f, 4.0f / 6.0f, 14.0f / 6.0f };

	XMFLOAT3 vehicleExtents = mOOBB.Extents;
	XMFLOAT3 wheelExtents = mWheel[0]->GetBoundingBox().Extents;

	btTransform btCarTransform;
	btCarTransform.setIdentity();
	btCarTransform.setOrigin(btVector3(mPosition.x, mPosition.y, mPosition.z));
	btCarTransform.setRotation(btQuaternion(mQuaternion.x, mQuaternion.y, mQuaternion.z, mQuaternion.w));

	LoadConvexHullShape(L"Models\\Car_Body_Convex_Hull.obj", physics);

	mBtRigidBody = physics->CreateRigidBody(1000.0f, btCarTransform, mBtCollisionShape);
	mVehicleRayCaster = std::make_shared<btDefaultVehicleRaycaster>(dynamicsWorld);
	mVehicle = std::make_shared<btRaycastVehicle>(mTuning, mBtRigidBody, mVehicleRayCaster.get());
	mBtRigidBody = mVehicle->getRigidBody();

	mBtRigidBody->setActivationState(DISABLE_DEACTIVATION);
	//mBtRigidBody->setGravity(btVector3(0, -20, 0));
	dynamicsWorld->addVehicle(mVehicle.get());

	mVehicle->setCoordinateSystem(0, 1, 2);

	btVector3 wheelDirectionCS0(0, -1, 0);
	btVector3 wheelAxleCS(-1, 0, 0);

	float wheelWidth = wheelExtents.x;
	float wheelRadius = wheelExtents.z;
	float wheelFriction = mWheelFriction;
	float suspensionStiffness = 20.0f;
	float suspensionDamping = 2.5f;
	float suspensionCompression = 4.4f;
	float rollInfluence = 0.001f;  //1.0f;

	// 앞바퀴
	bool isFrontWheel = true;
	float connectionHeight = -0.4f;

	btVector3 connectionPointCS0(vehicleExtents.x - 0.4f, connectionHeight, vehicleExtents.z - 0.5f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.1f, wheelRadius, mTuning, isFrontWheel);

	connectionPointCS0 = btVector3(-vehicleExtents.x + 0.4f, connectionHeight, vehicleExtents.z - 0.5f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.1f, wheelRadius, mTuning, isFrontWheel);

	// 뒷바퀴
	isFrontWheel = false;

	connectionPointCS0 = btVector3(vehicleExtents.x - 0.4f, connectionHeight, -vehicleExtents.z + 0.5f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.1f, wheelRadius, mTuning, isFrontWheel);

	connectionPointCS0 = btVector3(-vehicleExtents.x + 0.4f, connectionHeight, -vehicleExtents.z + 0.5f);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.1f, wheelRadius, mTuning, isFrontWheel);

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

void PhysicsPlayer::SetCorrectionTransform(SC::packet_player_transform* pck, float latency)
{
	mProgressMut.lock();
	mProgress = 0.0f;
	mProgressMut.unlock();

	mPrevOrigin = mCorrectionOrigin;
	mPrevQuat = mCorrectionQuat;

	mCorrectionOrigin.SetValue(
		pck->position[0],
		pck->position[1], 
		pck->position[2]);

	mCorrectionOrigin.Extrapolate(
		pck->linear_vel[0],
		pck->linear_vel[1],
		pck->linear_vel[2],
		latency);

	mCorrectionQuat.SetValue(
		pck->quaternion[0],
		pck->quaternion[1],
		pck->quaternion[2],
		pck->quaternion[3]);

	mCorrectionQuat.Extrapolate(
		pck->angular_vel[0],
		pck->angular_vel[1],
		pck->angular_vel[2],
		latency);
	
	mLinearVelocity.SetValue(pck->linear_vel[0], pck->linear_vel[1], pck->linear_vel[2]);
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
	scene->RenderPipelines(cmdList, cubemapIndex + 4, true);

	// resource barrier
	cmdList->ResourceBarrier(1, &Extension::ResourceBarrier(
		mCubeMap[mCurrentRenderTarget]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void PhysicsPlayer::RemoveObject(btDiscreteDynamicsWorld& dynamicsWorld, Pipeline& pipeline)
{
	GameObject::RemoveObject(dynamicsWorld, pipeline);

	if (mVehicle)
	{
		dynamicsWorld.removeVehicle(mVehicle.get());
	}

	int remove_count = 0;
	auto& objs = pipeline.GetRenderObjects();
	for (auto iter = objs.begin(); iter != objs.end();)
	{
		bool wheel_removed = false;
		for (int k = 0; k < 4; ++k)
		{
			auto wheel = GetWheel(k);
			if (wheel == iter->get())
			{
				iter = pipeline.DeleteObject(iter);
				remove_count++;
				wheel_removed = true;
				break;
			}
		}
		if (remove_count == 4)
			break;

		if (!wheel_removed)
			iter++;
	}
}

void PhysicsPlayer::UpdateFrontLight()
{
	XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&mQuaternion));
	for (int i = 0; i < 2; ++i)
	{
		mFrontLight[i].light.Position = Vector3::Transform(mLightOffset[i], XMLoadFloat4x4(&mWorld));
		mFrontLight[i].volumetric.Position = Vector3::Transform(mLightOffset[i], XMLoadFloat4x4(&mWorld));
		
		auto dir = mLook;
		dir.y -= 0.3f;
		mFrontLight[i].light.Direction = dir;
		mFrontLight[i].volumetric.Direction = dir;
	}
}

WheelObject::WheelObject(GameObject& parent) 
	: GameObject(), mParent{ parent }, 
	  mLocalOffset{}, mSteeringAngle{}, mIsStandAlone{ false }
{
	mMotionBlurOn = false;
}

WheelObject::~WheelObject()
{
}

void WheelObject::UpdatePosition(float Elapsed, const btTransform& wheelTransform)
{
	mIsStandAlone = true;

	btScalar m[16];
	wheelTransform.getOpenGLMatrix(m);
	mOldWorld = mWorld;
	mWorld = Matrix4x4::glMatrixToD3DMatrix(m);

	mPosition.x = mWorld(3, 0);
	mPosition.y = mWorld(3, 1);
	mPosition.z = mWorld(3, 2);

	mLook = XMFLOAT3(mWorld._31, mWorld._32, mWorld._33);
}

void WheelObject::SetSteeringAngle(float angle)
{
	mSteeringAngle = angle;
}

void WheelObject::Update(float elapsedTime, float updateRate)
{
	if (mIsStandAlone == false)
	{
		mPosition = mLocalOffset;

		auto linearVelocity = mParent.GetLinearVelocity().GetBtVector3();

		XMFLOAT3 rotation = { linearVelocity.length(), mSteeringAngle, 0 };

		mQuaternion = Vector4::RotateQuaternionRollPitchYaw(rotation);

		RotateDirectionVectors();
		UpdateTransform();

		mWorld = Matrix4x4::Multiply(mWorld, mParent.GetWorld());
	}
}