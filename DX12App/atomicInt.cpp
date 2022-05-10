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

void AtomicInt3::SetValue(const btVector3& vec)
{
	SetValue(
		(int)(vec.x() * FIXED_FLOAT_LIMIT),
		(int)(vec.y() * FIXED_FLOAT_LIMIT),
		(int)(vec.z() * FIXED_FLOAT_LIMIT));
}

void AtomicInt3::SetValue(const XMFLOAT3& xmf3)
{
	SetValue(
		(int)(xmf3.x * FIXED_FLOAT_LIMIT),
		(int)(xmf3.y * FIXED_FLOAT_LIMIT),
		(int)(xmf3.z * FIXED_FLOAT_LIMIT));
}

void AtomicInt3::Extrapolate(int dx, int dy, int dz, float dt)
{
	XMFLOAT3& val = GetXMFloat3();
	val.x += dx / FIXED_FLOAT_LIMIT * dt;
	val.y += dy / FIXED_FLOAT_LIMIT * dt;
	val.z += dz / FIXED_FLOAT_LIMIT * dt;
	SetValue(val);
}

btVector3 AtomicInt3::GetBtVector3() const
{
	Int3 val = mValue;

	return btVector3{
		val.x / FIXED_FLOAT_LIMIT,
		val.y / FIXED_FLOAT_LIMIT,
		val.z / FIXED_FLOAT_LIMIT };
}

XMFLOAT3 AtomicInt3::GetXMFloat3() const
{
	Int3 val = mValue;

	return XMFLOAT3{
		val.x / FIXED_FLOAT_LIMIT,
		val.y / FIXED_FLOAT_LIMIT,
		val.z / FIXED_FLOAT_LIMIT };
}


AtomicInt4::AtomicInt4()
	: mValue{ Int4{ 0, 0, 0, (int)FIXED_FLOAT_LIMIT } }
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

void AtomicInt4::SetValue(const btQuaternion& quat)
{
	SetValue(
		(int)(quat.x() * FIXED_FLOAT_LIMIT),
		(int)(quat.y() * FIXED_FLOAT_LIMIT),
		(int)(quat.z() * FIXED_FLOAT_LIMIT),
		(int)(quat.w() * FIXED_FLOAT_LIMIT));
}

void AtomicInt4::SetValue(const XMFLOAT4& quat)
{
	SetValue(
		(int)(quat.x * FIXED_FLOAT_LIMIT),
		(int)(quat.y * FIXED_FLOAT_LIMIT),
		(int)(quat.z * FIXED_FLOAT_LIMIT),
		(int)(quat.w * FIXED_FLOAT_LIMIT));
}

void AtomicInt4::Extrapolate(int dx, int dy, int dz, float dt)
{
	XMFLOAT3 vec = {
		dx / FIXED_FLOAT_LIMIT * dt,
		dy / FIXED_FLOAT_LIMIT * dt,
		dz / FIXED_FLOAT_LIMIT * dt };

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
		val.x / FIXED_FLOAT_LIMIT,
		val.y / FIXED_FLOAT_LIMIT,
		val.z / FIXED_FLOAT_LIMIT,
		val.w / FIXED_FLOAT_LIMIT };
}

XMFLOAT4 AtomicInt4::GetXMFloat4() const
{
	Int4 val = mValue;

	return XMFLOAT4{
		val.x / FIXED_FLOAT_LIMIT,
		val.y / FIXED_FLOAT_LIMIT,
		val.z / FIXED_FLOAT_LIMIT,
		val.w / FIXED_FLOAT_LIMIT };
}
