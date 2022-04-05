#pragma once

#define BT_NO_SIMD_OPERATOR_OVERLOADS
#define NOMINMAX

// Network 헤더 파일:
#include "NetLib/NetCommon.h"


#ifndef _DEBUG
#define _DEBUG
#else
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <dxgidebug.h>
#endif


// Window 헤더 파일:
#include <Windows.h>
#include <windowsx.h>
#include <sdkddkver.h>
#include <wrl.h>
#include <comdef.h>

// D3D12 헤더 파일:
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <DirectXCollision.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>

#include "d3dx12.h"
#include "DDSTextureLoader12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")


//D2D11 헤더파일:
#include <d2d1_3.h>
#include <d3d11on12.h>
#include <dwrite.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxguid.lib")

// C++ 헤더 파일:
#include <array>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <unordered_map>
#include <string>
#include <sstream>
#include <memory>
#include <fstream>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <tchar.h>
#include <conio.h>
#include <io.h>

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "BulletCollision/GImpact/btGImpactShape.h"

#include "d3dExtension.h"
#include "dxException.h"

#include "bulletWrap.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

using namespace std::chrono_literals;

extern UINT gRtvDescriptorSize;
extern UINT gDsvDescriptorSize;
extern UINT gCbvSrvUavDescriptorSize;

extern int gFrameWidth;
extern int gFrameHeight;

extern ComPtr<ID3D12Resource> CreateBufferResource(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	ComPtr<ID3D12Resource>& uploadBuffer,
	D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT);

extern ComPtr<ID3D12Resource> CreateTexture2DResource(
	ID3D12Device* device,
	UINT width, UINT height, UINT elements, UINT miplevels,
	DXGI_FORMAT format, D3D12_RESOURCE_FLAGS resourceFlags,
	D3D12_RESOURCE_STATES resourceStates, D3D12_CLEAR_VALUE* clearValue, UINT sampleCount = 1, UINT sampleQuality = 0);


