#include <omp.h>
#include <sys/time.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <string>
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

class Counter {
    int w;
    int mate_size;
    unsigned long long state_size;
    vector<vector<int>> tileConditon;

  public:
    Counter(int width) {
        w = width;
        mate_size = 3 * w - 1;
        state_size = 1ULL << mate_size;
        tileConditon = vector<vector<int>>(w * w, vector<int>(36, 1));
    }

    void setTileCondition(int cell, int tile, int value) {
        tileConditon[cell][tile] = value;
    }

    int get_binary_digit(unsigned long long n, int d) { return (n >> d) & 1; }

    unsigned long long set_bit(unsigned long long n, int d, int v) {
        unsigned long long m = n;
        unsigned long long mask = ~(1ULL << d);
        m = m & mask;
        unsigned long long bit = (unsigned long long)v << d;
        return m | bit;
    }

    // 設置判定するときと置くときで座標が異なる
    // => どっちの使用状況か引数で得る必要がある
    // => あるいは何番目のタイルまで設置したか
    int get_index(int dir, int x) {
        if (dir == UP)
            return 3 * x;
        if (dir == UPPER_RIGHT)
            return min(3 * x + 1, 3 * w - 3);
        if (dir == UPPER_LEFT)
            return max(3 * x - 1, 0);
        if (dir == LEFT)
            return max(3 * x - 2, 0);

        return 0;
    }

    bool can_put(int cell, int tile, unsigned long long mate) {
        // preCreaseによって、置いて良いタイルが制限されている場合
        if (tileConditon[cell][tile] == 0)
            return false;

        int x = cell % w;
        int y = cell / w;

        // 設置判定をする方向を計算
        int l = 1, ul = 1, u = 1, ur = 1;
        if (y == 0)
            ul = 0, u = 0, ur = 0;
        if (x == 0)
            l = 0, ul = 0;
        if (x == w - 1)
            ur = 0;

        // tileの4方向の辺を得る
        int tl = TILE[tile][LEFT];
        int tul = TILE[tile][UPPER_LEFT];
        int tu = TILE[tile][UP];
        int tur = TILE[tile][UPPER_RIGHT];

        // mateの4方向のindexを得る
        int li, ui, uli, uri;
        li = get_index(LEFT, x);
        ui = get_index(UP, x);
        uli = get_index(UPPER_LEFT, x);
        uri = get_index(UPPER_RIGHT, x);

        // mateの4方向の辺を得る
        int ml, mul, mu, mur;
        ml = get_binary_digit(mate, li);
        mul = get_binary_digit(mate, uli);
        mu = get_binary_digit(mate, ui);
        mur = get_binary_digit(mate, uri);

        // 接続判定
        if (l == 1 && tl != ml)
            return false;
        if (ul == 1 && tul != mul)
            return false;
        if (u == 1 && tu != mu)
            return false;
        if (ur == 1 && tur != mur)
            return false;
        return true;
    }

    unsigned long long put(int cell, int take, unsigned long long mate) {
        int x = cell % w;
        int y = cell / w;

        // 更新すべき方向
        int r = 1, ll = 1, d = 1, lr = 1;
        if (x == 0)
            ll = 0;
        if (x == w - 1)
            r = 0, lr = 0;

        // 更新すべき方向のindex
        int ri, lli, di, lri;
        ri = get_index(LEFT, x + 1);
        lli = get_index(UPPER_RIGHT, x - 1);
        di = get_index(UP, x);
        lri = mate_size - 1;

        int ul = get_index(UPPER_LEFT, x);

        // mateの更新
        unsigned long long mate2 = mate;
        int lr2 = get_binary_digit(mate2, mate_size - 1);
        mate2 = set_bit(mate, ul, lr2);
        int ds[4] = {RIGHT, LOWER_LEFT, DOWN, LOWER_RIGHT};
        int updates[4] = {r, ll, d, lr};
        int ids[4] = {ri, lli, di, lri};
        for (int i = 0; i < 4; i++) {
            if (updates[i] == 1)
                mate2 = set_bit(mate2, ids[i], TILE[take][ds[i]]);
        }

        return mate2;
    }

