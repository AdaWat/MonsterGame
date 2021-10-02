// MonsterGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <thread>

#include "Player.h"
#include "Monster.h"
#include "Item.h"

using namespace std;

void add_item(Character, char(*)[10][10]);
void move_item(Character(*), int, int, char(*)[10][10]);

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


	// initialise grid with blank values
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

	// add traps
	Item traps[5];
	for (int i = 0; i < 5; i++) {
		int r, c;
		do {
			r = rand() % 10;
			c = rand() % 10;
		} while (grid[r][c] != '#');
		// TODO: make this part cleaner (ideally remove defaults from Item constructor)
		Item t(c, r, 'T');
		traps[i] = t;
		grid[r][c] = traps[i].logo;
	}

	bool gameOver = false;
	bool keyHeldDown = false;
	bool monsterAwake = false;

	while (!gameOver) {
		// slow down loop
		this_thread::sleep_for(50ms);
		
		// ---Keyboard input---
		// W key
		if (GetAsyncKeyState(0x57) < 0 && !keyHeldDown) {
			move_item(&player, -1, 0, &grid);
			keyHeldDown = true;
		}
		// A key
		else if (GetAsyncKeyState(0x41) < 0 && !keyHeldDown) {
			move_item(&player, 0, -1, &grid);
			keyHeldDown = true;
		}
		// S key
		else if (GetAsyncKeyState(0x53) < 0 && !keyHeldDown) {
			move_item(&player, 1, 0, &grid);
			keyHeldDown = true;
		}
		// D key
		else if (GetAsyncKeyState(0x44) < 0 && !keyHeldDown) {
			move_item(&player, 0, 1, &grid);
			keyHeldDown = true;
		}
		if (!(GetAsyncKeyState(0x57) < 0 || GetAsyncKeyState(0x41) < 0 || GetAsyncKeyState(0x53) < 0 || GetAsyncKeyState(0x44) < 0) && keyHeldDown)
			keyHeldDown = false;

		// detect collision with monster
		if (player.position[0] == monster.position[0] && player.position[1] == monster.position[1]) {
			gameOver = true;
		}
		// detect collision with traps 
		for (Item trap : traps) {
			if (player.position[0] == trap.position[0] && player.position[1] == trap.position[1]) {
				monsterAwake = true;
				// TODO: delete all traps from memory and delete the traps array from memory. Then stop running this loop forever
				// TODO: make monster awake
			}
		}


		// populate the screen char array with the grid
		for (int r = 0; r < 10; r++) {
			for (int c = 0; c < 10; c++) {
				screen[(c + 2) * bufferWidth + (r * 2 + 4)] = grid[c][r];
			}
		}
		// Display Frame
		screen[bufferWidth * bufferHeight - 1] = '\0';	// end char array so Windows knows when to stop rendering
		WriteConsoleOutputCharacter(console, screen, bufferWidth * bufferHeight, { 0, 0 }, &bytesWritten);
	}

	CloseHandle(console);
	std::cout << "\nGAME OVER\n";
	return 0;
}

// Display the logo of an item/character
void add_item(Character c, char(*g)[10][10]) {
	(*g)[c.position[0]][c.position[1]] = c.logo;
	return;
}

void move_item(Character* c, int x, int y, char(*g)[10][10]) {
	// if valid move
	if (0 <= ((*c).position[0] + x) && ((*c).position[0] + x) <= 9 && 0 <= ((*c).position[1] + y) && ((*c).position[1] + y) <= 9) {
		// remove old item's position
		(*g)[(*c).position[0]][(*c).position[1]] = '#';
		// update item position
		(*c).move(x, y);
		// add item back to grid
		(*g)[(*c).position[0]][(*c).position[1]] = (*c).logo;
	}
	return;
}
