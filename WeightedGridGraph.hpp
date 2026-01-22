#ifndef WEIGHTED_GRID_GRAPH_HPP
#define WEIGHTED_GRID_GRAPH_HPP

#include "WeightedGraph.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <tuple>
#include <vector>
#include <string>

class WeightedGridGraph : public WeightedGraph
{
private:
    int width;
    int height;

public:
    WeightedGridGraph(int w, int h)
        : WeightedGraph(w * h), width(w), height(h) {}

    WeightedGridGraph(std::string dotstr)
        : WeightedGraph(81), width(9), height(9)
    {
        int rows = 8;
        int cols = 8;
        std::vector<std::vector<int>> dotArt(rows, std::vector<int>(cols, 0));
        for (int i = 0; i < rows * cols; ++i)
        {
            dotArt[i / cols][i % cols] = dotstr[i] - '0';
        }
        this->initializeFromDotArt(dotArt);
    }

    // 座標 (x, y) から頂点インデックスを取得
    int getIndex(int x, int y) const
    {
        if (x < 0 || x >= width || y < 0 || y >= height)
            return -1;
        return y * width + x;
    }

    // グリッド上の2点間に辺を追加
    void addEdge(int x1, int y1, int x2, int y2, int weight)
    {
        int u = getIndex(x1, y1);
        int v = getIndex(x2, y2);
        if (u != -1 && v != -1)
        {
            WeightedGraph::addEdge(u, v, weight);
        }
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // グリッド全体に、指定した重みで格子状の辺を自動的に追加する
    void initializeGridEdges(int weight)
    {
        if (weight == 0)
            return; // 重み0の辺は追加しない

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                // 右方向の辺 (逆方向は addEdge 内で自動的に追加される)
                if (x + 1 < width)
                {
                    addEdge(x, y, x + 1, y, weight);
                }
                // 下方向の辺 (逆方向は addEdge 内で自動的に追加される)
                if (y + 1 < height)
                {
                    addEdge(x, y, x, y + 1, weight);
                }
            }
        }
    }

    // ドット絵（0/1の二次元配列）から境界線を抽出し、重み1の辺として追加する
    // dotArt: rows x cols の二次元配列 (0:白, 1:黒)
    // 前提: このグラフは (cols + 1) x (rows + 1) のサイズで初期化されていること
    void initializeFromDotArt(const std::vector<std::vector<int>> &dotArt)
    {
        if (dotArt.empty())
            return;
        int rows = dotArt.size();
        int cols = dotArt[0].size();

        if (width != cols + 1 || height != rows + 1)
        {
            std::cerr
                << "Error: Graph dimensions do not match dot art dimensions."
                << std::endl;
            return;
        }

        auto getColor = [&](int r, int c)
        {
            if (r < 0 || r >= rows || c < 0 || c >= cols)
                return 0;
            return dotArt[r][c];
        };

        // 横方向の境界線 (y=r のライン上, x=c から x=c+1)
        for (int r = 0; r <= rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                int colorUp = getColor(r - 1, c);
                int colorDown = getColor(r, c);
                if (colorUp != colorDown)
                {
                    addEdge(c, r, c + 1, r, 1);
                }
            }
        }

        // 縦方向の境界線 (x=c のライン上, y=r から y=r+1)
        for (int c = 0; c <= cols; ++c)
        {
            for (int r = 0; r < rows; ++r)
            {
                int colorLeft = getColor(r, c - 1);
                int colorRight = getColor(r, c);
                if (colorLeft != colorRight)
                {
                    addEdge(c, r, c, r + 1, 1);
                }
            }
        }
    }

    // 重みが0以外の辺で構成される連結成分を繋ぐために必要な最小の追加辺数（マンハッタン距離ベースのMST）を計算する
    int getMinEdgesToConnect() const
    {
        int V = getVertexCount();
        std::vector<int> component(V, -1); // 各頂点が属する連結成分のID
        std::vector<std::vector<int>>
            components;                      // 各連結成分に含まれる頂点のリスト
        std::vector<bool> visited(V, false); // 訪問済みフラグ

        // 1. 重み正の辺を持つ頂点を抽出し、連結成分に分解する
        for (int i = 0; i < V; ++i)
        {
            if (visited[i])
                continue;

            // 頂点 i が重み正の辺を持っているか確認
            bool hasPositiveEdge = false;
            for (const auto &e : getNeighbors(i))
            {
                if (e.weight > 0)
                {
                    hasPositiveEdge = true;
                    break;
                }
            }

            if (!hasPositiveEdge)
                continue;

            // 新しい成分が見つかった
            std::vector<int> compNodes; // 現在の連結成分に属する頂点群
            std::vector<int> q;         // BFS用のキュー
            q.push_back(i);
            visited[i] = true;
            component[i] = components.size();
            compNodes.push_back(i);

            int head = 0;
            while (head < (int)q.size())
            {
                int u = q[head++];
                for (const auto &e : getNeighbors(u))
                {
                    if (e.weight > 0)
                    {
                        if (!visited[e.to])
                        {
                            visited[e.to] = true;
                            component[e.to] = components.size();
                            compNodes.push_back(e.to);
                            q.push_back(e.to);
                        }
                    }
                }
            }
            components.push_back(compNodes);
        }

        int k = components.size(); // 連結成分の数
        if (k <= 1)
            return 0;

        // 2. 成分間の最小距離（マンハッタン距離）を計算する
        // 成分間を結ぶ辺（メタ辺）の構造体
        struct CompEdge
        {
            int u, v, weight; // u, v: 連結成分ID, weight:
                              // 成分間の最短マンハッタン距離
            bool operator<(const CompEdge &other) const
            {
                return weight < other.weight;
            }
        };
        std::vector<CompEdge> compEdges; // 全成分間のペアに対する距離リスト

        for (int i = 0; i < k; ++i)
        {
            for (int j = i + 1; j < k; ++j)
            {
                int minDist = 1000000000;
                // 成分 i と 成分 j の間の最短距離を総当たりで計算
                for (int u : components[i])
                {
                    int ux = u % width;
                    int uy = u / width;
                    for (int v : components[j])
                    {
                        int vx = v % width;
                        int vy = v / width;
                        int d = std::abs(ux - vx) + std::abs(uy - vy);
                        if (d < minDist)
                            minDist = d;
                    }
                }
                compEdges.push_back({i, j, minDist});
            }
        }

        // 3. クラスカル法でMSTの重みを計算する
        std::sort(compEdges.begin(), compEdges.end());

        std::vector<int> parent(k); // Union-Find用の親配列
        for (int i = 0; i < k; ++i)
            parent[i] = i;

        // Union-Findのfind操作
        auto find = [&](int x)
        {
            while (x != parent[x])
                x = parent[x] = parent[parent[x]];
            return x;
        };

        int totalEdgesNeeded = 0; // 連結に必要な総コスト（追加辺数）
        for (const auto &e : compEdges)
        {
            int rootU = find(e.u);
            int rootV = find(e.v);
            if (rootU != rootV)
            {
                parent[rootU] = rootV;
                totalEdgesNeeded += e.weight;
            }
        }

        return totalEdgesNeeded;
    }

    // 連結にするために追加すべき辺のリストを取得する
    // 戻り値: 追加すべき辺の座標リスト [(x1, y1, x2, y2), ...]
    std::vector<std::tuple<int, int, int, int>> getEdgesToConnect() const
    {
        int V = getVertexCount();
        std::vector<int> component(V, -1);
        std::vector<std::vector<int>> components;
        std::vector<bool> visited(V, false);

        // 1. 重み正の辺を持つ頂点を抽出し、連結成分に分解する
        for (int i = 0; i < V; ++i)
        {
            if (visited[i])
                continue;
            bool hasPositiveEdge = false;
            for (const auto &e : getNeighbors(i))
            {
                if (e.weight > 0)
                {
                    hasPositiveEdge = true;
                    break;
                }
            }
            if (!hasPositiveEdge)
                continue;

            std::vector<int> compNodes;
            std::vector<int> q;
            q.push_back(i);
            visited[i] = true;
            component[i] = components.size();
            compNodes.push_back(i);

            int head = 0;
            while (head < (int)q.size())
            {
                int u = q[head++];
                for (const auto &e : getNeighbors(u))
                {
                    if (e.weight > 0 && !visited[e.to])
                    {
                        visited[e.to] = true;
                        component[e.to] = components.size();
                        compNodes.push_back(e.to);
                        q.push_back(e.to);
                    }
                }
            }
            components.push_back(compNodes);
        }

        std::vector<std::tuple<int, int, int, int>> addedEdges;
        int k = components.size();
        if (k <= 1)
            return addedEdges;

        // 2. 成分間の最小距離（マンハッタン距離）を計算する
        struct CompEdge
        {
            int u, v;         // Component IDs
            int nodeU, nodeV; // Actual node IDs
            int weight;
            bool operator<(const CompEdge &other) const
            {
                return weight < other.weight;
            }
        };
        std::vector<CompEdge> compEdges;

        for (int i = 0; i < k; ++i)
        {
            for (int j = i + 1; j < k; ++j)
            {
                int minDist = 1000000000;
                int bestU = -1, bestV = -1;
                for (int u : components[i])
                {
                    int ux = u % width;
                    int uy = u / width;
                    for (int v : components[j])
                    {
                        int vx = v % width;
                        int vy = v / width;
                        int d = std::abs(ux - vx) + std::abs(uy - vy);
                        if (d < minDist)
                        {
                            minDist = d;
                            bestU = u;
                            bestV = v;
                        }
                    }
                }
                compEdges.push_back({i, j, bestU, bestV, minDist});
            }
        }

        // 3. クラスカル法でMSTを構築し、経路を復元する
        std::sort(compEdges.begin(), compEdges.end());
        std::vector<int> parent(k);
        for (int i = 0; i < k; ++i)
            parent[i] = i;
        auto find = [&](int x)
        {
            while (x != parent[x])
                x = parent[x] = parent[parent[x]];
            return x;
        };

        for (const auto &e : compEdges)
        {
            int rootU = find(e.u);
            int rootV = find(e.v);
            if (rootU != rootV)
            {
                parent[rootU] = rootV;

                // 経路復元 (L字型経路)
                int curr = e.nodeU;
                int target = e.nodeV;
                int cx = curr % width;
                int cy = curr / width;
                int tx = target % width;
                int ty = target / width;

                while (cx != tx || cy != ty)
                {
                    int nx = cx;
                    int ny = cy;
                    // X方向に移動
                    if (cx < tx)
                        nx++;
                    else if (cx > tx)
                        nx--;
                    // Y方向に移動 (Xが一致したら)
                    else if (cy < ty)
                        ny++;
                    else if (cy > ty)
                        ny--;

                    addedEdges.emplace_back(cx, cy, nx, ny);
                    cx = nx;
                    cy = ny;
                }
            }
        }
        return addedEdges;
    }

    // パス（頂点列）を移動方向（直進、右折、左折、Uターン）の列に変換する
    std::vector<std::string>
    convertPathToTurns(const std::vector<int> &path) const
    {
        std::vector<std::string> turns;
        if (path.size() < 3)
            return turns;

        auto getTurn = [&](int u, int curr, int v) -> std::string
        {
            int ux = u % width;
            int uy = u / width;
            int cx = curr % width;
            int cy = curr / width;
            int vx = v % width;
            int vy = v / width;

            int dx1 = cx - ux;
            int dy1 = cy - uy;
            int dx2 = vx - cx;
            int dy2 = vy - cy;

            int cp = dx1 * dy2 - dy1 * dx2;
            int dp = dx1 * dx2 + dy1 * dy2;

            if (cp > 0)
                return "Right";
            if (cp < 0)
                return "Left";
            if (dp < 0)
                return "U-Turn";
            return "Straight";
        };

        // 中間の頂点でのターン
        for (size_t i = 1; i < path.size() - 1; ++i)
        {
            turns.push_back(getTurn(path[i - 1], path[i], path[i + 1]));
        }

        // 閉路の場合、接続部でのターンを追加
        if (path.front() == path.back() && path.size() > 2)
        {
            turns.push_back(getTurn(path[path.size() - 2], path[0], path[1]));
        }

        return turns;
    }

    struct TurnCounts
    {
        int right = 0;
        int left = 0;
        int straight = 0;
        int uTurn = 0;
    };

    // パスの右折・左折・直進・Uターンの数をカウントする
    TurnCounts getTurnCounts(const std::vector<int> &path) const
    {
        TurnCounts counts;
        std::vector<std::string> turns = convertPathToTurns(path);
        for (const auto &t : turns)
        {
            if (t == "Right")
                counts.right++;
            else if (t == "Left")
                counts.left++;
            else if (t == "Straight")
                counts.straight++;
            else if (t == "U-Turn")
                counts.uTurn++;
        }
        return counts;
    }

    // 頂点が十字路（4方向すべてに接続している）かどうかを判定する
    bool isCrossroad(int u) const
    {
        int count = 0;
        for (const auto &e : getNeighbors(u))
        {
            if (e.weight > 0)
                count++;
        }
        return count == 4;
    }

    // 回路が十字路を直進しているか判定する
    bool isStraightAtCrossroads(const std::vector<int> &path) const
    {
        std::vector<std::string> turns = convertPathToTurns(path);
        for (size_t i = 0; i < turns.size(); ++i)
        {
            int v = path[i + 1];
            if (isCrossroad(v))
            {
                if (turns[i] != "Straight")
                    return false;
            }
        }
        return true;
    }

    // 回路が十字路で一度も直進しないか判定する
    bool isNeverStraightAtCrossroads(const std::vector<int> &path) const
    {
        std::vector<std::string> turns = convertPathToTurns(path);
        for (size_t i = 0; i < turns.size(); ++i)
        {
            int v = path[i + 1];
            if (isCrossroad(v))
            {
                if (turns[i] == "Straight")
                    return false;
            }
        }
        return true;
    }
};

#endif // WEIGHTED_GRID_GRAPH_HPP