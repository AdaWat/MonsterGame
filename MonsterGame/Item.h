#pragma once
class Item
{
public:
	int position[2];
	char logo;

	Item(int x=0, int y=0, char type='T') {
		logo = type; // trap or gold
		position[0] += x;
		position[1] += y;
	}
};
