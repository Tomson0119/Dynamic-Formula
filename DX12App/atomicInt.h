#pragma once

#include <atomic>

class AtomicInt3
{
private:
	struct Int3
	{
		int x;
		int y;
		int z;
	};

public:
	AtomicInt3();
	AtomicInt3(int x_, int y_, int z_);
	AtomicInt3& operator=(const AtomicInt3& other);
	AtomicInt3& operator=(const XMFLOAT3& other);

	void SetZero();
	bool IsZero();

	void SetValue(int x_, int y_, int z_);
	// Set values from btVector3
	void SetValue(const btVector3& vec);
	// Set values from XMFLOAT3
	void SetValue(const XMFLOAT3& xmf3);

	void Extrapolate(int dx, int dy, int dz, float dt);

	btVector3 GetBtVector3() const;
	XMFLOAT3 GetXMFloat3() const;

private:
	std::atomic<Int3> mValue;
};

class AtomicInt4
{
private:
	struct Int4
	{
		int x;
		int y;
		int z;
		int w;
	};

public:
	AtomicInt4();
	AtomicInt4(int x_, int y_, int z_, int w_);
	AtomicInt4& operator=(const AtomicInt4& other);
	AtomicInt4& operator=(const XMFLOAT4& other);

	void SetValue(int x_, int y_, int z_, int w_);
	void SetValue(const btQuaternion& quat);
	void SetValue(const XMFLOAT4& quat);

	void Extrapolate(int dx, int dy, int dz, float dt);

	bool IsZero() const;

	btQuaternion GetBtQuaternion() const;
	XMFLOAT4 GetXMFloat4() const;

private:
	std::atomic<Int4> mValue;
};
