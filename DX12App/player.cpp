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

void Player::Update(float elapsedTime, XMFLOAT4X4* parent)
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

	GameObject::Update(elapsedTime, parent);
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

PhysicsPlayer::PhysicsPlayer() : Player()
{
}

void PhysicsPlayer::SetMesh(const std::shared_ptr<Mesh>& mesh, std::shared_ptr<btDiscreteDynamicsWorld> btDynamicsWorld)
{
	XMFLOAT3 vehicleExtents = mesh.get()->mOOBB.Extents;
	//auto wheelExtents = pWheelMesh.get()[0].GetBoundingBox().Extents;
	XMFLOAT3 wheelExtents = {0.1f, 0.1f, 0.1f};

	btCollisionShape* chassisShape = new btBoxShape(btVector3(vehicleExtents.x, vehicleExtents.y, vehicleExtents.z));

	btTransform btCarTransform;
	btCarTransform.setIdentity();
	btCarTransform.setOrigin(btVector3(mPosition.x, mPosition.y, mPosition.z));

	mBtRigidBody = BulletHelper::CreateRigidBody(1000.0f, btCarTransform, chassisShape, btDynamicsWorld);

	mVehicleRayCaster = new btDefaultVehicleRaycaster(btDynamicsWorld.get());
	mVehicle = new btRaycastVehicle(mTuning, mBtRigidBody, mVehicleRayCaster);

	mBtRigidBody->setActivationState(DISABLE_DEACTIVATION);
	btDynamicsWorld->addVehicle(mVehicle);

	float connectionHeight = 0.0f;

	mVehicle->setCoordinateSystem(0, 1, 2);

	btVector3 wheelDirectionCS0(0, -1, 0);
	btVector3 wheelAxleCS(-1, 0, 0);

	float wheelWidth = wheelExtents.x;
	float wheelRadius = wheelExtents.z;
	float wheelFriction = 500;  //BT_LARGE_FLOAT;
	float suspensionStiffness = 20.f;
	float suspensionDamping = 2.3f;
	float suspensionCompression = 4.4f;
	float rollInfluence = 0.01f;  //1.0f;

	// 앞바퀴
	bool isFrontWheel = true;

	btVector3 connectionPointCS0(vehicleExtents.x - (0.3 * wheelWidth), connectionHeight, vehicleExtents.z - wheelRadius);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.6, wheelRadius, mTuning, isFrontWheel);

	connectionPointCS0 = btVector3(-vehicleExtents.x + (0.3 * wheelWidth), connectionHeight, vehicleExtents.z - wheelRadius);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.6, wheelRadius, mTuning, isFrontWheel);

	// 뒷바퀴
	isFrontWheel = false;

	connectionPointCS0 = btVector3(-vehicleExtents.x + (0.3 * wheelWidth), connectionHeight, -vehicleExtents.z + wheelRadius);
	mVehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, 0.6, wheelRadius, mTuning, isFrontWheel);

	connectionPointCS0 = btVector3(vehicleExtents.x - (0.3 * wheelWidth), connectionHeight, -vehicleExtents.z + wheelRadius);
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