    unsigned long long count() {

        // tableの準備
        vector<vector<unsigned long long>> table(
            2, vector<unsigned long long>(state_size, 0));

        table[0][0] = 1;

        // mate配列の鍵を作る
        vector<omp_lock_t> locks(state_size);
        for (unsigned long long i = 0; i < state_size; i++) {
            omp_init_lock(&locks[i]);
        }

        for (int i = 0; i < w * w; i++) {
            int prev = i % 2;
            int next = (i + 1) % 2;

            // next配列の初期化
            for (unsigned long long k = 0; k < state_size; k++)
                table[next][k] = 0;

// 各mateの更新
#pragma omp parallel for schedule(guided)
            for (unsigned long long k = 0; k < state_size; k++) {
                for (int j = 0; j < 36; j++) { // タイルjを設置

                    // セルiにタイルjが置けるか？
                    if (!can_put(i, j, k))
                        continue;

                    // セルiにタイルjを設置したときのmateを得る
                    unsigned long long newstate = put(i, j, k);

                    // 書き込み先のロックを取得
                    omp_set_lock(&locks[newstate]);

                    // 得られたmateへ到達する経路数を加算する

                    table[next][newstate] += table[prev][k];

                    // ロックを開放
                    omp_unset_lock(&locks[newstate]);
                }
            }
        }

        // ロックの解放
        for (unsigned long long i = 0; i < state_size; i++) {
            omp_destroy_lock(&locks[i]);
        }

        // evenまたはoddのmate数の和を計算する
        unsigned long long sum = 0;
        int next = w % 2;

        for (unsigned long long state = 0; state < state_size; state++) {
            sum += table[next][state];
        }

        return sum * 16;
    }

    string findCP() {

        // tableの準備
        vector<vector<string>> table(2, vector<string>(state_size, "NG"));

        table[0][0] = "";

        // mate配列の鍵を作る
        vector<omp_lock_t> locks(state_size);
        for (unsigned long long i = 0; i < state_size; i++) {
            omp_init_lock(&locks[i]);
        }

        for (int i = 0; i < w * w; i++) {
            int prev = i % 2;
            int next = (i + 1) % 2;

            // next配列の初期化
            for (unsigned long long k = 0; k < state_size; k++)
                table[next][k] = "NG";

// 各mateの更新
#pragma omp parallel for schedule(guided)
            for (unsigned long long k = 0; k < state_size; k++) {
                if (table[prev][k] == "NG")
                    continue;

                for (int j = 0; j < 36; j++) { // タイルjを設置

                    // セルiにタイルjが置けるか？
                    if (!can_put(i, j, k))
                        continue;

                    // セルiにタイルjを設置したときのmateを得る
                    unsigned long long newstate = put(i, j, k);

                    // すでに経路があるなら書き込まない
                    if (table[next][newstate] != "NG")
                        continue;

                    string tilestr;
                    string head = "";
                    if (j < 10)
                        head = "0";
                    tilestr = head + to_string(j);

                    // 書き込み先のロックを取得
                    omp_set_lock(&locks[newstate]);

                    // 得られたmateへ到達する展開図を書き込む
                    table[next][newstate] = table[prev][k] + tilestr;

                    // ロックを開放
                    omp_unset_lock(&locks[newstate]);
                }
            }
        }

        // ロックの解放
        for (unsigned long long i = 0; i < state_size; i++) {
            omp_destroy_lock(&locks[i]);
        }

        // evenまたはoddのmate数の和を計算する
        unsigned long long sum = 0;
        int next = w % 2;

        for (unsigned long long state = 0; state < state_size; state++) {
            if (table[next][state] != "NG")
                return table[next][state];
        }
        return "No CP";
    }

