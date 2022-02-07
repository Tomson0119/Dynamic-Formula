#include "common.h"
#include "Player.h"

Player::Player()
	: mPosition{ 0.0f, 0.0f, 0.0f },
	  Empty{ true }, Color{ -1 }, Ready{ false }, 
	  ID{ -1 }, Name{ }
{
}

void Player::SetPosition(float x, float y, float z)
{
	mPosition = { x, y, z };
}
