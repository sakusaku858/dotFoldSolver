#include "loopToFolds.h"

#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <stack>

using namespace std;

#define FRONT 1
#define BACK 0

char FOLD_DIR[8] = {S, R, X, R, L, X, R, X};
int FOLD_FLIP[8] = {0, 1, 1, 0, 1, 0, 0, 1};

// 折り割り当て生成時の計算過程もちいる折り状態
class FoldState
{
public:
    array<int, 32> folds;
    int length;
    int side;

    FoldState()
    {
        length = 0;
        side = FRONT;
        folds.fill(0);
    }

    // 裏返す
    void reverse() { side = (side + 1) % 2; }

    // 折り目を追加する
    void push(int fold)
    {
        if (length >= 32)
            return;

        folds[length] = fold;
        length++;

        if (fold == 1 || fold == 4)
            this->reverse();
    }

    // 直前の折り目との組み合わせでNGなパターンがないかチェック
    // "34" は "16" と同一視できるため除外
    // "36" は "14" と同一視できるため除外
    bool is_valid_sequence() const
    {
        if (length < 2)
            return true;
        int cur = folds[length - 1];
        int prev = folds[length - 2];
        if (prev == 3 && (cur == 4 || cur == 6))
            return false;
        return true;
    }

    // コーナーのNG判定
    bool is_ng_corner(int left, int right)
    {
        if (left == 0 || left == 4 || left == 6)
        {
            if (right == 4 || right == 6)
                return true;
        }
        if (left == 1 || left == 3)
        {
            if (right == 0 || right == 1 || right == 3)
                return true;
        }
        return false;
    }
};

vector<int> toInt(string s)
{
    map<char, int> ctoi = {{'S', S}, {'R', R}, {'L', L}, {'X', X}};
    vector<int> ans;
    for (char c : s)
        ans.push_back(ctoi.at(c));
    return ans;
}

bool is_ng_corner(const int left, const int right)
{
    if (left == 0 || left == 4 || left == 6)
    {
        if (right == 4 || right == 6)
        {
            return true;
        }
    }
    if (left == 1 || left == 3)
    {
        if (right == 0 || right == 1 || right == 3)
        {
            return true;
        }
    }
    return false;
}

// コーナーの隣の2頂点から出る折り線が接続していなくてはいけない
bool corner_neighbor_condition(const vector<int> &v)
{
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
    if (s.length() == 32)
    {
        s = s + s;
    }

    // NGワードを含んでいたらfalse
    for (int i = 0; i < 12; i++)
    {
        if (s.find(NG_WORDS[i]) != string::npos)
            return false;
    }

    return true;
}

// 垂直および水平の外周辺の本数は偶数でなければいけない
bool vh_edge_condition(const vector<int> &v)
{
    int count = 0;
    for (int i = 0; i < 32; i++)
    {
        if (v[i] == 3 || v[i] == 6)
            count++;
    }

    return count % 2 == 0;
}

// (3,4)と(1,6)は同値
// (3,6)と(1,4)は同値
bool has_34_36_assignment(const vector<int> &v)
{
    if (v.size() < 2)
        return true;
    for (int i = 0; i < v.size() - 1; i++)
    {
        if (v[i] == 3 && v[i + 1] == 4)
            return true;
        if (v[i] == 3 && v[i + 1] == 6)
            return true;
    }
    return false;
}

