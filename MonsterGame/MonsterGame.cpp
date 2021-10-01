// MonsterGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Player.h"
#include "Monster.h"

using namespace std;


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

void add_item(Character c, char (*g)[10][10]) {
    (*g)[c.position[0]][c.position[1]] = c.logo;
    return;
}

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

    show_grid(grid);
}
