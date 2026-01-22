#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define S 0
#define R 1
#define L -1
#define X 2

#define FRONT 1
#define BACK 0

char FOLD_DIR[8] = {S, R, X, R, L, X, R, X};
int FOLD_FLIP[8] = {0, 1, 1, 0, 1, 0, 0, 1};

vector<int> toInt(string s) {
    map<char, int> ctoi = {{'S', S}, {'R', R}, {'L', L}, {'X', X}};
    vector<int> ans;
    for (char c : s)
        ans.push_back(ctoi.at(c));
    return ans;
}

// コーナーの隣の2頂点から出る折り線が接続していなくてはいけない
bool corner_neighbor_condition(const vector<int> &v) {
    if (v.size() < 4)
        return true;

    string NG_WORDS[12] = {"084", "086", "484", "486", "684", "686",
                           "180", "181", "183", "380", "381", "383"};

    // 末尾を先頭に持ってきた折り番号リストを得る
    ostringstream oss;
    for (int i : v)
        oss << i;

    string s = oss.str();
    s.pop_back();
    if (s.length() == 32) {
        s = s + s;
    }

    // NGワードを含んでいたらfalse
    for (int i = 0; i < 12; i++) {
        if (s.find(NG_WORDS[i]) != string::npos)
            return false;
    }

    return true;
}

// 垂直および水平の外周辺の本数は偶数でなければいけない
bool vh_edge_condition(const vector<int> &v) {
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if (v[i] == 3 || v[i] == 6)
            count++;
    }

    return count % 2 == 0;
}

bool equate_folds_assignment(const vector<int> &v) { //
    if (v.size() < 2)
        return true;

    string NG_WORDS[2] = {"34", "36"};

    // 配列を文字列に変換
    ostringstream oss;
    for (int i : v)
        oss << i;

    string s = oss.str();
    s.pop_back();

    if (s.length() == 32) {
        s = s + s;
    }

    // NGワードを含んでいたらfalse
    for (int i = 0; i < 2; i++) {
        if (s.find(NG_WORDS[i]) != string::npos)
            return false;
    }

    return true;
}

// 偶頂点から出る斜め辺の本数
// 奇頂点から出る斜め辺の本数
// ともに偶数本でなければならない
bool diag_parity_condition(const vector<int> &v) {
    int odd_count = 0;
    int even_count = 0;

    for (int i = 0; i < 32; i++) {
        int increment = 0;
        if (v[i] == 1 || v[i] == 3 || v[i] == 4 || v[i] == 6)
            increment = 1;

        if (i % 2 == 0)
            even_count += increment;
        else
            odd_count += increment;
    }

    return even_count % 2 == 0 && odd_count % 2 == 0;
}

// 右左折形式から折割当形式に変換する関数
// このプログラムの最重要な処理
vector<vector<int>> LRtoNum(string input_str) {
    // LRSからなる入力文字列を-1,0,1の列に変換
    vector<int> input = toInt(input_str);

    vector<int> root;
    root.push_back(8);     // カドは8という折り方
    root.push_back(FRONT); // 表裏で割り当てる折り番号が逆になるから必要

    queue<vector<int>> que;
    que.push(root);

    vector<vector<int>> answers;

    while (!que.empty()) {
        vector<int> p = que.front(); // pには32頂点の折り番号が格納される
        que.pop();

        // 同一視できる割当があるなら
        if (equate_folds_assignment(p) == false) {
            // continue;
        }

        if (p.size() == 33 && p.back() == FRONT) {
            // 1. コーナー近傍条件
            // 2. 垂直辺条件
            // 3. 斜め辺パリティ条件
            bool condition = corner_neighbor_condition(p) &&
                             vh_edge_condition(p) && diag_parity_condition(p);

            if (condition)
                answers.push_back(p);
            continue;
        }

        if (p.size() == 33) {
            continue;
        }

        int side = p.back(); // 表向きか裏向きか
        int i = p.size() - 1;
        int input_dir = input[i];

        if (i % 8 == 0 && input_dir == S)
            continue;

        if (i % 8 == 0 && input_dir == R && side == BACK)
            continue;

        if (i % 8 == 0 && input_dir == L && side == FRONT)
            continue;

        if (i % 8 == 0) {
            p[i] = 8;
            p.push_back(side);
            que.push(p);
            continue;
        }

        if (side == BACK)
            input_dir = input_dir * (-1);

        for (int j = 0; j < 8; j++) {
            if (FOLD_DIR[j] == input_dir) {
                vector<int> q = p;
                q[i] = j;
                int side2 = (side + FOLD_FLIP[j]) % 2;
                q.push_back(side2);
                que.push(q);
            }
        }
    }

    return answers;
}

string rotate_string(string a, int n) {
    string prefix = a.substr(0, n);
    string suffix = a.substr(n);
    return suffix + prefix;
}

int main(void) {
    string str1 = "SLRLLLRRSLRLLLRRSLRLLLRRSLRLLLRR";
    string str2 = "RLRRLLRRRLRLLLRRRLRLSRRLRLSRSRSL";
    string input_str = str1;

    // 複数の折り割当が格納される
    vector<vector<int>> answers;

    // 8通りのズラシを考慮し、右左折形式から折番号形式に変換
    for (int i = 0; i < 8; i++) {
        vector<vector<int>> tmp_ans;
        string str = rotate_string(input_str, i);
        tmp_ans = LRtoNum(str);
        answers.insert(answers.end(), tmp_ans.begin(), tmp_ans.end());
    }

    cout << "num of fold assignments" << endl;
    cout << answers.size() << endl;

// 答えを10個表示
#if 1
    for (int i = 0; i < 10; i++) {
        if (i >= answers.size())
            break;
        vector<int> ans = answers[i];
        for (int fold : ans) {
            cout << fold << " ";
        }
        cout << endl;
    }
#endif

    // テキストファイルに出力
    ofstream outfile("data.txt");

    if (outfile.is_open()) {
        for (auto ans : answers) {
            for (int i = 0; i < 32; i++) {
                outfile << ans[i] << " ";
            }
            outfile << endl;
        }
        outfile.close();
    }

    return 0;
}