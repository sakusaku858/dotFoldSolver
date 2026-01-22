#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "boundary_extractor.hpp"

using namespace std;

using Node = int;
using Path = std::vector<Node>;

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

    // CPの探索

    // 結果の出力

    return 0;
}