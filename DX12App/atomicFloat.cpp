#include "stdafx.h"
#include "atomicFloat.h"


AtomicFloat3::AtomicFloat3()
	: mValue{ Float3{ 0.0f, 0.0f, 0.0f } }
{
}

AtomicFloat3::AtomicFloat3(float x_, float y_, float z_)
	: mValue{ Float3{ x_, y_, z_ } }
{
}

AtomicFloat3& AtomicFloat3::operator=(const AtomicFloat3& other)
{
	SetValue(other.GetXMFloat3());
	return *this;
}

AtomicFloat3& AtomicFloat3::operator=(const XMFLOAT3& other)
{
	SetValue(other);
	return *this;
}

void AtomicFloat3::SetZero()
{
	mValue = Float3{ 0.0f, 0.0f, 0.0f };
}

bool AtomicFloat3::IsZero()
{
	Float3 val = mValue;
	return (val.x == 0.0f && val.y == 0.0f && val.z == 0.0f);
}

void AtomicFloat3::SetValue(float x_, float z_)
{
	Float3 val = mValue;
	mValue = Float3{ x_, val.y, z_ };
}

void AtomicFloat3::SetValue(float x_, float y_, float z_)
{
	mValue = Float3{ x_, y_, z_ };
}

void AtomicFloat3::SetValue(const vec3& vec)
{
	auto compPos = Compressor::DecodePos(vec);
	SetValue(compPos[0], compPos[1], compPos[2]);
}

void AtomicFloat3::SetValue(const btVector3& vec)
{
	SetValue(vec.x(), vec.y(), vec.z());
}

void AtomicFloat3::SetValue(const XMFLOAT3& xmf3)
{
	SetValue(xmf3.x, xmf3.y, xmf3.z);
}

void AtomicFloat3::Extrapolate(float dx, float dz, float dt)
{
	XMFLOAT3& val = GetXMFloat3();
	val.x += dx * dt;
	val.z += dz * dt;
	SetValue(val);
}

void AtomicFloat3::Extrapolate(float dx, float dy, float dz, float dt)
{
	XMFLOAT3& val = GetXMFloat3();
	val.x += dx * dt;
	val.y += dy * dt;
	val.z += dz * dt;
	SetValue(val);
}

btVector3 AtomicFloat3::GetBtVector3() const
{
	Float3 val = mValue;
	return btVector3{ val.x, val.y, val.z };
}

XMFLOAT3 AtomicFloat3::GetXMFloat3() const
{
	Float3 val = mValue;
	return XMFLOAT3{ val.x, val.y, val.z };
}


AtomicFloat4::AtomicFloat4()
	: mValue{ Float4{ 0.0f, 0.0f, 0.0f, 1.0f } }
{
}

AtomicFloat4::AtomicFloat4(float x_, float y_, float z_, float w_)
	: mValue{ Float4{ x_, y_, z_, w_ } }
{
}

AtomicFloat4& AtomicFloat4::operator=(const AtomicFloat4& other)
{
	SetValue(other.GetXMFloat4());
	return *this;
}

AtomicFloat4& AtomicFloat4::operator=(const XMFLOAT4& other)
{
	SetValue(other);
	return *this;
}

void AtomicFloat4::SetValue(float x_, float y_, float z_, float w_)
{
	mValue = Float4{ x_, y_, z_, w_ };
}

void AtomicFloat4::SetValue(const quat3& quat)
{
	auto elems = Compressor::DecodeQuat(quat);
	SetValue(elems[0], elems[1], elems[2], elems[3]);
}

void AtomicFloat4::SetValue(const btQuaternion& quat)
{
	SetValue(quat.x(), quat.y(), quat.z(), quat.w());
}

void AtomicFloat4::SetValue(const XMFLOAT4& quat)
{
	SetValue(quat.x, quat.y, quat.z, quat.w);
}

void AtomicFloat4::Extrapolate(float dx, float dy, float dz, float dt)
{
	XMFLOAT3 vec = { dx  * dt, dy * dt,	dz * dt };

	XMVECTOR a = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&vec));

	XMFLOAT4 origin = GetXMFloat4();
	XMVECTOR nextQuat = XMVector4Normalize(XMQuaternionMultiply(a, XMLoadFloat4(&origin)));

	XMStoreFloat4(&origin, nextQuat);
	SetValue(origin);
}

bool AtomicFloat4::IsZero() const
{
	Float4 val = mValue;
	return (val.x == 0.0f && val.y == 0.0f && val.z == 0.0f && val.w == 0.0f);
}

btQuaternion AtomicFloat4::GetBtQuaternion() const
{
	Float4 val = mValue;
	return btQuaternion{ val.x, val.y, val.z, val.w };
}

XMFLOAT4 AtomicFloat4::GetXMFloat4() const
{
	Float4 val = mValue;
	return XMFLOAT4{ val.x, val.y, val.z, val.w };
}