inline UINT GetConstantBufferSize(UINT bytes)
{
	return ((bytes + 255) & ~255);
}

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(f)												\
{																		\
	HRESULT hr_ = (f);													\
	std::wstring wfn = AnsiToWString(__FILE__);							\
	if (FAILED(hr_))	{ throw DxException(hr_, L#f, wfn, __LINE__); } \
}																		
#endif


////////////////////////////////////////////////////////////////////////////
//
#define NUM_LIGHTS 3

#define POINT_LIGHT		  1
#define SPOT_LIGHT		  2
#define DIRECTIONAL_LIGHT 3

struct LightInfo
{
	XMFLOAT3 Diffuse = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float    FalloffStart = 0.0f;
	XMFLOAT3 Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float    FalloffEnd = 0.0f;
	XMFLOAT3 Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float    SpotPower = 0.0f;
	float    Range;
	int		 Type;
	
	void SetInfo(
		const XMFLOAT3& diffuse,
		const XMFLOAT3& position,
		const XMFLOAT3& direction,
		const float& falloffStart,
		const float& falloffEnd,
		const float& spotPower,
		float range, int type)
	{
		Diffuse = diffuse;
		Position = position;
		Direction = direction;
		FalloffStart = falloffStart;
		FalloffEnd = falloffEnd;
		SpotPower = spotPower;
		Range = range;
		Type = type;
	}
};

struct LightConstants
{
	XMFLOAT4X4 ShadowTransform[3];
	XMFLOAT4 Ambient;
	LightInfo Lights[NUM_LIGHTS];
};

struct CameraConstants
{
	XMFLOAT4X4 View;
	XMFLOAT4X4 Proj;
	XMFLOAT4X4 ViewProj;
	XMFLOAT3 CameraPos;
	float Aspect;
	XMFLOAT4X4 oldView;
};

struct Material
{
	XMFLOAT3 Ambient;
	float	 padding0;
	XMFLOAT4 Diffuse;
	XMFLOAT3 Specular;
	float	 Exponent;
	XMFLOAT3 Emission;
	float	 IOR;

	Material()
		: Ambient(1.0f,1.0f,1.0f),
		  Diffuse(1.0f, 1.0f, 1.0f, 1.0f),
		  Specular(1.0f,1.0f,1.0f),
		  Emission(0.0f,0.0f,0.0f),
		  Exponent(300.0f), IOR(1.45f), padding0(0)
	{
	}
};

struct ObjectConstants
{
	XMFLOAT4X4 World;
	XMFLOAT4X4 oldWorld;
	int32_t cubemapOn;
	int32_t motionBlurOn;
	int32_t rimLightOn;
};

struct MaterialConstants
{
	XMFLOAT4X4 TexTransform;
	Material Mat;
};

struct GameInfoConstants
{
	XMFLOAT4 RandFloat4;
	XMFLOAT3 PlayerPosition;
	float CurrentTime;
	float ElapsedTime;
};

struct InstancingInfo
{
	XMFLOAT4X4 World;
	XMFLOAT4X4 oldWorld;
};


////////////////////////////////////////////////////////////////////////////
//
struct AtomicInt3
{
	AtomicInt3()
		: x{ 0 }, y{ 0 }, z{ 0 }
	{
	}

	AtomicInt3(int x_, int y_, int z_)
		: x{ x_ }, y{ y_ }, z{ z_ }
	{
	}

	AtomicInt3& operator=(const AtomicInt3& other)
	{
		x.store(other.x);
		y.store(other.y);
		z.store(other.z);
		return *this;
	}

	void SetZero()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	bool IsZero()
	{
		return (x == 0 && y == 0 && z == 0);
	}

	void SetValue(int x_, int y_, int z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}

	// Set values from btVector3
	void SetValue(const btVector3& vec)
	{
		SetValue(
			(int)(vec.x() * FIXED_FLOAT_LIMIT),
			(int)(vec.y() * FIXED_FLOAT_LIMIT),
			(int)(vec.z() * FIXED_FLOAT_LIMIT));
	}

	// Set values from XMFLOAT3
	void SetValue(const XMFLOAT3& xmf3)
	{
		SetValue(
			(int)(xmf3.x * FIXED_FLOAT_LIMIT),
			(int)(xmf3.y * FIXED_FLOAT_LIMIT),
			(int)(xmf3.z * FIXED_FLOAT_LIMIT));
	}

	btVector3 GetBtVector3() const
	{
		return btVector3{ 
			x / FIXED_FLOAT_LIMIT, 
			y / FIXED_FLOAT_LIMIT, 
			z / FIXED_FLOAT_LIMIT };
	}

	XMFLOAT3 GetXMFloat3() const
	{
		return XMFLOAT3{ 
			x / FIXED_FLOAT_LIMIT, 
			y / FIXED_FLOAT_LIMIT, 
			z / FIXED_FLOAT_LIMIT };
	}

	std::atomic_int x;
	std::atomic_int y;
	std::atomic_int z;
};

struct AtomicInt4
{
	AtomicInt4()
		: x{ 0 }, y{ 0 }, z{ 0 }, w{ (int)FIXED_FLOAT_LIMIT }
	{
	}

	AtomicInt4(int x_, int y_, int z_, int w_)
		: x{ x_ }, y{ y_ }, z{ z_ }, w{ w_ }
	{
	}

	AtomicInt4& operator=(const AtomicInt4& other)
	{
		x.store(other.x);
		y.store(other.y);
		z.store(other.z);
		w.store(other.w);
		return *this;
	}

	void SetValue(int x_, int y_, int z_, int w_)
	{
		x = x_;
		y = y_;
		z = z_;
		w = w_;
	}

	void SetValue(const btQuaternion& quat)
	{
		SetValue(
			(int)(quat.x() * FIXED_FLOAT_LIMIT),
			(int)(quat.y() * FIXED_FLOAT_LIMIT),
			(int)(quat.z() * FIXED_FLOAT_LIMIT),
			(int)(quat.w() * FIXED_FLOAT_LIMIT));
	}

	void SetValue(const XMFLOAT4& quat)
	{
		SetValue(
			(int)(quat.x * FIXED_FLOAT_LIMIT),
			(int)(quat.y * FIXED_FLOAT_LIMIT),
			(int)(quat.z * FIXED_FLOAT_LIMIT),
			(int)(quat.w * FIXED_FLOAT_LIMIT));
	}

	bool IsZero() const
	{
		return (x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f);
	}

	btQuaternion GetBtQuaternion() const
	{
		return btQuaternion{
			x / FIXED_FLOAT_LIMIT,
			y / FIXED_FLOAT_LIMIT,
			z / FIXED_FLOAT_LIMIT,
			w / FIXED_FLOAT_LIMIT };
	}

	XMFLOAT4 GetXMFloat4() const
	{
		return XMFLOAT4{
			x / FIXED_FLOAT_LIMIT,
			y / FIXED_FLOAT_LIMIT,
			z / FIXED_FLOAT_LIMIT,
			w / FIXED_FLOAT_LIMIT };
	}

	std::atomic_int x;
	std::atomic_int y;
	std::atomic_int z;
	std::atomic_int w;
};


////////////////////////////////////////////////////////////////////////////
//
namespace BulletMath
{
	inline bool Equals(const btVector3& a, const btVector3& b, btScalar epsilon)
	{
		btScalar xSub = fabs(a.x() - b.x());
		btScalar ySub = fabs(a.y() - b.y());
		btScalar zSub = fabs(a.z() - b.z());

		return (xSub < epsilon&& ySub < epsilon&& zSub < epsilon);
	}

	inline bool Equals(const btQuaternion& a, const btQuaternion& b, float epsilon)
	{
		btScalar len = (a - b).length2();
		return (len < (epsilon* epsilon));
	}

	inline bool IsZero(const btQuaternion& quat)
	{
		return (quat.x() == 0.0f && quat.y() == 0.0f && quat.z() == 0.0f && quat.w() == 0.0f);
	}
}

namespace Math
{
	const float PI = 3.1415926535f;

	inline int RandInt(int min, int max)
	{
		return rand() % (max - min + 1) + min;
	}

	inline float RandFloat(float min, float max)
	{
		float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		return r * (max - min) + min;
	}

	inline float ClampFloat(float x, float min, float max)
	{
		return (x < min) ? min : ((x > max) ? max : x);
	}
}

namespace Vector3
{
	inline XMFLOAT3 btVectorToXM(const btVector3& v)
	{
		return XMFLOAT3(v.x(), v.y(), v.z());
	}

	inline XMFLOAT3 Zero()
	{
		return XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	inline XMFLOAT3 VectorToFloat3(FXMVECTOR& vector)
	{
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, vector);
		return ret;
	}

	inline XMFLOAT3 Replicate(float value)
	{
		return VectorToFloat3(XMVectorReplicate(value));
	}

	inline XMFLOAT3 Multiply(float scalar, const XMFLOAT3& v)
	{
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, scalar * XMLoadFloat3(&v));
		return ret;
	}

	inline XMFLOAT3 Multiply(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return XMFLOAT3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
	}

	inline XMFLOAT3 Divide(float scalar, XMFLOAT3& v)
	{
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMLoadFloat3(&v) / scalar);
		return ret;
	}

	inline XMFLOAT3 MultiplyAdd(float delta, XMFLOAT3& src, XMFLOAT3& dst)
	{		
		XMVECTOR v1 = XMLoadFloat3(&Replicate(delta));
		XMVECTOR v2 = XMLoadFloat3(&src);
		XMVECTOR v3 = XMLoadFloat3(&dst);
		return VectorToFloat3(XMVectorMultiplyAdd(v1, v2, v3));
	}

	inline XMFLOAT3 TransformNormal(XMFLOAT3& src, FXMMATRIX& mat)
	{
		return VectorToFloat3(XMVector3TransformNormal(XMLoadFloat3(&src), mat));
	}

	inline XMFLOAT3 Transform(XMFLOAT3& src, FXMMATRIX& mat)
	{
		return VectorToFloat3(XMVector3Transform(XMLoadFloat3(&src), mat));
	}

	inline XMFLOAT3 TransformCoord(XMFLOAT3& src, XMFLOAT4X4& mat)
	{
		return VectorToFloat3(XMVector3TransformCoord(XMLoadFloat3(&src), XMLoadFloat4x4(&mat)));
	}

	inline XMFLOAT3 Normalize(XMFLOAT3& v)
	{
		return VectorToFloat3(XMVector3Normalize(XMLoadFloat3(&v)));
	}

	inline XMFLOAT3 Subtract(XMFLOAT3& v1, XMFLOAT3& v2)
	{
		return VectorToFloat3(XMVectorSubtract(XMLoadFloat3(&v1), XMLoadFloat3(&v2)));
	}

	inline XMFLOAT3 ScalarProduct(XMFLOAT3& v, float scalar)
	{
		return VectorToFloat3(XMLoadFloat3(&v) * scalar);
	}

	inline XMFLOAT3 Cross(XMFLOAT3& v1, XMFLOAT3& v2)
	{
		return VectorToFloat3(XMVector3Cross(XMLoadFloat3(&v1), XMLoadFloat3(&v2)));
	}

	inline float Length(XMFLOAT3& v)
	{
		XMFLOAT3 ret = VectorToFloat3(XMVector3Length(XMLoadFloat3(&v)));
		return ret.x;
	}

	inline float Dot(XMFLOAT3& v1, XMFLOAT3& v2)
	{
		return XMVectorGetX(XMVector3Dot(XMLoadFloat3(&v1), XMLoadFloat3(&v2)));
	}

	inline XMFLOAT3 Add(XMFLOAT3& v, float value)
	{
		return VectorToFloat3(XMVectorAdd(XMLoadFloat3(&v), XMVectorReplicate(value)));
	}

	inline XMFLOAT3 Add(XMFLOAT3& v1, XMFLOAT3& v2)
	{
		return VectorToFloat3(XMLoadFloat3(&v1) + XMLoadFloat3(&v2));
	}

	inline XMFLOAT3 Add(XMFLOAT3& v1, XMFLOAT3& v2, float distance)
	{
		return VectorToFloat3(XMLoadFloat3(&v1) + XMLoadFloat3(&v2) * distance);
	}

	inline XMFLOAT3 ClampFloat3(XMFLOAT3& input, XMFLOAT3& min, XMFLOAT3& max)
	{
		XMFLOAT3 ret;
		ret.x = (min.x > input.x) ? min.x : ((max.x < input.x) ? max.x : input.x);
		ret.y = (min.y > input.y) ? min.y : ((max.y < input.y) ? max.y : input.y);
		ret.z = (min.z > input.z) ? min.z : ((max.z < input.z) ? max.z : input.z);
		return ret;
	}

	inline XMFLOAT3 Absf(XMFLOAT3& v)
	{
		return XMFLOAT3(std::abs(v.x), std::abs(v.y), std::abs(v.z));
	}

	inline bool Equal(XMFLOAT3& v1, XMFLOAT3& v2)
	{
		return XMVector3Equal(XMLoadFloat3(&v1), XMLoadFloat3(&v2));
	}

	inline bool Less(XMFLOAT3& v, float x)
	{
		return XMVector3Less(XMLoadFloat3(&v), XMVectorReplicate(x));
	}

	inline XMFLOAT3 Lerp(const XMFLOAT3& from, const XMFLOAT3& to, float t)
	{
		return VectorToFloat3(XMVectorLerp(XMLoadFloat3(&from), XMLoadFloat3(&to), t));
	}
}

