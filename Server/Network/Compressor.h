#pragma once

#include "Protocol.h"
#include <cmath>
#include <tuple>
#include <array>

const float POS_Y_SHIFT = 10.0f;
const float POS_FLOAT_PRECISION = 512.0f;
const float QUAT_FLOAT_PRECISION = 32767.0f;

struct packet
{
	uint8_t idx : 2;
};

class Compressor
{
public:
	static vec3 EncodePos(float x, float y, float z)
	{
		vec3 compPos{};
		compPos.x = (int)(x * POS_FLOAT_PRECISION);
		compPos.y = (unsigned short)((y + POS_Y_SHIFT) * POS_FLOAT_PRECISION);
		compPos.z = (int)(z * POS_FLOAT_PRECISION);
		return compPos;
	}

	static std::array<float, 3> DecodePos(const vec3& compPos)
	{
		float x = (float)compPos.x / POS_FLOAT_PRECISION;
		float y = (float)compPos.y / POS_FLOAT_PRECISION - POS_Y_SHIFT;
		float z = (float)compPos.z / POS_FLOAT_PRECISION;
		return { x, y, z };
	}

	static quat3 EncodeQuat(float x, float y, float z, float w)
	{
		float values[4] = { x,y,z,w };
		
		uint8_t idx = -1; // 2 bit
		float maxValue = -1.0f;
		float sign = 1.0f;

		for (int i = 0; i < 4; i++)
		{
			float av = abs(values[i]);
			if (av > maxValue)
			{
				maxValue = av;
				sign = (values[i] < 0) ? -1.0f : 1.0f;
				idx = i;
			}
		}

		quat3 compQuat{};
		compQuat.max_idx = idx;

		switch (compQuat.max_idx)
		{
		case 0: // x is largest.
			compQuat.elem1 = (short)(y * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem2 = (short)(z * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem3 = (short)(w * sign * QUAT_FLOAT_PRECISION);
			break;

		case 1: // y is largest.
			compQuat.elem1 = (short)(x * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem2 = (short)(z * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem3 = (short)(w * sign * QUAT_FLOAT_PRECISION);
			break;

		case 2: // z is largest.
			compQuat.elem1 = (short)(x * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem2 = (short)(y * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem3 = (short)(w * sign * QUAT_FLOAT_PRECISION);
			break;

		case 3: // w is largest.
			compQuat.elem1 = (short)(x * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem2 = (short)(y * sign * QUAT_FLOAT_PRECISION);
			compQuat.elem3 = (short)(z * sign * QUAT_FLOAT_PRECISION);
			break;
		}
		return compQuat;
	}

	static std::array<float, 4> DecodeQuat(const quat3& compQuat)
	{
		float a = (float)compQuat.elem1 / QUAT_FLOAT_PRECISION;
		float b = (float)compQuat.elem2 / QUAT_FLOAT_PRECISION;
		float c = (float)compQuat.elem3 / QUAT_FLOAT_PRECISION;
		float d = sqrt(1.0f - (a * a + b * b + c * c));

		switch (compQuat.max_idx)
		{
		case 0:
			return { d, a, b, c };

		case 1:
			return { a, d, b, c };

		case 2:
			return { a, b, d, c };

		case 3:
			return { a, b, c, d };
		}
		return { 0.0f, 0.0f, 0.0f, 1.0f };
	}
};