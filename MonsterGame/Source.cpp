// MonsterGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "Player.h"
#include "Monster.h"

using namespace std;

void add_item(Character, char(*)[10][10]);

int bufferWidth = 80;
int bufferHeight = 60;

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[bufferWidth * bufferHeight];
	for (int i = 0; i < bufferWidth * bufferHeight; i++) screen[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;


	// initialise grid
	char grid[10][10];
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			grid[i][j] = '#';
		}
	}

	// add monster
	Monster monster(5, 5);
	add_item(monster, &grid);

	// add player
	Player player(0, 0);
	add_item(player, &grid);


	while (1) {




		// populate the screen char array with the grid
		for (int r = 0; r < 10; r++) {
			for (int c = 0; c < 10; c++) {
				screen[(r + 2) * bufferWidth + (c * 2 + 4)] = grid[r][c];
			}
		}
		// Display Frame
		screen[bufferWidth * bufferHeight - 1] = '\0';
		WriteConsoleOutputCharacter(console, screen, bufferWidth * bufferHeight, { 0, 0 }, &bytesWritten);
	}
	return 0;
}


void add_item(Character c, char(*g)[10][10]) {
	(*g)[c.position[0]][c.position[1]] = c.logo;
	return;
}



/*#include <iostream>
#include <Windows.h>
#include "Player.h"
#include "Monster.h"

using namespace std;

void show_grid(char[10][10]);
void add_item(Character, char(*)[10][10]);

int main()
{
	// initialise grid
	char grid[10][10];
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			grid[i][j] = 'x';
		}
	}

	// add monster
	Monster monster(5, 5);
	add_item(monster, &grid);

	// add player
	Player player(0, 0);
	add_item(player, &grid);

	show_grid(grid);;
}


// TODO: make array size variable (use pointers to initialise dymanic size arrays)
void show_grid(char arr[10][10]) {
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			cout << arr[i][j] << ' ';
		}
		cout << endl;
	}
	return;
}

void add_item(Character c, char(*g)[10][10]) {
	(*g)[c.position[0]][c.position[1]] = c.logo;
	return;
}*/