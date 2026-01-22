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

struct Point
{
    int x;
    int y;
};

struct Edge
{
    Point p1;
    Point p2;
};

// 範囲内に入っているか
bool is_in_range(int min, int x, int max) { return min <= x && x <= max; }

// 範囲内に入っているか
bool is_in_board(int x, int y)
{
    return is_in_range(0, x, 7) && is_in_range(0, y, 7);
}

// 隣接するマスの値を得る
int get_neighbor(int **dots, int index, int dir) {}

// ドット行列から境界線の列を計算する
vector<Edge> dots_to_border_edges(int **dots)
{

    int dirs[4] = {UP, DOWN, LEFT, RIGHT};

    // 各マスについて、上下左右のマスを見る
    for (int i = 0; i < 64; i++)
    {
        if (dots[i] == 0)
            continue;
    }
}

int main(int argc, char **argv)
{

    // 引数の個数チェック
    if (argc != 2)
    {
        cout << "64桁のドット絵を入力してください" << endl;
        exit(1);
    }

    // ドット絵のマスの数チェック
    string dotstr = argv[1];
    if (dotstr.size() == 64)
    {
        cout << "正常な引数が与えられました" << endl;
    }
    else
    {
        cout << "無効な引数が与えられました" << endl;
    }

    // ドット絵を二次元配列に変換
    int dots[8][8];
    for (int i = 0; i < 64; i++)
    {
        int x = i % 8;
        int y = i / 8;
        dots[y][x] = dotstr[i] - '0';
    }

    return 0;
}