namespace Vector4
{
	inline XMFLOAT4 Zero()
	{
		return XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	inline bool Equal(XMFLOAT4& v1, XMFLOAT4& v2)
	{
		return XMVector4Equal(XMLoadFloat4(&v1), XMLoadFloat4(&v2));
	}

	inline XMFLOAT4 Add(XMFLOAT4& v1, XMFLOAT4& v2)
	{
		XMFLOAT4 ret;
		XMStoreFloat4(&ret, XMVectorAdd(XMLoadFloat4(&v1), XMLoadFloat4(&v2)));
		return ret;
	}

	inline XMFLOAT4 Multiply(float scalar, XMFLOAT4& v)
	{
		XMFLOAT4 ret;
		XMStoreFloat4(&ret, scalar * XMLoadFloat4(&v));
		return ret;
	}

	inline XMFLOAT4 RotateQuaternionAxis(const XMFLOAT3& axis, float angle)
	{
		XMFLOAT4 ret;
		XMStoreFloat4(&ret, XMQuaternionRotationAxis(XMLoadFloat3(&axis), angle));
		return ret;
	}

	inline XMFLOAT4 RotateQuaternionRollPitchYaw(const XMFLOAT3& rotation)
	{
		XMFLOAT4 ret;
		XMStoreFloat4(&ret, XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z));
		return ret;
	}

