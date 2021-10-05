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

const int boardRows = 8;
const int boardCols = 12;

const int bufferWidth = 80;
const int bufferHeight = 60;

const int numberOfTraps = 10;
const char blank = L'#';

void add_item(Character, char(*)[boardRows][boardCols]);
void move_item(Character(*), int, int, char(*)[boardRows][boardCols]);
int* get_blank_cell(char(*)[boardRows][boardCols]);

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[bufferWidth * bufferHeight];
	for (int i = 0; i < bufferWidth * bufferHeight; i++) screen[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;


	// initialise grid with blank values
	static char grid[boardRows][boardCols];
	for (int i = 0; i < boardRows; i++) {
		for (int j = 0; j < boardCols; j++) {
			grid[i][j] = blank;
		}
	}

	// add monster
	Monster monster(boardRows - 1, boardCols - 1);
	add_item(monster, &grid);

	// add player
	Player player(0, 0);
	add_item(player, &grid);

	// add traps
	std::vector<unique_ptr<Item>> traps;
	for (int i = 0; i < numberOfTraps; i++) {
		int* pos = get_blank_cell(&grid);
		traps.push_back(std::make_unique<Item>(pos[0], pos[1], 'T'));
		grid[pos[0]][pos[1]] = traps[i]->logo;
	}

	// add gold
	int* goldPos = get_blank_cell(&grid);
	Item gold(goldPos[0], goldPos[1], 'G');
	grid[gold.position[0]][gold.position[1]] = gold.logo;

	bool gameOver = false;
	bool keyHeldDown = false;
	bool monsterAwake = false;
	bool monsterMove = false;

	int score = 0;

	while (!gameOver) {
		// slow down loop
		this_thread::sleep_for(50ms);

		// ---Keyboard input---
		if (!keyHeldDown) {
			// W key
			if (GetAsyncKeyState(0x57) < 0) {
				move_item(&player, -1, 0, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
			// A key
			else if (GetAsyncKeyState(0x41) < 0) {
				move_item(&player, 0, -1, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
			// S key
			else if (GetAsyncKeyState(0x53) < 0) {
				move_item(&player, 1, 0, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
			// D key
			else if (GetAsyncKeyState(0x44) < 0) {
				move_item(&player, 0, 1, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
		}
		else if (!(GetAsyncKeyState(0x57) < 0 || GetAsyncKeyState(0x41) < 0 || GetAsyncKeyState(0x53) < 0 || GetAsyncKeyState(0x44) < 0)) {
			// if a button isn't held down, allow the player to move next loop
			keyHeldDown = false;
		}

		// detect collision with monster
		if (player.position[0] == monster.position[0] && player.position[1] == monster.position[1]) {
			gameOver = true;
		}

		// detect collision with traps 
		if (!monsterAwake) {
			for (std::unique_ptr<Item> &o : traps) {
				if (player.position[0] == o->position[0] && player.position[1] == o->position[1]) {
					// TODO: stop displaying the traps (b/c they are about to be cleared)
					for (int r = 0; r < boardRows; r++) {
						for (int c = 0; c < boardCols; c++) {
							if (grid[r][c] == o->logo) {
								grid[r][c] = blank;
							}
						}
					}
					// free up memory
					traps.clear();
					monsterAwake = true;
				}
			}
		}

		// detect collision with gold
		if (player.position[0] == gold.position[0] && player.position[1] == gold.position[1]) {
			score++;
			goldPos = get_blank_cell(&grid);
			gold.position[0] = goldPos[0];
			gold.position[1] = goldPos[1];
			grid[gold.position[0]][gold.position[1]] = gold.logo;	// redraw
		}

		if (monsterAwake && monsterMove) {
			// move monster
			bool moveBias = rand() % 2 - 1;		// true=vertical bias  false=horizontal bias
			int vertOffset = player.position[0] - monster.position[0];
			int horOffset = player.position[1] - monster.position[1];

			if ((vertOffset < 0 && moveBias) || (vertOffset < 0 && horOffset == 0)) {
				move_item(&monster, -1, 0, &grid);
			}
			else if ((vertOffset > 0 && moveBias) || (vertOffset > 0 && horOffset == 0)) {
				move_item(&monster, 1, 0, &grid);
			}
			else if ((horOffset < 0 && !moveBias) || (horOffset < 0 && vertOffset == 0)) {
				move_item(&monster, 0, -1, &grid);
			}
			else if ((horOffset > 0 && !moveBias) || (horOffset > 0 && vertOffset == 0)) {
				move_item(&monster, 0, 1, &grid);
			}
		}
		monsterMove = false;


		// draw gold (in case the monster walked over it in the last loop)
		grid[gold.position[0]][gold.position[1]] = !(monster.position[0] == gold.position[0] && monster.position[1] == gold.position[1]) ? gold.logo : monster.logo;

		// populate the screen char array with the grid
		for (int r = 0; r < boardRows; r++) {
			for (int c = 0; c < boardCols; c++) {
				screen[(r + 2) * bufferWidth + (c * 2 + 4)] = grid[r][c];
			}
		}

		// Display score
		//TODO: draw score to buffer
		//swprintf_s(&screen, "Score: %s", score);

		// Display Frame
		screen[bufferWidth * bufferHeight - 1] = '\0';	// end char array so Windows knows when to stop rendering
		WriteConsoleOutputCharacter(console, screen, bufferWidth * bufferHeight, { 0, 0 }, &bytesWritten);
	}

	CloseHandle(console);
	std::cout << "\nGAME OVER\n";
	return 0;
}

// Display the logo of an item/character
void add_item(Character c, char(*g)[boardRows][boardCols]) {
	(*g)[c.position[0]][c.position[1]] = c.logo;
	return;
}

void move_item(Character* c, int x, int y, char(*g)[boardRows][boardCols]) {
	// if valid move
	if (0 <= ((*c).position[0] + x) && ((*c).position[0] + x) <= boardRows - 1 && 0 <= ((*c).position[1] + y) && ((*c).position[1] + y) <= boardCols - 1) {
		// remove old item's position
		(*g)[(*c).position[0]][(*c).position[1]] = blank;
		// update item position
		(*c).move(x, y);
		// add item back to grid
		(*g)[(*c).position[0]][(*c).position[1]] = (*c).logo;
	}
	return;
}

int* get_blank_cell(char(*g)[boardRows][boardCols]) {
	static int pos[2];
	do {
		pos[0] = rand() % boardRows;
		pos[1] = rand() % boardCols;
	} while ((*g)[pos[0]][pos[1]] != blank);
	return pos;
}