    // findCPFastで使う
    // 3次元配列を1次元配列で表現した
    int get_table_index(int row, int state, int step) {
        return row * state_size * w * w + state * w * w + step;
    }

    string to_str(int a) {
        string ans;
        if (a < 10) {
            ans = "0" + to_string(a);
        } else {
            ans = to_str(a);
        }
        return ans;
    }

    string findCPFast() {

        // tableの準備
        int cell_size = state_size * 2 * w * w;
        int *table = (int *)malloc(sizeof(int) * cell_size);
        for (int i = 0; i < cell_size; i++) {
            table[i] = -1;
        }
        table[0] = 1;

        // mate配列の鍵を作る
        vector<omp_lock_t> locks(state_size);
        for (unsigned long long i = 0; i < state_size; i++) {
            omp_init_lock(&locks[i]);
        }

        for (int i = 0; i < w * w; i++) {
            int prev = i % 2;
            int next = (i + 1) % 2;

            // next配列の初期化
            for (unsigned long long k = 0; k < state_size; k++) {
                int l = get_table_index(next, k, 0);
                table[l] = -1;
            }

// 各mateの更新
#pragma omp parallel for schedule(guided)
            for (unsigned long long k = 0; k < state_size; k++) {
                int prev_i = get_table_index(prev, k, 0);
                if (table[prev_i] == -1)
                    continue;

                for (int j = 0; j < 36; j++) { // タイルjを設置

                    // セルiにタイルjが置けるか？
                    if (!can_put(i, j, k))
                        continue;

                    // セルiにタイルjを設置したときのmateを得る
                    unsigned long long newstate = put(i, j, k);
                    int next_i = get_table_index(next, newstate, 0);

                    // すでに経路があるなら書き込まない
                    if (table[next_i] != -1)
                        continue;

                    // 書き込み先のロックを取得
                    omp_set_lock(&locks[newstate]);

                    // 得られたmateへ到達する展開図を書き込む
                    for (int step = 0; step < i; i++) {
                        table[next_i + step] = table[prev_i + step];
                    }
                    table[next_i + i] = j;

                    // ロックを開放
                    omp_unset_lock(&locks[newstate]);
                }
            }
        }

        // ロックの解放
        for (unsigned long long i = 0; i < state_size; i++) {
            omp_destroy_lock(&locks[i]);
        }

        // evenまたはoddのmate数の和を計算する
        unsigned long long sum = 0;
        int next = w % 2;

        string ansstr = "No CP";

        for (unsigned long long state = 0; state < state_size; state++) {
            int next_i = get_table_index(next, state, 0);
            if (table[next_i] != -1) {
                string tmp = "";
                for (int step = 0; step < w * w; step++) {
                    tmp += to_str(table[next_i + step]);
                }
                ansstr = tmp;
                break;
            }
        }
        return ansstr;
    }
};

int main(int argc, char **argv) {
    int w = atoi(argv[1]);
    Counter c(w);

    vector<int> edges;
    for (int i = 2; i < argc; i++) {
        edges.push_back(atoi(argv[i]));
    }

    // PreCreaseの設定
    vector<vector<int>> preEdges(w * w, vector<int>(8, -1));
    for (int i = 0; i < w * w; i++) {
        for (int d = 0; d < 8; d++) {
            preEdges[i][d] = edges[i * 8 + d];
        }
    }

    for (int i = 0; i < w * w; i++) {
        for (int t = 0; t < 36; t++) {
            bool puttable = true;
            for (int d = 0; d < 8; d++) {
                if (preEdges[i][d] == -1)
                    continue;
                if (preEdges[i][d] != TILE[t][d]) {
                    puttable = false;
                    break;
                }
            }
            c.setTileCondition(i, t, puttable);
        }
    }

    cout << c.findCP();

    return 0;
}