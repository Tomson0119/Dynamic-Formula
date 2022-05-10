#pragma once

#include "Protocol.h"
#include <cmath>
#include <tuple>
#include <array>

struct packet
{
	uint8_t idx : 2;
};

class Compressor
{
public:
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
			compQuat.elem1 = (short)(y * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem2 = (short)(z * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem3 = (short)(w * sign * FIXED_FLOAT_LIMIT);
			break;

		case 1: // y is largest.
			compQuat.elem1 = (short)(x * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem2 = (short)(z * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem3 = (short)(w * sign * FIXED_FLOAT_LIMIT);
			break;

		case 2: // z is largest.
			compQuat.elem1 = (short)(x * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem2 = (short)(y * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem3 = (short)(w * sign * FIXED_FLOAT_LIMIT);
			break;

		case 3: // w is largest.
			compQuat.elem1 = (short)(x * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem2 = (short)(y * sign * FIXED_FLOAT_LIMIT);
			compQuat.elem3 = (short)(z * sign * FIXED_FLOAT_LIMIT);
			break;
		}
		return compQuat;
	}

	static std::array<float, 4> DecodeQuat(const quat3& compQuat)
	{
		float a = (float)compQuat.elem1 / FIXED_FLOAT_LIMIT;
		float b = (float)compQuat.elem2 / FIXED_FLOAT_LIMIT;
		float c = (float)compQuat.elem3 / FIXED_FLOAT_LIMIT;
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