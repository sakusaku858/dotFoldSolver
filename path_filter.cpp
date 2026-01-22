#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <array>

using namespace std;

using Node = int;
using Path = std::vector<Node>;

// --- フチの交差判定 ---
bool must_turn_at_intersection(const Path &path)
{
    enum class Direction
    {
        HORIZONTAL,
        VERTICAL,
        NONE
    };

    int len = path.size();

    vector<Direction> directions(81, Direction::NONE);

    for (int i = 0; i < len; i++)
    {
        Node curr = path[i];
        Node next = path[(i + 1) % len];
        Node prev = path[(i + len - 1) % len];

        Direction curr_dir = Direction::NONE;

        if (prev % 9 == curr % 9 && curr % 9 == next % 9)
            curr_dir = Direction::VERTICAL;

        if (prev / 9 == curr / 9 && curr / 9 == next / 9)
            curr_dir = Direction::HORIZONTAL;

        if (directions[curr] == Direction::HORIZONTAL && curr_dir == Direction::VERTICAL)
            return false;

        if (directions[curr] == Direction::VERTICAL && curr_dir == Direction::HORIZONTAL)
            return false;

        directions[curr] = curr_dir;
    }
    return true;
}

// 時計回りで一周するかを判定する
bool has_right_turns_excess_4(const Path &path)
{
    int len = path.size();
    if (len < 3)
        return false; // 閉路として成立しない

    int right_turns = 0;
    int left_turns = 0;

    for (int i = 0; i < len; i++)
    {
        Node prev = path[(i + len - 1) % len];
        Node curr = path[i];
        Node next = path[(i + 1) % len];

        // ベクトル1: prev -> curr
        int dx1 = (curr % 9) - (prev % 9);
        int dy1 = (curr / 9) - (prev / 9);

        // ベクトル2: curr -> next
        int dx2 = (next % 9) - (curr % 9);
        int dy2 = (next / 9) - (curr / 9);

        // 外積の計算
        int cross_product = dx1 * dy2 - dy1 * dx2;

        if (cross_product > 0)
        {
            right_turns++;
        }
        else if (cross_product < 0)
        {
            left_turns++;
        }
        // cross_product == 0 の場合は直進なのでカウントしない
    }

    // 「右折が左折より4回多い」ことを判定
    // (座標系によっては left - right == 4 になる可能性があるため、
    //  厳密に右回りを指定するか、差の絶対値を見るかは用途によります)
    return (right_turns - left_turns == 4);
}

void print_path(const Path &path)
{
    for (Node n : path)
    {
        std::cout << n << " ";
    }
    std::cout << std::endl;
}

// --- 条件判定ロジック（ここを自由に書き換える） ---
bool check_condition(const Path &path)
{
    if (path.empty())
        return false;

    if (!must_turn_at_intersection(path))
    {
        return false;
    }

    if (!has_right_turns_excess_4(path))
    {
        return false;
    }

    return true; // デフォルトではすべて通す
}

// パスの読み込み
std::vector<Path> load_paths(const std::string &filename)
{
    std::vector<Path> paths;
    std::ifstream ifs(filename);
    if (!ifs)
        return paths;

    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty())
            continue;
        Path path;
        std::stringstream ss(line);
        Node n;
        while (ss >> n)
            path.push_back(n);
        paths.push_back(path);
    }
    return paths;
}

// パスの保存
void save_paths(const std::string &filename, const std::vector<Path> &paths)
{
    std::ofstream ofs(filename);
    for (const auto &path : paths)
    {
        for (size_t i = 0; i < path.size(); ++i)
        {
            ofs << path[i] << (i == path.size() - 1 ? "" : " ");
        }
        ofs << "\n";
    }
}

// --- test関数 ---

void run_test(const std::string &test_name, const Path &path, bool expected)
{
    bool result = must_turn_at_intersection(path);
    std::cout << "[" << (result == expected ? "PASS" : "FAIL") << "] "
              << test_name << " (Result: " << (result ? "True" : "False") << ")" << std::endl;
}

