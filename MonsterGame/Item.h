#pragma once
class Item
{
public:
	int position[2] = { 0, 0 };
	wchar_t logo;

	// TODO: ideally remove defaults from Item constructor
	Item(int x=0, int y=0, wchar_t type=L'T') {
		logo = type; // trap or gold
		position[0] += x;
		position[1] += y;
	}
};
