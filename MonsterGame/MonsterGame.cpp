// MonsterGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

using namespace std;

// TODO: make array size variable (use pointers to initialise dymanic size arrays)
void show_grid(char arr[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            cout << arr[i][j] << ' ';
        }
        cout << endl;
    }
}

char move(char pos[2], int x_displacement, int y_displacement) {
    return { pos[0] + x_displacement , pos[1] + y_displacement };
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
    int monster[] = {5, 5};
    grid[monster[0]][monster[1]] = 'M';

    // add player
    int player[] = { 0, 0 };
    grid[player[0]][player[1]] = 'P';

    show_grid(grid);
}
