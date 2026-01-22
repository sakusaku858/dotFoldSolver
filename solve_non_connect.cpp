#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <map>
#include <array>

#include "boundary_extractor.hpp"
#include "foldsToEdges.h"
#include "ftcp.h"

using namespace std;

#define FRONT 1
#define BACK 0

#define L (-1)
#define S 0
#define R 1
#define X 2

char FOLD_DIR[8] = {S, R, X, R, L, X, R, X};
int FOLD_FLIP[8] = {0, 1, 1, 0, 1, 0, 0, 1};

using Node = int;
using Path = std::vector<Node>;
using FoldAssignment = std::vector<int>;

////////////////////////////////////////////////////////////
// --- 折り割当関係 ---

enum class Shape
{
    RIGHT,
    LEFT,
    STRAIGHT,
};

Shape get_shape(Node prev, Node curr, Node next)
{
    int dx1 = (curr % 9) - (prev % 9);
    int dy1 = (curr / 9) - (prev / 9);
    int dx2 = (next % 9) - (curr % 9);
    int dy2 = (next / 9) - (curr / 9);

    // ベクトル1: prev -> curr
    // ベクトル2: curr -> next
    int cross_product = dx1 * dy2 - dy1 * dx2;

    if (cross_product > 0)
    {
        return Shape::RIGHT;
    }
    else if (cross_product < 0)
    {
        return Shape::LEFT;
    }
    else
    {
        return Shape::STRAIGHT;
    }
}

string shape_to_string(Shape s)
{
    switch (s)
    {
    case Shape::RIGHT:
        return "R";
    case Shape::LEFT:
        return "L";
    case Shape::STRAIGHT:
        return "S";
    default:
        return "X";
    }
}

string path_to_shape_string(const Path &path)
{
    stringstream ss;
    for (int i = 0; i < 32; i++)
    {
        Node prev = path[(i + 31) % 32];
        Node curr = path[i];
        Node next = path[(i + 1) % 32];
        Shape s = get_shape(prev, curr, next);
        ss << shape_to_string(s);
    }
    return ss.str();
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

vector<int> toInt(string s)
{
    map<char, int> ctoi = {{'S', S}, {'R', R}, {'L', L}, {'X', X}};
    vector<int> ans;
    for (char c : s)
        ans.push_back(ctoi.at(c));
    return ans;
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

vector<FoldAssignment> generate_fold_assignments(string dirctory)
{
    // データの読み込み
    cout << "load " << dirctory << endl;
    ifstream ifs(dirctory);
    string line;
    vector<Path> paths;
    while (getline(ifs, line))
    {
        stringstream ss(line);
        Path path;
        int node;
        while (ss >> node)
        {
            path.push_back(node);
        }
        if (!path.empty())
            paths.push_back(path);
    }
    ifs.close();

    // 方向文字列形式に変換
    vector<string> shape_strings;
    for (Path path : paths)
    {
        string shape_string = path_to_shape_string(path);
        shape_strings.push_back(shape_string);
    }

    // 折り割当形式を生成
    vector<vector<int>> all_folds;
    for (string shape_string : shape_strings)
    {
        vector<vector<int>> folds = LRtoNum2(shape_string);
        all_folds.insert(all_folds.end(), folds.begin(), folds.end());
    }

    // 出力テスト
    cout << all_folds.size() << " fold assingments found." << endl;

    return all_folds;
}

////////////////////////////////////////////////////////////////////////////////////

void searchCP(vector<FoldAssignment> fold_assignments, string &cp, string &four_corners)
{
    for (FoldAssignment fold_assignment : fold_assignments)
    {

        // Arrayに変換
        array<int, 32> fold_array;
        std::copy(fold_assignment.begin(), fold_assignment.end(), fold_array.begin());

        // CPの探索
        cp = folds_to_cpstr(fold_array);

        if (cp != "No CP")
        {
            cout << cp << endl;

            // 四隅の割当
            string cornersstr = "";
            for (int i = 0; i < 4; i++)
            {
                int outer = i * 8 + 1;
                int e = get_edge_from_fold(fold_array[outer], 2);
                cornersstr += " " + to_string(e);
            }
            four_corners = cornersstr;

            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////

/// @brief ///////////////////////////////////////////////////////////////
/// @param argc
/// @param argv 64文字からなるドット絵
/// @return //
int main(int argc, char *argv[])
{
    // バリデーション
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <dotstr>" << std::endl;
        return 1;
    }

    // ドット絵の読み込み
    string dotstr = argv[1];
    BoundaryExtractor be;
    be.save_as_edges(dotstr, "edges.txt");

    // 各境界線を通る閉路を計算
    string cmd = ".\\shortest_euler_walk.exe edges.txt solutions.txt";
    system(cmd.c_str());

    // 前処理で枝刈り
    string cmd2 = ".\\path_filter.exe solutions.txt filtered_solutions.txt";
    system(cmd2.c_str());

    // 折り割当の生成
    vector<FoldAssignment> fold_assignments = generate_fold_assignments("filtered_solutions.txt");

    // CPの探索
    string cp = "", four_corners = "";
    searchCP(fold_assignments, cp, four_corners);

    // 結果の出力
    cout << "CPSTR:" << cp << endl;
    cout << "CORNERS:" << four_corners << endl;

    return 0;
}