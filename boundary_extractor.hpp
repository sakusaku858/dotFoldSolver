#ifndef BOUNDARY_EXTRACTOR_HPP
#define BOUNDARY_EXTRACTOR_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

struct Edge
{
    int u, v;
    // 重複判定などのために比較演算子があると便利
    bool operator==(const Edge &other) const
    {
        return (u == other.u && v == other.v) || (u == other.v && v == other.u);
    }
};

class BoundaryExtractor
{
public:
    static std::vector<Edge> extract(const std::string &bitmap)
    {
        if (bitmap.length() != 64)
        {
            throw std::invalid_argument("Bitmap must be exactly 64 characters.");
        }

        std::vector<Edge> edges;
        auto get_node = [](int x, int y)
        { return y * 9 + x; };

        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                if (bitmap[y * 8 + x] == '1')
                {
                    // 上
                    if (y == 0 || bitmap[(y - 1) * 8 + x] == '0')
                        edges.push_back({get_node(x, y), get_node(x + 1, y)});
                    // 下
                    if (y == 7 || bitmap[(y + 1) * 8 + x] == '0')
                        edges.push_back({get_node(x, y + 1), get_node(x + 1, y + 1)});
                    // 左
                    if (x == 0 || bitmap[y * 8 + (x - 1)] == '0')
                        edges.push_back({get_node(x, y), get_node(x, y + 1)});
                    // 右
                    if (x == 7 || bitmap[y * 8 + (x + 1)] == '0')
                        edges.push_back({get_node(x + 1, y), get_node(x + 1, y + 1)});
                }
            }
        }
        return edges;
    }

    // オイラー閉路が存在するか（次数チェック）の簡易バリデーション
    static bool is_eulerian(const std::vector<Edge> &edges)
    {
        if (edges.empty())
            return false;
        std::vector<int> degree(81, 0);
        for (const auto &e : edges)
        {
            degree[e.u]++;
            degree[e.v]++;
        }
        for (int d : degree)
        {
            if (d % 2 != 0)
                return false; // すべての頂点の次数が偶数である必要がある
        }
        return true;
    }

    // 2. 指定されたファイル名にエッジ形式で保存する機能
    static void save_as_edges(const std::string &bitmap, const std::string &filename)
    {
        std::vector<Edge> edges = extract(bitmap);

        std::ofstream ofs(filename);
        if (!ofs)
        {
            throw std::runtime_error("Could not open file: " + filename);
        }

        for (const auto &e : edges)
        {
            ofs << e.u << " " << e.v << "\n";
        }
    }
};

#endif