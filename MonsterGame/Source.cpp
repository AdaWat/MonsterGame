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

const int boardCols = 70;
const int boardRows = 45;
const int numberOfTraps = 5;

const int bufferWidth = 165;
const int bufferHeight = 50;

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

int main();

void add_item(Character, wchar_t(*)[boardRows][boardCols]);
void move_item(Character(*), int, int, wchar_t(*)[boardRows][boardCols]);
int* get_blank_cell(wchar_t(*)[boardRows][boardCols]);
float get_dist(int, int);
void draw_grid(wchar_t(*)[boardRows][boardCols], wchar_t[bufferWidth * bufferHeight], Character(*), bool);
void generate_maze(wchar_t(*)[boardRows][boardCols]);
pair<int, int> path_find(Character(*), Character(*), wchar_t(*)[boardRows][boardCols]);
void detect_traps(bool(*), Character(*), wchar_t(*)[boardRows][boardCols], vector<unique_ptr<Item>>(*));
void detect_gold(int(*), Character(*), Item(*), wchar_t(*)[boardRows][boardCols]);


int main()
{
	srand(time(nullptr));	// set seed for random numbers
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[bufferWidth * bufferHeight];
	for (int i = 0; i < bufferWidth * bufferHeight; i++)
		screen[i] = L' ';

	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;

	// initialise grid with blank values
	static wchar_t grid[boardRows][boardCols];
	// TODO: use memset()
	for (int i = 0; i < boardRows; i++)
		for (int j = 0; j < boardCols; j++)
			grid[i][j] = blank;

	generate_maze(&grid);

	// add player
	Player player(0, 0);
	add_item(player, &grid);

	// add monster (in bottom right corner)
	Monster monster(boardRows % 2 == 1 ? boardRows - 1 : boardRows - 2, boardCols % 2 == 1 ? boardCols - 1 : boardCols - 2);
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

		// keyboard input
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
		// if a button isn't held down, allow the player to move next loop
		else if (!(GetAsyncKeyState(W_KEY) < 0 || GetAsyncKeyState(A_KEY) < 0 || GetAsyncKeyState(S_KEY) < 0 || GetAsyncKeyState(D_KEY) < 0 || GetAsyncKeyState(Q_KEY) < 0))
			keyHeldDown = false;

		// detect collision with monster
		if (player.position[0] == monster.position[0] && player.position[1] == monster.position[1])
			gameOver = true;

		detect_traps(&monsterAwake, &player, &grid, &traps);	// detect collision with traps
		detect_gold(&score, &player, &gold, &grid); // detect collision with gold

		// move monster logic
		if (monsterAwake && monsterMove) {
			pair<int, int> monNextPos = path_find(&monster, &player, &grid);
			if (easyMode)
				move_item(&monster, monNextPos.first - monster.position[0], monNextPos.second - monster.position[1], &grid);
			else {
				if (monNextPos.first == player.position[0] && monNextPos.second == player.position[1]) 
					move_item(&monster, monNextPos.first - monster.position[0], monNextPos.second - monster.position[1], &grid);
				else if (monster.position[0] % 2 == 0 && monster.position[1] % 2 == 0)
					move_item(&monster, 2 * (monNextPos.first - monster.position[0]), 2 * (monNextPos.second - monster.position[1]), &grid); // move monster 2 squares at a time
				else
					move_item(&monster, monNextPos.first - monster.position[0], monNextPos.second - monster.position[1], &grid);
			}
		}
		monsterMove = false;


		// draw gold (in case the monster walked over it in the last loop)
		grid[gold.position[0]][gold.position[1]] = !(monster.position[0] == gold.position[0] && monster.position[1] == gold.position[1]) ? gold.logo : monster.logo;

		draw_grid(&grid, screen, &player, easyMode);

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

void draw_grid(wchar_t(*g)[boardRows][boardCols], wchar_t s[bufferWidth * bufferHeight], Character (*player), bool easyMode) {
	int radius = 6;
	// populate the screen char array with the grid
	for (int r = 0; r < boardRows; r++)
		for (int c = 0; c < boardCols; c++)
			if (!easyMode && (get_dist((*player).position[0] - r, (*player).position[1] - c) < radius || 
				(c == boardCols - 1 && boardCols % 2 == 0) || (r == boardRows - 1 && boardRows % 2 == 0)))
				s[(r + 2) * bufferWidth + c * 2 + 4] = (*g)[r][c];
			else if (!easyMode)
				s[(r + 2) * bufferWidth + c * 2 + 4] = ' ';
			else
				s[(r + 2) * bufferWidth + c * 2 + 4] = (*g)[r][c];

	// display walls as squares
	for (int r = 0; r < boardRows; r++)
		for (int c = 0; c < boardCols; c++)
			if ((*g)[r][c] == wall)
				if (!easyMode && (get_dist((*player).position[0] - r, (*player).position[1] - c) < radius || 
					(c == boardCols - 1 && boardCols % 2 == 0) || (r == boardRows - 1 && boardRows % 2 == 0)))
					s[(r + 2) * bufferWidth + c * 2 + 5] = (*g)[r][c];
				else if (!easyMode)
					s[(r + 2) * bufferWidth + c * 2 + 5] = ' ';
				else
					s[(r + 2) * bufferWidth + c * 2 + 5] = (*g)[r][c];

	// top wall
	for (int i = 2; i < boardCols * 2 + 4; i++)
		s[bufferWidth + i] = wall;
	// left wall
	for (int i = 2; i < boardRows + 2; i++) {
		s[bufferWidth * i + 2] = wall;
		s[bufferWidth * i + 3] = wall;
	}
	// bottom wall
	if (boardRows % 2 == 1)
		for (int i = 2; i < boardCols * 2 + 4; i++)
			s[bufferWidth * (boardRows + 2) + i] = wall;
	// right wall
	if (boardCols % 2 == 1)
		for (int i = 1; i < boardRows + 2; i++) {
			s[bufferWidth * i + boardCols * 2 + 4] = wall;
			s[bufferWidth * i + boardCols * 2 + 5] = wall;
		}
	// bottom-right corner
	if (boardRows % 2 == 1 && boardCols % 2 == 1) {
		s[bufferWidth * (boardRows + 2) + boardCols * 2 + 4] = wall;
		s[bufferWidth * (boardRows + 2) + boardCols * 2 + 5] = wall;
	}
	return;
}

vector<pair<int, int>> get_unvisited_neighbour_coords(pair<int, int> cell, wchar_t(*maze)[boardRows][boardCols]) {
	vector<pair<int, int>> unvisitedNeighbours;
	// top neighbour
	if (cell.first > 0 && (*maze)[cell.first - 2][cell.second] == blank)
		unvisitedNeighbours.push_back(make_pair(cell.first - 2, cell.second));
	// bottom neighbour
	if (cell.first < boardRows - 2 && (*maze)[cell.first + 2][cell.second] == blank)
		unvisitedNeighbours.push_back(make_pair(cell.first + 2, cell.second));
	// left neighbour
	if (cell.second > 0 && (*maze)[cell.first][cell.second - 2] == blank)
		unvisitedNeighbours.push_back(make_pair(cell.first, cell.second - 2));
	// right neighbour
	if (cell.second < boardCols - 2 && (*maze)[cell.first][cell.second + 2] == blank)
		unvisitedNeighbours.push_back(make_pair(cell.first, cell.second + 2));
	return unvisitedNeighbours;
}

void generate_maze(wchar_t(*g)[boardRows][boardCols]) {
	stack<pair<int, int>> cellPath;
	wchar_t maze[boardRows][boardCols];
	// initialise grid of walls with every other cell not being a wall
	for (int r = 0; r < boardRows; r++)
		for (int c = 0; c < boardCols; c++)
			maze[r][c] = r % 2 == 0 && c % 2 == 0 ? blank : wall;

	cellPath.push(make_pair(0, 0));	// starting point
	maze[cellPath.top().first][cellPath.top().second] = L'V';
	int visitedCellCounter = 1;

	while (visitedCellCounter < ceil(boardRows / 2.0) * ceil(boardCols / 2.0)) {
		vector<pair<int, int>> unvisitedNeighbours = get_unvisited_neighbour_coords(cellPath.top(), &maze); // get coords of unvisited neighbours for cell at top of stack

		// if number of unvisited neighbours is 0, pop top cell off stack. Repeat until there is a cell with unvisited neighbours
		while (size(unvisitedNeighbours) == 0) {
			cellPath.pop();
			unvisitedNeighbours = get_unvisited_neighbour_coords(cellPath.top(), &maze);
		}
		pair<int, int> neighbour = unvisitedNeighbours[rand() % unvisitedNeighbours.size()]; // choose a random neighbour

		// remove wall between chosen neighbour and current cell
		int wallR = (cellPath.top().first + neighbour.first) / 2;
		int wallC = (cellPath.top().second + neighbour.second) / 2;
		maze[wallR][wallC] = blank;

		cellPath.push(neighbour); // add chosen neighbour to stack
		maze[cellPath.top().first][cellPath.top().second] = L'V'; // mark cell as visited
		visitedCellCounter++;
	}

	// add walls to grid
	for (int r = 0; r < boardRows; r++)
		for (int c = 0; c < boardCols; c++)
			if (maze[r][c] == wall)
				(*g)[r][c] = wall;
	return;
}

vector<pair<int, int>> get_next_frontiers(pair<int, int> cell, wchar_t(*g)[boardRows][boardCols]) {
	vector<pair<int, int>> nextFrontiers;
	// top neighbour
	if (cell.first > 0 && (*g)[cell.first - 1][cell.second] != wall)
		nextFrontiers.push_back(make_pair(cell.first - 1, cell.second));

	// bottom neighbour
	if (cell.first < boardRows - 1 && (*g)[cell.first + 1][cell.second] != wall)
		nextFrontiers.push_back(make_pair(cell.first + 1, cell.second));

	// left neighbour
	if (cell.second > 0 && (*g)[cell.first][cell.second - 1] != wall)
		nextFrontiers.push_back(make_pair(cell.first, cell.second - 1));

	// right neighbour
	if (cell.second < boardCols - 1 && (*g)[cell.first][cell.second + 1] != wall)
		nextFrontiers.push_back(make_pair(cell.first, cell.second + 1));

	return nextFrontiers;
}

pair<int, int> path_find(Character(*mon), Character(*player), wchar_t(*g)[boardRows][boardCols]) {
	pair<int, int> monPos = make_pair((*mon).position[0], (*mon).position[1]);
	pair<int, int> playerPos = make_pair((*player).position[0], (*player).position[1]);

	// get starting frontiers
	vector<pair<int, int>> startingFrontiers = get_next_frontiers(monPos, g);

	// if monster can only move in 1 direction, return the only possible move
	if (size(startingFrontiers) == 1)
		return startingFrontiers[0];

	int startingFrontierCounter = 0;
	for (pair<int, int> startingFrontier : startingFrontiers) {
		if (startingFrontier == playerPos)
			return startingFrontier;

		startingFrontierCounter++;
		vector<pair<pair<int, int>, pair<int, int>>> frontiers;	// holds all frontiers that derive from current starting frontier
		vector<pair<int, int>> nextFrontiers = get_next_frontiers(startingFrontier, g);

		for (pair<int, int> nextFrontier : nextFrontiers)
			if (nextFrontier != monPos)
				frontiers.push_back(make_pair(nextFrontier, startingFrontier));  // add next frontiers to frontiers vector

		for (int i = 0; i < size(frontiers); i++) {
			pair<pair<int, int>, pair<int, int>> frontier = frontiers[i];
			// if a frontier reaches the player, return the starting frontier
			if (frontier.first == playerPos)
				return startingFrontier;

			nextFrontiers = get_next_frontiers(frontier.first, g);

			for (pair<int, int> nextFrontier : nextFrontiers)
				if (nextFrontier != frontier.second)
					frontiers.push_back(make_pair(nextFrontier, frontier.first));  // add next frontiers to frontiers vector
		}

		// if there is only 1 starting frontier left to check, return that one instantly
		if (startingFrontierCounter == size(startingFrontiers) - 1)
			return startingFrontiers[size(startingFrontiers) - 1];
	}
}

void detect_traps(bool(*monsterAwake), Character(*player), wchar_t(*g)[boardRows][boardCols], vector<unique_ptr<Item>>(*traps)) {
	// detect collision with traps 
	if (!(*monsterAwake))
		for (unique_ptr<Item>& o : (*traps))
			if ((*player).position[0] == o->position[0] && (*player).position[1] == o->position[1]) {
				// TODO: stop displaying the traps (b/c they are about to be cleared)
				for (int r = 0; r < boardRows; r++)
					for (int c = 0; c < boardCols; c++)
						if ((*g)[r][c] == o->logo)
							(*g)[r][c] = blank;
				// free up memory
				(*traps).clear();
				(*monsterAwake) = true;
			}
}

void detect_gold(int(*score), Character(*player), Item(*gold), wchar_t(*g)[boardRows][boardCols]) {
	if ((*player).position[0] == (*gold).position[0] && (*player).position[1] == (*gold).position[1]) {
		(*score)++;
		int* goldPos = get_blank_cell(&(*g));
		(*gold).position[0] = goldPos[0];
		(*gold).position[1] = goldPos[1];
		(*g)[(*gold).position[0]][(*gold).position[1]] = (*gold).logo;	// redraw
	}
}