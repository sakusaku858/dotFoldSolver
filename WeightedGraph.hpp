#ifndef WEIGHTED_GRAPH_H
#define WEIGHTED_GRAPH_H

#include <algorithm>
#include <iostream>
#include <vector>

// 辺を表す構造体
struct Edge
{
    int to;     // 行き先の頂点ID
    int weight; // 辺の重み

    Edge(int t, int w) : to(t), weight(w) {}
};

// 重み付きグラフクラス (整数重み・無向グラフ)
class WeightedGraph
{
private:
    int V;                              // 頂点数
    std::vector<std::vector<Edge>> adj; // 隣接リスト

public:
    // コンストラクタ
    // vertices: 頂点数
    WeightedGraph(int vertices) : V(vertices) { adj.resize(vertices); }

    // 辺の追加
    void addEdge(int u, int v, int w)
    {
        // 範囲外アクセス防止
        if (u < 0 || u >= V || v < 0 || v >= V)
            return;

        adj[u].emplace_back(v, w);
        // 無向グラフなので逆方向の辺も追加
        adj[v].emplace_back(u, w);
    }

    // 指定した頂点から出る辺のリストを取得
    const std::vector<Edge> &getNeighbors(int u) const { return adj[u]; }

    // 頂点数の取得
    int getVertexCount() const { return V; }

    // グラフの情報を表示（デバッグ用）
    void print() const
    {
        for (int i = 0; i < V; ++i)
        {
            std::cout << i << ":";
            for (const auto &e : adj[i])
            {
                std::cout << " -> " << e.to << "(" << e.weight << ")";
            }
            std::cout << std::endl;
        }
    }

    // 重みが0以外の辺だけを考慮したときに、辺を持つ頂点集合が連結しているかを判定する
    // (辺を持たない孤立点は無視する)
    bool isConnectedIgnoringZeroWeightEdges() const
    {
        int startNode = -1;
        // 重みが正の辺を持つ最初の頂点を探す
        for (int i = 0; i < V; ++i)
        {
            for (const auto &e : adj[i])
            {
                if (e.weight > 0)
                {
                    startNode = i;
                    break;
                }
            }
            if (startNode != -1)
                break;
        }

        // 重みが正の辺が一つもない場合は連結とみなす
        if (startNode == -1)
            return true;

        // BFSで到達可能な頂点を探索
        std::vector<bool> visited(V, false);
        std::vector<int> q;
        q.reserve(V);
        q.push_back(startNode);
        visited[startNode] = true;

        int head = 0;
        while (head < (int)q.size())
        {
            int u = q[head++];
            for (const auto &e : adj[u])
            {
                if (e.weight > 0)
                {
                    if (!visited[e.to])
                    {
                        visited[e.to] = true;
                        q.push_back(e.to);
                    }
                }
            }
        }

        // 重みが正の辺を持つすべての頂点が訪問されたか確認
        for (int i = 0; i < V; ++i)
        {
            bool hasPositiveEdge = false;
            for (const auto &e : adj[i])
            {
                if (e.weight > 0)
                {
                    hasPositiveEdge = true;
                    break;
                }
            }

            // 重みのある辺を持つのに、探索で到達できなかった頂点がある場合
            if (hasPositiveEdge && !visited[i])
            {
                return false;
            }
        }

        return true;
    }

    // すべての辺をその重みの回数分通って、元の場所に戻ってくる方法（オイラー回路）を列挙する
    std::vector<std::vector<int>> findAllEulerianCircuits(int startNode = 0)
    {
        std::vector<std::vector<int>> results;

        long long totalWeight = 0;
        for (int i = 0; i < V; ++i)
        {
            long long degree = 0;
            for (const auto &e : adj[i])
            {
                degree += e.weight;
            }
            if (degree % 2 != 0)
            {
                return results; // 次数が奇数の頂点がある場合、オイラー回路は存在しない
            }
            totalWeight += degree;
        }
        totalWeight /= 2;

        std::vector<std::vector<Edge>> currentAdj = adj;
        std::vector<int> path;
        path.reserve(totalWeight + 1);
        path.push_back(startNode);

        dfsEuler(startNode, totalWeight, path, currentAdj, results);

        return results;
    }

private:
    void dfsEuler(int u, long long edgesLeft, std::vector<int> &path,
                  std::vector<std::vector<Edge>> &currentAdj,
                  std::vector<std::vector<int>> &results)
    {
        if (edgesLeft == 0)
        {
            if (u == path[0])
            {
                results.push_back(path); // ここで現在の path のコピーが保存されます
            }
            return;
        }

        for (auto &e : currentAdj[u])
        {
            if (e.weight > 0)
            {
                int v = e.to;
                e.weight--;
                for (auto &revE : currentAdj[v])
                {
                    if (revE.to == u && revE.weight > 0)
                    {
                        revE.weight--;
                        path.push_back(v); // 経路を進める
                        dfsEuler(v, edgesLeft - 1, path, currentAdj, results);
                        path.pop_back(); // バックトラック：探索から戻ったら状態を元に戻す
                        revE.weight++;
                        break; // 1つの逆辺だけを戻す
                    }
                }
                e.weight++;
            }
        }
    }
};

#endif // WEIGHTED_GRAPH_H