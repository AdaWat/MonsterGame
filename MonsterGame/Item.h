#pragma once
class Item
{
public:
	int position[2] = { 0, 0 };
	char logo;

	// TODO: ideally remove defaults from Item constructor
	Item(int x=0, int y=0, char type='T') {
		logo = type; // trap or gold
		position[0] += x;
		position[1] += y;
	}
};
