#pragma once

struct OBJ_MASK
{
	static const int NONE		= 1;
	static const int VEHICLE	= 2;
	static const int MISSILE	= 4;
	static const int TRACK		= 8;
	static const int CHECKPOINT = 16;
};

struct OBJ_MASK_GROUP
{
	static const int VEHICLE = OBJ_MASK::VEHICLE | 
							   OBJ_MASK::TRACK   |
							   OBJ_MASK::MISSILE | 
							   OBJ_MASK::CHECKPOINT;

	static const int MISSILE	= OBJ_MASK::VEHICLE | OBJ_MASK::TRACK;
	static const int TRACK		= OBJ_MASK::VEHICLE | OBJ_MASK::MISSILE;
	static const int CHECKPOINT	= OBJ_MASK::VEHICLE;
	static const int INVINCIBLE = OBJ_MASK::TRACK;
};