// (1) 動的計画法を文字列じゃなく配列でやったほうが速いのか
// (2) 動的計画法を表じゃなく、集合でやったほうが速いのか
// (3) 動的計画法を使わず、全探索するほうが速いのか
// 各手法に対し、計算時間を計測する

// 入力として、ランダムに生成した折り割当の探索にかかった時間を計測する
// すべての内部頂点の状態を入力とする
// 1：折る
// 0：折らない
// -1：未定
// 初めに7を入れる（問題サイズ）
// 入力例 ： 7 1 1 0 0 -1 -1 ....

#include <chrono>
#include <format>
#include <iostream>
#include <omp.h>
#include <random>
#include <stack>
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

// ランダムな折り割当を生成する
vector<int> create_random_folds() {
    vector<int> folds;
    vector<int> rndf = {0, 1, 3, 4, 6};

    // 乱数生成用
    unsigned int seed = 42;
    mt19937 engine(seed);
    uniform_int_distribution<> dist(0, 4);

    for (int i = 0; i < 32; i++) {
        if (i % 8 == 0) {
            folds.push_back(8);
        } else {
            int findx = dist(engine);
            int f = rndf[findx];
            folds.push_back(f);
        }
    }

    return folds;
}

// vectorを印字する
void print_vector(vector<int> v) {
    for (const int &n : v) {
        cout << n << " ";
    }
    cout << endl;
}

// p1から見たp2の方向を返す
int get_dir(int x1, int y1, int x2, int y2) {
    int x = x2 - x1;
    int y = y2 - y1;
    if (x == 0 && y < 0)
        return UP;
    if (x > 0 && y < 0)
        return UPPER_RIGHT;
    if (x > 0 && y == 0)
        return RIGHT;
    if (x > 0 && y > 0)
        return LOWER_RIGHT;
    if (x == 0 && y > 0)
        return DOWN;
    if (x < 0 && y > 0)
        return LOWER_LEFT;
    if (x < 0 && y == 0)
        return LEFT;
    return UPPER_LEFT;
}

