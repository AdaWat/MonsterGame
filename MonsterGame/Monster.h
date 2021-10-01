#pragma once
#include "Character.h"
class Monster : public Character
{
public:
	Monster(int x, int y)
	{
		position[0] = x;
		position[1] = y;
		logo = 'M';
	}
};