	inline XMFLOAT4 Slerp(const XMFLOAT4& from, const XMFLOAT4& to, float t)
	{
		XMFLOAT4 ret;
		XMStoreFloat4(&ret, XMQuaternionSlerp(XMLoadFloat4(&from), XMLoadFloat4(&to), t));
		return ret;
	}
}

namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity4x4()
	{	
		XMFLOAT4X4 ret;
		XMStoreFloat4x4(&ret, XMMatrixIdentity());
		return ret;
	}

	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& mat)
	{
		XMFLOAT4X4 ret;
		XMStoreFloat4x4(&ret, XMMatrixTranspose(XMLoadFloat4x4(&mat)));
		return ret;
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& mat1, XMFLOAT4X4& mat2)
	{
		XMFLOAT4X4 ret;
		XMStoreFloat4x4(&ret, XMMatrixMultiply(XMLoadFloat4x4(&mat1), XMLoadFloat4x4(&mat2)));
		return ret;
	}

	inline XMFLOAT4X4 Multiply(FXMMATRIX& mat1, XMFLOAT4X4& mat2)
	{
		XMFLOAT4X4 ret;
		XMStoreFloat4x4(&ret, mat1 * XMLoadFloat4x4(&mat2));
		return ret;
	}

	inline XMFLOAT4X4 Reflect(XMFLOAT4& plane)
	{
		XMFLOAT4X4 ret;
		XMStoreFloat4x4(&ret, XMMatrixReflect(XMLoadFloat4(&plane)));
		return ret;
	}

	inline XMFLOAT4X4 CalulateWorldTransform(const XMFLOAT3& position, const XMFLOAT4& quaternion, const XMFLOAT3& scale)
	{
		XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion));
		XMMATRIX scaling = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
		
		XMFLOAT4X4 world{};
		XMStoreFloat4x4(&world, scaling * rotation * translation);
		return world;
	}

	inline XMFLOAT4X4 glMatrixToD3DMatrix(btScalar* btMat)
	{
		XMFLOAT4X4 xmf4x4Result;

		xmf4x4Result._11 = btMat[0];
		xmf4x4Result._12 = btMat[1];
		xmf4x4Result._13 = btMat[2];
		xmf4x4Result._14 = btMat[3];

		xmf4x4Result._21 = btMat[4];
		xmf4x4Result._22 = btMat[5];
		xmf4x4Result._23 = btMat[6];
		xmf4x4Result._24 = btMat[7];
		
		xmf4x4Result._31 = btMat[8];
		xmf4x4Result._32 = btMat[9];
		xmf4x4Result._33 = btMat[10];
		xmf4x4Result._34 = btMat[11];
		
		xmf4x4Result._41 = btMat[12];
		xmf4x4Result._42 = btMat[13];
		xmf4x4Result._43 = btMat[14];
		xmf4x4Result._44 = btMat[15];

		return xmf4x4Result;
	}
}