// 折り割当から各内部頂点の状態を得る
// -1: 未定
//  0: 折らない
//  1: 折る
vector<int> get_vertex_state(vector<int> folds) {
    vector<int> edges = vector<int>(49 * 8, -1);

    for (int i = 0; i < folds.size(); i++) {
        // 折り番号
        int f = folds[i];

        // 各辺の値
        int e1 = f % 2;
        int e2 = f / 2 % 2;
        int e3 = f / 4 % 2;

        // 座標を得る
        int area = i / 8;
        int mod = i % 8;
        int upper = (area == 0);
        int right = (area == 1);
        int lower = (area == 2);
        int left = (area == 3);
        int x = upper * mod + right * 8 + lower * (8 - mod);
        int y = right * mod + lower * 8 + left * (8 - mod);

        // 隣接するマスの座標を得る
        // p1は先頭、p2は中心、p3は後方
        int x1 = x + upper - lower - right + left;
        int y1 = y + upper - lower + right - left;
        int x2 = x - right + left;
        int y2 = y + upper - lower;
        int x3 = x - upper + lower - right + left;
        int y3 = y + upper - lower - right + left;

        cout << std::format("({},{})", x, y) << endl;
        cout << std::format("({},{}), ({},{}), ({},{})", x1, y1, x2, y2, x3, y3)
             << endl;
        cout << endl;

        // 辺の方向
        int d1 = get_dir(x1, y1, x, y);
        int d2 = get_dir(x2, y2, x, y);
        int d3 = get_dir(x3, y3, x, y);

        // マス目のインデックス
        int m1 = (x1 - 1) + (y1 - 1) * 7;
        int m2 = (x2 - 1) + (y2 - 1) * 7;
        int m3 = (x3 - 1) + (y3 - 1) * 7;

        vector<int> xs = {x1, x2, x3};
        vector<int> ys = {y1, y2, y3};
        vector<int> es = {e1, e2, e3};
        vector<int> dirs = {d1, d2, d3};
        vector<int> ms = {m1, m2, m3};

        // 3つの辺を更新
        for (int j = 0; j < 3; j++) {
            int px = xs[j];
            int py = ys[j];
            bool out = px < 1 || px > 7 || py < 1 || py > 7;
            if (out)
                continue;
            edges[ms[j] * 8 + dirs[j]] = es[j];
        }
    }
    return edges;
}

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

        int count_same_state = 0;

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
                    if (table[next][newstate] != "NG") {
                        count_same_state++;
                        continue;
                    }

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

        cout << "count same state : " << count_same_state << endl;

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

    string findCPint() {

        int count_same_state = 0;

        vector<int8_t> NG;
        // tableの準備
        vector<vector<vector<int8_t>>> table(
            2, vector<vector<int8_t>>(state_size, NG));

        vector<int8_t> zero;
        table[0][0] = zero;

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
                table[next][k] = NG;

// 各mateの更新
#pragma omp parallel for schedule(guided)
            for (unsigned long long k = 0; k < state_size; k++) {
                if (table[prev][k] == NG)
                    continue;

                for (int j = 0; j < 36; j++) { // タイルjを設置

                    // セルiにタイルjが置けるか？
                    if (!can_put(i, j, k))
                        continue;

                    // セルiにタイルjを設置したときのmateを得る
                    unsigned long long newstate = put(i, j, k);

                    // すでに経路があるなら書き込まない
                    if (table[next][newstate] != NG) {
                        count_same_state++;
                        continue;
                    }

                    // 書き込み先のロックを取得
                    omp_set_lock(&locks[newstate]);

                    // 得られたmateへ到達する展開図を書き込む
                    vector<int8_t> new_vec = table[prev][k];
                    new_vec.push_back(i);
                    table[next][newstate] = new_vec;

                    // ロックを開放
                    omp_unset_lock(&locks[newstate]);
                }
            }
        }

        // ロックの解放
        for (unsigned long long i = 0; i < state_size; i++) {
            omp_destroy_lock(&locks[i]);
        }

        cout << "count same state : " << count_same_state << endl;

        // evenまたはoddのmate数の和を計算する
        unsigned long long sum = 0;
        int next = w % 2;

        for (unsigned long long state = 0; state < state_size; state++) {
            if (table[next][state] != NG) {
                vector<int8_t> cp = table[next][state];
                string ans = "";
                for (int i = 0; i < w * w; i++) {
                    string head = "";
                    if (cp[i] < 10) {
                        head = "0";
                    }
                    ans += head + to_str(cp[i]);
                }
                return ans;
            }
        }
        return "No CP";
    }
};

string naive_alg(vector<int> vstate) {
    vector<int8_t> cp = vector<int8_t>(49, -1);
    stack<pair<int, int>> stack;

    for (int t = 0; t < 36; t++) {
        stack.push(pair<int, int>(0, t));
    }

    while (!stack.empty()) {
        pair<int, int> p = stack.top();
        stack.pop();

        int m = p.first;  // マス番号
        int t = p.second; // タイル番号

        // おけるか判定
        bool can_put = true;
        for (int d = 0; d < 8; d++) {
            int e = TILE[t][d];
            int s = vstate[m * 8 + d];
            if (s != -1 && s != e) {
                can_put = false;
                break;
            }
        }

        if (can_put) {
            cp[m] = t;

            if (m == 48) {
                return "find";
            }

            for (int u = 0; u < 36; u++) {
                stack.push(pair<int, int>(m + 1, u));
            }
        }
    }
    return "No CP";
}

int main(void) {

    vector<int> folds = create_random_folds();
    print_vector(folds);

    // 8*49個の要素
    vector<int> edges = get_vertex_state(folds);
    print_vector(edges);

    int w = 7;
    Counter c(w);

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

    auto start = std::chrono::high_resolution_clock::now();

    // string foundCP = c.findCPint();
    // cout << foundCP << endl;
    naive_alg(edges);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "処理が完了しました。" << std::endl;
    std::cout << "実行時間: " << duration.count() << " ミリ秒 (ms)"
              << std::endl;

    return 0;
}