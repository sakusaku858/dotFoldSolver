#include <array>
#include <iostream>
#include <vector>

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

const int DIRECTIONS[8][2] = {
    {0, -1}, // UP
    {1, -1}, // UR
    {1, 0},  // R
    {1, 1},  // LR
    {0, 1},  // DN
    {-1, 1}, // LL
    {-1, 0}, // L
    {-1, -1} // UL
};

// 方向dを逆向きにする
int reverse_directon(int d) { return (d + 4) % 8; }

// 外周頂点outの座標を得る
void get_coords_of_outer_vertex(int out, int &x, int &y) {
    int pos = out / 8;
    int mod = out % 8;

    if (pos == 0) {
        x = mod;
        y = 0;
        return;
    }

    if (pos == 1) {
        x = 8;
        y = mod;
        return;
    }

    if (pos == 2) {
        x = 8 - mod;
        y = 8;
        return;
    }

    if (pos == 3) {
        x = 0;
        y = 8 - mod;
        return;
    }
}

// 外周頂点outが持つe番目が示す方向を返す
int get_edge_dirrection_of_outer_vertex(int out, int e) {
    int pos = out / 8;

    array<int, 3> dir;

    if (pos == 0) {
        dir = {LOWER_RIGHT, DOWN, LOWER_LEFT};
    } else if (pos == 1) {
        dir = {LOWER_LEFT, LEFT, UPPER_LEFT};
    } else if (pos == 2) {
        dir = {UPPER_LEFT, UP, UPPER_RIGHT};
    } else {
        dir = {UPPER_RIGHT, RIGHT, LOWER_RIGHT};
    }

    return dir[e];
}

bool in_range(int min, int x, int max) { return min <= x && x <= max; }

// 外周頂点outが持つe番目の辺の先にある内部頂点の番号を返す
// 辺の先に内部頂点がないときは-1を返す
int get_innner_vertex_number(int out, int e) {

    int x, y;

    int dir = get_edge_dirrection_of_outer_vertex(out, e);
    get_coords_of_outer_vertex(out, x, y);
    x = x - 1 + DIRECTIONS[dir][0];
    y = y - 1 + DIRECTIONS[dir][1];

    if (!in_range(0, x, 6) || !in_range(0, y, 6))
        return -1;

    return x + y * 7;
}

// 折り割り当て番号のn/3番目の辺の値を得る
int get_edge_from_fold(int f, int n) { return (f >> n) % 2; }

// 折り割り当てから各内部頂点の状態を得る
vector<int> create_edges_by_folds(array<int, 32> &folds) {
    vector<int> edges(49 * 8, -1);

    // 90度のカドを45度にするような折りをしない
    edges[UPPER_LEFT] = 0;
    edges[6 * 8 + UPPER_RIGHT] = 0;
    edges[42 * 8 + LOWER_LEFT] = 0;
    edges[48 * 8 + LOWER_RIGHT] = 0;

    for (int i = 0; i < 32; i++) {
        if (i % 8 == 0) {
            continue;
        }

        for (int j = 0; j < 3; j++) {

            // 外部頂点iのj番目の辺を設定する

            int in = get_innner_vertex_number(i, j); // 接続先の内部頂点番号
            if (in == -1)
                continue;

            // 外部頂点から延びる辺の方向
            int dout = get_edge_dirrection_of_outer_vertex(i, j);

            // 内部頂点 in から見た外部頂点 out の方向
            int d = reverse_directon(dout);

            // 外部頂点 out の j 番目の辺の値
            int e = get_edge_from_fold(folds[i], j);

            edges[in * 8 + d] = e;
        }
    }

    return edges;
}

// 折り割り当てから各内部頂点の状態を得る
array<int, 398> create_edges_by_folds_arr(array<int, 32> &folds) {
    array<int, 398> edges;
    edges.fill(-1);

    // 90度のカドを45度にするような折りをしない
    edges[UPPER_LEFT] = 0;
    edges[6 * 8 + UPPER_RIGHT] = 0;
    edges[42 * 8 + LOWER_LEFT] = 0;
    edges[48 * 8 + LOWER_RIGHT] = 0;

    for (int i = 0; i < 32; i++) {
        if (i % 8 == 0) {
            continue;
        }

        for (int j = 0; j < 3; j++) {

            // 外部頂点iのj番目の辺を設定する

            int in = get_innner_vertex_number(i, j); // 接続先の内部頂点番号
            if (in == -1)
                continue;

            // 外部頂点から延びる辺の方向
            int dout = get_edge_dirrection_of_outer_vertex(i, j);

            // 内部頂点 in から見た外部頂点 out の方向
            int d = reverse_directon(dout);

            // 外部頂点 out の j 番目の辺の値
            int e = get_edge_from_fold(folds[i], j);

            edges[in * 8 + d] = e;
        }
    }

    return edges;
}