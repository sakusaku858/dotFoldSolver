// 5方向が確定しているときにおけるタイルの数を数える

#include <iostream>

using namespace std;

#define UP 0
#define UPPER_RIGHT 1
#define RIGHT 2
#define LOWER_RIGHT 3
#define DOWN 4
#define LOWER_LEFT 5
#define LEFT 6
#define UPPER_LEFT 7
#define DIR_MAX 8

const int TILE[36][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 1},
                         {0, 0, 1, 0, 0, 0, 1, 0}, {0, 0, 1, 0, 0, 1, 1, 1},
                         {0, 0, 1, 0, 1, 1, 0, 1}, {0, 0, 1, 1, 1, 0, 0, 1},
                         {0, 1, 0, 0, 0, 1, 0, 0}, {0, 1, 0, 0, 1, 0, 1, 1},
                         {0, 1, 0, 0, 1, 1, 1, 0}, {0, 1, 0, 1, 0, 1, 0, 1},
                         {0, 1, 0, 1, 1, 0, 1, 0}, {0, 1, 0, 1, 1, 1, 1, 1},
                         {0, 1, 1, 0, 1, 0, 0, 1}, {0, 1, 1, 1, 0, 0, 1, 0},
                         {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 0, 1},
                         {1, 0, 0, 0, 1, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 1, 1},
                         {1, 0, 0, 1, 0, 1, 1, 0}, {1, 0, 0, 1, 1, 1, 0, 0},
                         {1, 0, 1, 0, 0, 1, 0, 1}, {1, 0, 1, 0, 1, 0, 1, 0},
                         {1, 0, 1, 0, 1, 1, 1, 1}, {1, 0, 1, 1, 0, 1, 0, 0},
                         {1, 0, 1, 1, 1, 0, 1, 1}, {1, 0, 1, 1, 1, 1, 1, 0},
                         {1, 1, 0, 0, 1, 0, 0, 1}, {1, 1, 0, 1, 0, 0, 1, 0},
                         {1, 1, 0, 1, 0, 1, 1, 1}, {1, 1, 0, 1, 1, 1, 0, 1},
                         {1, 1, 1, 0, 0, 1, 0, 0}, {1, 1, 1, 0, 1, 0, 1, 1},
                         {1, 1, 1, 0, 1, 1, 1, 0}, {1, 1, 1, 1, 0, 1, 0, 1},
                         {1, 1, 1, 1, 1, 0, 1, 0}, {1, 1, 1, 1, 1, 1, 1, 1}};

int main(void) {

#if 0
    // 5方向が決まっているときにおけるタイルの総数
    for (int i = 0; i < 32; i++) {
        int c = 0;
        int ll = i % 2;
        int l = i / 2 % 2;
        int ul = i / 4 % 2;
        int u = i / 8 % 2;
        int ur = i / 16 % 2;
        cout << ll << l << ul << u << ur << endl;
        for (int j = 0; j < 36; j++) {
            if (TILE[j][LOWER_LEFT] != ll)
                continue;
            if (TILE[j][LEFT] != l)
                continue;
            if (TILE[j][UPPER_LEFT] != ul)
                continue;
            if (TILE[j][UP] != u)
                continue;
            if (TILE[j][UPPER_RIGHT] != ur)
                continue;
            c++;
        }
        cout << c << endl;
    }
#endif

#if 0
    // 4方向が決まっているときにおけるタイルの総数
    for (int i = 0; i < 16; i++) {
        int c = 0;
        int l = i % 2;
        int ul = i / 2 % 2;
        int u = i / 4 % 2;
        int ur = i / 8 % 2;
        for (int j = 0; j < 36; j++) {
            if (TILE[j][LEFT] != l)
                continue;
            if (TILE[j][UPPER_LEFT] != ul)
                continue;
            if (TILE[j][UP] != u)
                continue;
            if (TILE[j][UPPER_RIGHT] != ur)
                continue;
            c++;
        }
        cout << c << endl;
    }
#endif

#if 1
    // 6方向が決まっているときにおけるタイルの総数
    for (int i = 0; i < 64; i++) {
        int c = 0;
        int ll = i % 2;
        int l = i / 2 % 2;
        int ul = i / 4 % 2;
        int u = i / 8 % 2;
        int ur = i / 16 % 2;
        int d = i / 32 % 2;
        for (int j = 0; j < 36; j++) {
            if (TILE[j][LOWER_LEFT] != ll)
                continue;
            if (TILE[j][LEFT] != l)
                continue;
            if (TILE[j][UPPER_LEFT] != ul)
                continue;
            if (TILE[j][UP] != u)
                continue;
            if (TILE[j][UPPER_RIGHT] != ur)
                continue;
            if (TILE[j][DOWN] != d)
                continue;
            c++;
        }
        cout << c << endl;
    }
#endif

    return 0;
}