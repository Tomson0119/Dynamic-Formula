#pragma once

class Player
{
public:
	Player() : mPosition{ btVector3(0.0f, 0.0f, 0.0f) } { }
	~Player() { }

private:
	btVector3 mPosition;
};