void run_test_main()
{
    // --- テストケース 1: 単純な閉路（交差なし） ---
    // 0 -> 1 -> 10 -> 9 -> 0
    Path simple_cycle = {0, 1, 10, 9};
    run_test("Simple cycle (No intersection)", simple_cycle, true);

    // --- テストケース 2: 十字路だが、2回目が「曲がっている」（セーフ） ---
    // 1(上) -> 10(中) -> 19(下) [垂直直進]
    // その後、どこかを回って...
    // 11(右) -> 10(中) -> 1(上) [曲がった！]
    Path figure_eight_turn = {1, 10, 19, 28, 29, 20, 11, 10};
    run_test("Intersection with turn (Safe)", figure_eight_turn, true);

    // --- テストケース 3: 十字路で「直進」（アウト） ---
    // 中心(10)を縦に通り、次に横に来てそのまま直進して突き抜ける
    // 1(上) -> 10(中) -> 19(下) -> ... -> 11(右) -> 10(中) -> 9(左)
    Path cross_straight_fail = {1, 10, 19, 28, 29, 20, 11, 10, 9, 0};
    run_test("Intersection with straight (Fail)", cross_straight_fail, false);

    // --- テストケース 4: T字路での直進（セーフ） ---
    // 自己交差していない場所での直進は当然OK
    Path straight_line = {0, 1, 2, 3, 4};
    run_test("Normal straight (Safe)", straight_line, true);
}

// --- 3. 距離制約判定 (先頭がカド固定) ---

// clang-format off
// 展開図（1辺8、一周32）の座標を計算
void get_origami_coord_int(int index, int &x, int &y) {
    if (index < 8)       { x = index;      y = 0; }         // 下辺
    else if (index < 16) { x = 8;          y = index - 8; } // 右辺
    else if (index < 24) { x = 24 - index; y = 8; }         // 上辺
    else                 { x = 0;          y = 32 - index; }// 左辺
}
//clang-format on

// 距離の2乗を計算するヘルパー
inline int dist_sq(int x1, int y1, int x2, int y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

// 2乗比較による距離制約判定
bool check_distance_constraint(const Path &path) {
    const int N = 32;
    for (int i = 0; i < N; ++i) {
        int x_fold_i = path[i] % 9, y_fold_i = path[i] / 9;
        int x_unfold_i, y_unfold_i;
        get_origami_coord_int(i, x_unfold_i, y_unfold_i);

        for (int j = i + 1; j < N; ++j) {
            // 折った後の距離の2乗
            int x_fold_j = path[j] % 9, y_fold_j = path[j] / 9;
            int d2_folded = dist_sq(x_fold_i, y_fold_i, x_fold_j, y_fold_j);

            // 展開図上の距離の2乗
            int x_unfold_j, y_unfold_j;
            get_origami_coord_int(j, x_unfold_j, y_unfold_j);
            int d2_unfolded = dist_sq(x_unfold_i, y_unfold_i, x_unfold_j, y_unfold_j);

            // 物理的制約: d_folded <= d_unfolded
            // 両辺正なので、2乗のまま比較してOK
            if (d2_folded > d2_unfolded) return false;
        }
    }
    return true;
}

// パスの循環シフト
Path rotate_path(const Path &path, int offset)
{
    int n = path.size();
    Path rotated(n);
    for (int i = 0; i < n; ++i)
        rotated[i] = path[(i + offset) % n];
    return rotated;
}

// --- main関数 ---

int main()
{
    bool test_mode = false;
    if (test_mode)
    {
        run_test_main();
        return 0;
    }

    const std::string input_file = "solutions.txt";
    const std::string output_file = "filtered_solutions.txt";

    std::cout << "Reading " << input_file << "..." << std::endl;
    std::vector<Path> all_paths = load_paths(input_file);

    if (all_paths.empty())
    {
        std::cerr << "No paths found or could not open file." << std::endl;
        return 1;
    }

    vector<Path> filtered;

    // 判定に合格した「回転済みパス」をそのまま一行ずつ書き出す
    for (const auto &original_path : all_paths)
    {
        // 1. 基本形状のフィルタ（十字路、右左折）
        if (!must_turn_at_intersection(original_path))
            continue;
        if (!has_right_turns_excess_4(original_path))
            continue;

        // 2. 8通りの回転について距離制約をチェック
        // TODO : 0 始まりの解しか出力されてない（偶然？）
        for (int offset = 0; offset < 8; ++offset)
        {
            Path shifted_path = rotate_path(original_path, offset);
            
            if (check_distance_constraint(shifted_path))
            {
                filtered.push_back(shifted_path);
                break;
            }
        }
    }

    std::cout << "Results: " << filtered.size() << " / " << all_paths.size() << " paths matched." << std::endl;
    save_paths(output_file, filtered);
    std::cout << "Saved to " << output_file << std::endl;
    
    // ファイル出力
    std::ofstream ofs(output_file);
    if (!ofs)
    {
        std::cerr << "Error: Cannot open file for writing: " << output_file << std::endl;
        return 1;
    }

    for (const auto &path : filtered)
    {
        for (size_t i = 0; i < path.size(); ++i)
        {
            ofs << path[i] << (i == path.size() - 1 ? "" : " ");
        }
        ofs << "\n";
    }

    std::cout << "Successfully saved " << filtered.size() << " solutions to " << output_file << std::endl;

    return 0;
}