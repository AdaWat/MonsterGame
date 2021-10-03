// MonsterGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
// 

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>

#include "Player.h"
#include "Monster.h"
#include "Item.h"

using namespace std;
// TODO: FIX COORDINATE SYSTEM - MAKE CONSISTANT

const int boardWidth = 8;
const int numberOfTraps = 5;

const int bufferWidth = 80;
const int bufferHeight = 60;
const char blank = L'#';

void add_item(Character, char(*)[boardWidth][boardWidth]);
void move_item(Character(*), int, int, char(*)[boardWidth][boardWidth]);

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[bufferWidth * bufferHeight];
	for (int i = 0; i < bufferWidth * bufferHeight; i++) screen[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;


	// initialise grid with blank values
	char grid[boardWidth][boardWidth];
	for (int i = 0; i < boardWidth; i++) {
		for (int j = 0; j < boardWidth; j++) {
			grid[i][j] = blank;
		}
	}

	// add monster
	Monster monster(boardWidth - 1, boardWidth - 1);
	add_item(monster, &grid);

	// add player
	Player player(0, 0);
	add_item(player, &grid);

	// add traps
	std::vector<unique_ptr<Item>> traps;
	for (int i = 0; i < numberOfTraps; i++) {
		int r, c;
		do {
			r = rand() % boardWidth;
			c = rand() % boardWidth;
		} while (grid[r][c] != blank);
		// TODO: make this part cleaner (ideally remove defaults from Item constructor)
		//Item t(c, r, 'T');
		//traps[i] = t;
		traps.push_back(std::make_unique<Item>(c, r, 'T'));
		grid[r][c] = traps[i]->logo;
	}

	bool gameOver = false;
	bool keyHeldDown = false;
	bool monsterAwake = false;

	while (!gameOver) {
		// slow down loop
		this_thread::sleep_for(50ms);

		// ---Keyboard input---
		if (!keyHeldDown) {
			// W key
			if (GetAsyncKeyState(0x57) < 0) {
				move_item(&player, -1, 0, &grid);
				keyHeldDown = true;
			}
			// A key
			else if (GetAsyncKeyState(0x41) < 0) {
				move_item(&player, 0, -1, &grid);
				keyHeldDown = true;
			}
			// S key
			else if (GetAsyncKeyState(0x53) < 0) {
				move_item(&player, 1, 0, &grid);
				keyHeldDown = true;
			}
			// D key
			else if (GetAsyncKeyState(0x44) < 0) {
				move_item(&player, 0, 1, &grid);
				keyHeldDown = true;
			}
		}
		else if (!(GetAsyncKeyState(0x57) < 0 || GetAsyncKeyState(0x41) < 0 || GetAsyncKeyState(0x53) < 0 || GetAsyncKeyState(0x44) < 0))
			keyHeldDown = false;

		// detect collision with monster
		if (player.position[0] == monster.position[0] && player.position[1] == monster.position[1]) {
			gameOver = true;
		}

		// detect collision with traps 
		if (!monsterAwake) {
			for (auto &o : traps) {
				if (player.position[0] == o->position[0] && player.position[1] == o->position[1]) {
					// free up memory
					traps.clear();
					monsterAwake = true;

					// TODO: make monster awake
				}
			}
		}


		// populate the screen char array with the grid
		for (int r = 0; r < boardWidth; r++) {
			for (int c = 0; c < boardWidth; c++) {
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
void add_item(Character c, char(*g)[boardWidth][boardWidth]) {
	(*g)[c.position[0]][c.position[1]] = c.logo;
	return;
}

void move_item(Character* c, int x, int y, char(*g)[boardWidth][boardWidth]) {
	// if valid move
	if (0 <= ((*c).position[0] + x) && ((*c).position[0] + x) <= boardWidth - 1 && 0 <= ((*c).position[1] + y) && ((*c).position[1] + y) <= boardWidth - 1) {
		// remove old item's position
		(*g)[(*c).position[0]][(*c).position[1]] = blank;
		// update item position
		(*c).move(x, y);
		// add item back to grid
		(*g)[(*c).position[0]][(*c).position[1]] = (*c).logo;
	}
	return;
}