// 偶頂点から出る斜め辺の本数
// 奇頂点から出る斜め辺の本数
// ともに偶数本でなければならない
bool diag_parity_condition(const vector<int> &v)
{
    int odd_count = 0;
    int even_count = 0;

    for (int i = 0; i < 32; i++)
    {
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

vector<vector<int>> NG_LIST = {
    {0, 8, 4},
    {0, 8, 6},
    {4, 8, 4},
    {4, 8, 6},
    {6, 8, 4},
    {6, 8, 6},
    {1, 8, 0},
    {1, 8, 1},
    {1, 8, 3},
    {3, 8, 0},
    {3, 8, 1},
    {3, 8, 3},
    {3, 4},
    {3, 6}};

bool has_NG_words(vector<int> v)
{
    if (v.size() == 32)
    {
        v.push_back(v[0]);
    }

    for (auto ng : NG_LIST)
    {
        if (v.size() < ng.size())
            continue;

        bool flag = true;
        for (int i = 0; i < ng.size(); i++)
        {
            int n = ng[ng.size() - 1 - i];
            int m = v[v.size() - 1 - i];
            if (m != n)
            {
                flag = false;
                break;
            }
        }
        if (flag)
            return true;
    }

    return false;
}

// LRS形式から折り割当に変換する関数
vector<vector<int>> LRtoNum2(string input_str)
{

    // 8の倍数番目の文字がSなら不適
    if (input_str[0] == 'S' || input_str[8] == 'S' || input_str[16] == 'S' || input_str[24] == 'S')
        return vector<vector<int>>();

    // LRSからなる入力文字列を-1,0,1の列に変換
    vector<int> lrint = toInt(input_str);

    // ここでは配列のインデックスとして使いたいため1を加算
    for (int i = 0; i < lrint.size(); i++)
    {
        lrint[i] += 1;
    }

    // 紙の表裏を決定
    int side = (lrint[0] == R + 1) ? FRONT : BACK;

    // 向きと折り番号の対応
    vector<int> fold_table[2][3];
    fold_table[FRONT][S + 1] = {0};
    fold_table[FRONT][L + 1] = {4};
    fold_table[FRONT][R + 1] = {1, 3, 6};
    fold_table[BACK][S + 1] = {0};
    fold_table[BACK][R + 1] = {4};
    fold_table[BACK][L + 1] = {1, 3, 6};

    // 表裏が入れ替わるかどうか
    map<int, int> flip_map;
    flip_map[0] = 0; // 折り方0番は裏返らない
    flip_map[1] = 1;
    flip_map[3] = 0;
    flip_map[4] = 1;
    flip_map[6] = 0;
    flip_map[8] = 0;

    // --- NGリスト ---
    // "084", "086", "484", "486", "684", "686",
    // "180", "181", "183", "380", "381", "383" : 折れない
    // "34", "36" : 折れるが同値な折り方が存在する

    // --- 探索処理 ---

    vector<vector<int>> answers;

    struct Node
    {
        int side;
        vector<int> folds;
    };

    stack<Node> stk;
    stk.push({side, {}});

    while (!stk.empty())
    {
        Node node = stk.top();
        stk.pop();

        // 終了処理
        if (node.folds.size() == 32)
        {
            if (has_NG_words(node.folds))
                continue;

            answers.push_back(node.folds);
            continue;
        }

        // 8の倍数の処理
        if (node.folds.size() % 8 == 0)
        {
            int lr = lrint[node.folds.size()];
            if (node.side == BACK && lr == R + 1)
                continue;
            if (node.side == FRONT && lr == L + 1)
                continue;

            node.folds.push_back(8);
            stk.push(node);
            continue;
        }

        // 一般の処理

        for (auto f : fold_table[node.side][lrint[node.folds.size()]])
        {
            vector<int> new_folds = node.folds;
            new_folds.push_back(f);

            if (has_NG_words(new_folds))
                continue;

            stk.push({(node.side + flip_map[f]) % 2, new_folds});
        }
    }

    return answers;
}

// 右左折形式から折割当形式に変換する関数
// このプログラムの最重要な処理
// input_strは8の倍数文字目がSでないことが保証されている
// input_strは偶数頂点から出る斜め線の本数と
// 奇数頂点から出る斜め線の本数が共に偶数本であることが保証されている
vector<vector<int>> LRtoNum(string input_str)
{
    // LRSからなる入力文字列を-1,0,1の列に変換
    vector<int> input = toInt(input_str);

    vector<int> root;
    root.push_back(8);     // カドは8という折り方
    root.push_back(FRONT); // 表裏で割り当てる折り番号が逆になるから必要

    queue<vector<int>> que;
    que.push(root);

    vector<vector<int>> answers;

    while (!que.empty())
    {
        vector<int> p = que.front(); // pには32頂点の折り番号が格納される
        que.pop();

        // 同一視できる割当があるなら
        if (has_34_36_assignment(p))
            continue;

        // コーナー条件
        if (p.size() == 11 || p.size() == 19 || p.size() == 27)
        {
            int right = p[p.size() - 2];
            int left = p[p.size() - 4];
            if (is_ng_corner(left, right))
            {
                continue;
            }
        }

        if (p.size() == 33 && p.back() == FRONT)
        {
            bool condition = corner_neighbor_condition(p);

            // 31-0-1 頂点の接続が正しいか
            if (p[31] % 2 != p[1] / 4)
                continue;

            if (condition)
                answers.push_back(p);
            continue;
        }

        if (p.size() == 33)
        {
            continue;
        }

        int side = p.back();      // 表向きか裏向きか
        int i = p.size() - 1;     // 直前の頂点番号
        int input_dir = input[i]; // 直前の頂点の折れ曲がり方向

        if (i % 8 == 0 && input_dir == R && side == BACK)
            continue;

        if (i % 8 == 0 && input_dir == L && side == FRONT)
            continue;

        if (i % 8 == 0)
        {
            p[i] = 8;
            p.push_back(side);
            que.push(p);
            continue;
        }

        if (side == BACK)
            input_dir = input_dir * (-1); // 表から見たときの折れ曲がり方向

        // これ何やってるの？
        for (int j = 0; j < 8; j++)
        {
            if (FOLD_DIR[j] == input_dir)
            {
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

string rotate_string(string a, int n)
{
    string prefix = a.substr(0, n);
    string suffix = a.substr(n);
    return suffix + prefix;
}

vector<vector<int>> createAllFolds(string input_str)
{
    // return LRtoNum(input_str);
    vector<vector<int>> all_folds = LRtoNum2(input_str);
    cout << all_folds.size() << " folds found." << endl;
    return all_folds;
}

#if 0
int main(void)
{
    string str1 = "SLRLLLRRSLRLLLRRSLRLLLRRSLRLLLRR";
    string str2 = "RLRRLLRRRLRLLLRRRLRLSRRLRLSRSRSL";
    string input_str = str1;

    // 複数の折り割当が格納される
    vector<vector<int>> answers = createAllFolds(input_str);

    cout << "num of fold assignments" << endl;
    cout << answers.size() << endl;

    // 答えを10個表示
    for (int i = 0; i < 10; i++)
    {
        if (i >= answers.size())
            break;
        vector<int> ans = answers[i];
        for (int fold : ans)
        {
            cout << fold << " ";
        }
        cout << endl;
    }

    // テキストファイルに出力
    ofstream outfile("data.txt");

    if (outfile.is_open())
    {
        for (auto ans : answers)
        {
            for (int i = 0; i < 32; i++)
            {
                outfile << ans[i] << " ";
            }
            outfile << endl;
        }
        outfile.close();
    }

    return 0;
}
#endif