#pragma once

#include <atomic>

class AtomicFloat3
{
private:
	struct Float3
	{
		float x;
		float y;
		float z;
	};

public:
	AtomicFloat3();
	AtomicFloat3(float x_, float y_, float z_);
	AtomicFloat3& operator=(const AtomicFloat3& other);
	AtomicFloat3& operator=(const XMFLOAT3& other);

	void SetZero();
	bool IsZero();

	void SetValue(float x_, float z_);
	void SetValue(float x_, float y_, float z_);
	void SetValue(const vec3& vec);
	void SetValue(const btVector3& vec);
	void SetValue(const XMFLOAT3& xmf3);

	void Extrapolate(float dx, float dz, float dt);
	void Extrapolate(float dx, float dy, float dz, float dt);

	btVector3 GetBtVector3() const;
	XMFLOAT3 GetXMFloat3() const;

private:
	std::atomic<Float3> mValue;
};

class AtomicFloat4
{
private:
	struct Float4
	{
		float x;
		float y;
		float z;
		float w;
	};

public:
	AtomicFloat4();
	AtomicFloat4(float x_, float y_, float z_, float w_);
	AtomicFloat4& operator=(const AtomicFloat4& other);
	AtomicFloat4& operator=(const XMFLOAT4& other);

	void SetValue(float x_, float y_, float z_, float w_);
	void SetValue(const quat3& quat);
	void SetValue(const btQuaternion& quat);
	void SetValue(const XMFLOAT4& quat);

	void Extrapolate(float dx, float dy, float dz, float dt);

	bool IsZero() const;

	btQuaternion GetBtQuaternion() const;
	XMFLOAT4 GetXMFloat4() const;

private:
	std::atomic<Float4> mValue;
};
