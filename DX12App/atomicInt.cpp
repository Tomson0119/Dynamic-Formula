#include "stdafx.h"
#include "atomicInt.h"


AtomicInt3::AtomicInt3()
	: mValue{ Int3{ 0, 0, 0 } }
{
}

AtomicInt3::AtomicInt3(int x_, int y_, int z_)
	: mValue{ Int3{ x_, y_, z_ } }
{
}

AtomicInt3& AtomicInt3::operator=(const AtomicInt3& other)
{
	SetValue(other.GetXMFloat3());
	return *this;
}

AtomicInt3& AtomicInt3::operator=(const XMFLOAT3& other)
{
	SetValue(other);
	return *this;
}

void AtomicInt3::SetZero()
{
	mValue = Int3{ 0, 0, 0 };
}

bool AtomicInt3::IsZero()
{
	Int3 val = mValue;
	return (val.x == 0 && val.y == 0 && val.z == 0);
}

void AtomicInt3::SetValue(int x_, int y_, int z_)
{
	mValue = Int3{ x_, y_, z_ };
}

void AtomicInt3::SetValue(const vec3& vec)
{
	SetValue(vec.x, (int)vec.y, vec.z);
}

void AtomicInt3::SetValue(const btVector3& vec)
{
	SetValue(
		(int)(vec.x() * POS_FLOAT_PRECISION),
		(int)(vec.y() * POS_FLOAT_PRECISION),
		(int)(vec.z() * POS_FLOAT_PRECISION));
}

void AtomicInt3::SetValue(const XMFLOAT3& xmf3)
{
	SetValue(
		(int)(xmf3.x * POS_FLOAT_PRECISION),
		(int)(xmf3.y * POS_FLOAT_PRECISION),
		(int)(xmf3.z * POS_FLOAT_PRECISION));
}

void AtomicInt3::Extrapolate(int dx, int dy, int dz, float dt)
{
	XMFLOAT3& val = GetXMFloat3();
	val.x += dx / POS_FLOAT_PRECISION * dt;
	val.y += dy / POS_FLOAT_PRECISION * dt;
	val.z += dz / POS_FLOAT_PRECISION * dt;
	SetValue(val);
}

btVector3 AtomicInt3::GetBtVector3() const
{
	Int3 val = mValue;

	return btVector3{
		val.x / POS_FLOAT_PRECISION,
		val.y / POS_FLOAT_PRECISION,
		val.z / POS_FLOAT_PRECISION };
}

XMFLOAT3 AtomicInt3::GetXMFloat3() const
{
	Int3 val = mValue;

	return XMFLOAT3{
		val.x / POS_FLOAT_PRECISION,
		val.y / POS_FLOAT_PRECISION,
		val.z / POS_FLOAT_PRECISION };
}


AtomicInt4::AtomicInt4()
	: mValue{ Int4{ 0, 0, 0, (int)QUAT_FLOAT_PRECISION } }
{
}

AtomicInt4::AtomicInt4(int x_, int y_, int z_, int w_)
	: mValue{ Int4{ x_, y_, z_, w_ } }
{
}

AtomicInt4& AtomicInt4::operator=(const AtomicInt4& other)
{
	SetValue(other.GetXMFloat4());
	return *this;
}

AtomicInt4& AtomicInt4::operator=(const XMFLOAT4& other)
{
	SetValue(other);
	return *this;
}

void AtomicInt4::SetValue(int x_, int y_, int z_, int w_)
{
	mValue = Int4{ x_, y_, z_, w_ };
}

void AtomicInt4::SetValue(const quat3& quat)
{
	auto elems = Compressor::DecodeQuat(quat);
	SetValue(
		(int)(elems[0] * QUAT_FLOAT_PRECISION),
		(int)(elems[1] * QUAT_FLOAT_PRECISION),
		(int)(elems[2] * QUAT_FLOAT_PRECISION),
		(int)(elems[3] * QUAT_FLOAT_PRECISION));
}

void AtomicInt4::SetValue(const btQuaternion& quat)
{
	SetValue(
		(int)(quat.x() * QUAT_FLOAT_PRECISION),
		(int)(quat.y() * QUAT_FLOAT_PRECISION),
		(int)(quat.z() * QUAT_FLOAT_PRECISION),
		(int)(quat.w() * QUAT_FLOAT_PRECISION));
}

void AtomicInt4::SetValue(const XMFLOAT4& quat)
{
	SetValue(
		(int)(quat.x * QUAT_FLOAT_PRECISION),
		(int)(quat.y * QUAT_FLOAT_PRECISION),
		(int)(quat.z * QUAT_FLOAT_PRECISION),
		(int)(quat.w * QUAT_FLOAT_PRECISION));
}

void AtomicInt4::Extrapolate(int dx, int dy, int dz, float dt)
{
	XMFLOAT3 vec = {
		dx / QUAT_FLOAT_PRECISION * dt,
		dy / QUAT_FLOAT_PRECISION * dt,
		dz / QUAT_FLOAT_PRECISION * dt };

	XMVECTOR a = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&vec));

	XMFLOAT4 origin = GetXMFloat4();
	XMVECTOR nextQuat = XMVector4Normalize(XMQuaternionMultiply(a, XMLoadFloat4(&origin)));

	XMStoreFloat4(&origin, nextQuat);
	SetValue(origin);
}

bool AtomicInt4::IsZero() const
{
	Int4 val = mValue;
	return (val.x == 0.0f && val.y == 0.0f && val.z == 0.0f && val.w == 0.0f);
}

btQuaternion AtomicInt4::GetBtQuaternion() const
{
	Int4 val = mValue;

	return btQuaternion{
		val.x / QUAT_FLOAT_PRECISION,
		val.y / QUAT_FLOAT_PRECISION,
		val.z / QUAT_FLOAT_PRECISION,
		val.w / QUAT_FLOAT_PRECISION };
}

XMFLOAT4 AtomicInt4::GetXMFloat4() const
{
	Int4 val = mValue;

	return XMFLOAT4{
		val.x / QUAT_FLOAT_PRECISION,
		val.y / QUAT_FLOAT_PRECISION,
		val.z / QUAT_FLOAT_PRECISION,
		val.w / QUAT_FLOAT_PRECISION };
}
