#pragma once

#include "MeshData.h"

class Player
{
public:
	Player();
	~Player() = default;

	void SetPosition(float x, float y, float z);
	const btVector3& GetPosition() const { return mPosition; }

public:
	bool Empty;
	char Color;
	bool Ready;
	int ID;
	char Name[MAX_NAME_SIZE];

private:
	btVector3 mPosition;
};