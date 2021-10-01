#pragma once
#include "Character.h"
class Player : public Character
{
public:
	Player(int x, int y)
	{
		position[0] = x;
		position[1] = y;
		logo = 'P';
	}
};