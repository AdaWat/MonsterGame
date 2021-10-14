// MonsterGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
// 

#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <thread>
#include <vector>
#include <stack>

#include "Player.h"
#include "Monster.h"
#include "Item.h"

using namespace std;

const int boardCols = 50;
const int boardRows = 30;
const int numberOfTraps = 1;

const int bufferWidth = 120;
const int bufferHeight = 35;

const wchar_t blank = L' ';
const wchar_t wall = L'█';
bool easyMode = false;

enum keys {
	W_KEY = 0x57,
	A_KEY = 0x41,
	S_KEY = 0x53,
	D_KEY = 0x44,
	Q_KEY = 0x51,
};

void add_item(Character, wchar_t(*)[boardRows][boardCols]);
void move_item(Character(*), int, int, wchar_t(*)[boardRows][boardCols]);
int* get_blank_cell(wchar_t(*)[boardRows][boardCols]);
float get_dist(int, int);
void draw_grid(wchar_t(*)[boardRows][boardCols], wchar_t[bufferWidth * bufferHeight]);
void generate_maze(wchar_t(*)[boardRows][boardCols]);

int main()
{
	srand(time(nullptr));	// set seed for random numbers
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[bufferWidth * bufferHeight];
	for (int i = 0; i < bufferWidth * bufferHeight; i++) {
		screen[i] = L' ';
	}
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;


	// initialise grid with blank values
	static wchar_t grid[boardRows][boardCols];
	// TODO: use memset()
	for (int i = 0; i < boardRows; i++) {
		for (int j = 0; j < boardCols; j++) {
			grid[i][j] = blank;
		}
	}

	generate_maze(&grid);

	// add player
	Player player(0, 0);
	add_item(player, &grid);

	// add monster in bottom right corner that isn't a wall
	Monster monster(grid[boardRows - 1][boardCols - 1] == blank ? boardRows - 1 : boardRows - 2, grid[boardRows - 1][boardCols - 1] == blank ? boardCols - 1 : boardCols - 2);
	add_item(monster, &grid);


	// add traps
	vector<unique_ptr<Item>> traps;
	for (int i = 0; i < numberOfTraps; i++) {
		int* pos = get_blank_cell(&grid);
		traps.push_back(make_unique<Item>(pos[0], pos[1], L'T'));
		grid[pos[0]][pos[1]] = traps[i]->logo;
	}

	// add gold
	int* goldPos = get_blank_cell(&grid);
	Item gold(goldPos[0], goldPos[1], L'G');
	grid[gold.position[0]][gold.position[1]] = gold.logo;

	bool gameOver = false;
	bool keyHeldDown = false;
	bool monsterAwake = false;
	bool monsterMove = false;

	int score = 0;

	auto getMonsterDist = [&](int x, int y) {
		return abs(get_dist(player.position[0] - (monster.position[0] + x), player.position[1] - (monster.position[1] + y)));
	};

	while (!gameOver) {
		// slow down loop
		this_thread::sleep_for(50ms);

		// ---Keyboard input---
		if (!keyHeldDown) {
			if (GetAsyncKeyState(W_KEY) < 0) {
				move_item(&player, -1, 0, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
			else if (GetAsyncKeyState(A_KEY) < 0) {
				move_item(&player, 0, -1, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
			else if (GetAsyncKeyState(S_KEY) < 0) {
				move_item(&player, 1, 0, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
			else if (GetAsyncKeyState(D_KEY) < 0) {
				move_item(&player, 0, 1, &grid);
				keyHeldDown = true;
				monsterMove = true;
			}
			else if (GetAsyncKeyState(Q_KEY) < 0) {
				easyMode = !easyMode;
				keyHeldDown = true;
			}
		}
		else if (!(GetAsyncKeyState(W_KEY) < 0 || GetAsyncKeyState(A_KEY) < 0 || GetAsyncKeyState(S_KEY) < 0 || GetAsyncKeyState(D_KEY) < 0 || GetAsyncKeyState(Q_KEY) < 0)) {
			// if a button isn't held down, allow the player to move next loop
			keyHeldDown = false;
		}
		// TODO: make most of these subroutines
		// detect collision with monster
		if (player.position[0] == monster.position[0] && player.position[1] == monster.position[1]) {
			gameOver = true;
		}

		// detect collision with traps 
		if (!monsterAwake) {
			for (unique_ptr<Item>& o : traps) {
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

		// move monster logic
		if (monsterAwake && monsterMove) {
			if (easyMode) {
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
			else {
				float upDist = getMonsterDist(-1, 0);
				float downDist = getMonsterDist(1, 0);
				float leftDist = getMonsterDist(0, -1);
				float rightDist = getMonsterDist(0, 1);
				float minDist = min(upDist, min(downDist, min(leftDist, rightDist)));

				if (minDist == upDist) {
					move_item(&monster, -1, 0, &grid);
				}
				else if (minDist == downDist) {
					move_item(&monster, 1, 0, &grid);
				}
				else if (minDist == leftDist) {
					move_item(&monster, 0, -1, &grid);
				}
				else if (minDist == rightDist) {
					move_item(&monster, 0, 1, &grid);
				}
			}
		}
		monsterMove = false;


		// draw gold (in case the monster walked over it in the last loop)
		grid[gold.position[0]][gold.position[1]] = !(monster.position[0] == gold.position[0] && monster.position[1] == gold.position[1]) ? gold.logo : monster.logo;

		draw_grid(&grid, screen);

		// Display score
		std::swprintf(&screen[2 * bufferWidth + boardCols * 2 + 10], 12, L"Score: %d", score);
		// Display mode
		if (easyMode)
			std::swprintf(&screen[6 * bufferWidth + boardCols * 2 + 10], 12, L"Mode: easy");
		else
			std::swprintf(&screen[6 * bufferWidth + boardCols * 2 + 10], 12, L"Mode: hard");


		// Display Frame
		screen[bufferWidth * bufferHeight - 1] = '\0';	// end char array so Windows knows when to stop rendering
		WriteConsoleOutputCharacter(console, screen, bufferWidth * bufferHeight, { 0, 0 }, &bytesWritten);
	}

	CloseHandle(console);
	std::cout << "\nGAME OVER\n";
	return 0;
}

// Display the logo of an item/character
void add_item(Character c, wchar_t(*g)[boardRows][boardCols]) {
	(*g)[c.position[0]][c.position[1]] = c.logo;
	return;
}

void move_item(Character* c, int x, int y, wchar_t(*g)[boardRows][boardCols]) {
	// if valid move
	if (0 <= ((*c).position[0] + x) && ((*c).position[0] + x) <= boardRows - 1 && 0 <= ((*c).position[1] + y) && ((*c).position[1] + y) <= boardCols - 1
		&& (*g)[(*c).position[0] + x][(*c).position[1] + y] != wall) {
		(*g)[(*c).position[0]][(*c).position[1]] = blank; // remove old item's position
		(*c).move(x, y); // update item position
		(*g)[(*c).position[0]][(*c).position[1]] = (*c).logo; // add item back to grid
	}
	return;
}

int* get_blank_cell(wchar_t(*g)[boardRows][boardCols]) {
	static int pos[2];
	do {
		pos[0] = rand() % boardRows;
		pos[1] = rand() % boardCols;
	} while ((*g)[pos[0]][pos[1]] != blank);
	return pos;
}

float get_dist(int x, int y) {
	return sqrt(x * x + y * y);
}

void draw_grid(wchar_t(*g)[boardRows][boardCols], wchar_t s[bufferWidth * bufferHeight]) {
	// populate the screen char array with the grid
	for (int r = 0; r < boardRows; r++) {
		for (int c = 0; c < boardCols; c++) {
			s[(r + 2) * bufferWidth + c * 2 + 4] = (*g)[r][c];
		}
	}
	for (int r = 0; r < boardRows; r++) {
		for (int c = 0; c < boardCols; c++) {
			if ((*g)[r][c] == wall)
				s[(r + 2) * bufferWidth + c * 2 + 5] = (*g)[r][c];	// display walls as squares
		}
	}
	// top wall
	for (int i = 0; i < boardCols*2+4; i++) {
		s[bufferWidth + 2 + i] = wall;
	}
	// left wall
	for (int i = 0; i < boardRows; i++) {
		s[bufferWidth*(i+2) + 2] = wall;
		s[bufferWidth * (i + 2) + 3] = wall;
	}
	// bottom wall
	if (boardRows % 2 == 1) {
		for (int i = 0; i < boardCols * 2 + 4; i++) {
			s[(bufferWidth*(boardRows+2)) + 2 + i] = wall;
		}
	}
	// right wall
	
	for (int i = 0; i < boardRows; i++) {
		s[bufferWidth * (i + 2) + boardCols*2 + 4] = wall;
		s[bufferWidth * (i + 2) + boardCols * 2 + 5] = wall;
	}
	
	return;
}

vector<pair<int, int>> get_unvisited_neighbour_coords(pair<int, int> cell, wchar_t(*maze)[boardRows][boardCols]) {
	vector<pair<int, int>> unvisitedNeighbours;
	// top neighbour
	if (cell.first > 0 && (*maze)[cell.first - 2][cell.second] == blank) {
		unvisitedNeighbours.push_back(make_pair(cell.first - 2, cell.second));
	}
	// bottom neighbour
	if (cell.first < boardRows - 2 && (*maze)[cell.first + 2][cell.second] == blank) {
		unvisitedNeighbours.push_back(make_pair(cell.first + 2, cell.second));
	}
	// left neighbour
	if (cell.second > 0 && (*maze)[cell.first][cell.second - 2] == blank) {
		unvisitedNeighbours.push_back(make_pair(cell.first, cell.second - 2));
	}
	// right neighbour
	if (cell.second < boardCols - 2 && (*maze)[cell.first][cell.second + 2] == blank) {
		unvisitedNeighbours.push_back(make_pair(cell.first, cell.second + 2));
	}
	return unvisitedNeighbours;
}

void generate_maze(wchar_t(*g)[boardRows][boardCols]) {
	// TODO: why are some areas of maze closed off?
	stack<pair<int, int>> cellPath;
	wchar_t maze[boardRows][boardCols];
	// initialise grid of walls with every other cell not being a wall
	for (int r = 0; r < boardRows; r++) {
		for (int c = 0; c < boardCols; c++) {
			maze[r][c] = r % 2 == 0 && c % 2 == 0 ? blank : wall;
		}
	}
	cellPath.push(make_pair(0, 0));	// starting point
	maze[cellPath.top().first][cellPath.top().second] = L'V';
	int visitedCellCounter = 1;

	while (visitedCellCounter < ceil(boardRows / 2.0) * ceil(boardCols / 2.0)) {
		// --get coords of unvisited neighbours for cell at top of stack--
		vector<pair<int, int>> unvisitedNeighbours = get_unvisited_neighbour_coords(cellPath.top(), &maze);

		// --if number of unvisited neighbours is 0, pop top cell off stack. Repeat until there is a cell with unvisited neighbours--
		while (size(unvisitedNeighbours) == 0) {
			cellPath.pop();
			unvisitedNeighbours = get_unvisited_neighbour_coords(cellPath.top(), &maze);
		}
		// --choose a random neighbour--
		pair<int, int> neighbour = unvisitedNeighbours[rand() % unvisitedNeighbours.size()];

		// --remove wall between chosen neighbour and current cell--
		int wallR = (cellPath.top().first + neighbour.first) / 2;
		int wallC = (cellPath.top().second + neighbour.second) / 2;
		maze[wallR][wallC] = blank;

		// --add chosen neighbour to stack--
		cellPath.push(neighbour);

		// --mark cell as visited--
		maze[cellPath.top().first][cellPath.top().second] = L'V';
		visitedCellCounter++;
	}
	// mark last cell as visited (probably not necessary)
	maze[cellPath.top().first][cellPath.top().second] = L'V';

	// Temporary solution to add walls to grid?
	for (int r = 0; r < boardRows; r++) {
		for (int c = 0; c < boardCols; c++) {
			if (maze[r][c] == wall)
				(*g)[r][c] = wall;
		}
	}
	return;
}
