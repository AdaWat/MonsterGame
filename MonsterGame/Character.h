#pragma once
class Character
{
public:
	int position[2];
	wchar_t logo;

	void move(int x, int y)
	{
		position[0] += x;
		position[1] += y;
		return;
	}
};
