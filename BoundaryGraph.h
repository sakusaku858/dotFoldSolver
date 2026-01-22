#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

struct Point {
    int x, y;
    bool operator<(const Point &other) const;
    bool operator==(const Point &other) const;
};

struct Edge {
    Point p1, p2;
    bool operator<(const Edge &o) const;
};

class BoundaryGraph {
  public:
    std::map<Point, std::vector<Point>> adj; // 隣接リスト
    int total_edges = 0;

    void addEdge(int x1, int y1, int x2, int y2);

    void build(const std::vector<std::vector<int>> &grid);

    // DFSで到達可能な頂点をカウント
    void dfs(const Point &u, std::set<Point> &visited);

    // 連結かどうかを判定する関数
    bool isConnected();

    // ループ（座標列）を方向（右折・左折・直進）の配列に変換する
    std::vector<std::string> getPathDirections(const std::vector<Point> &path);

    // 方向の配列から、右折と左折のどちらが優勢かを判定する
    // これにより、ループが時計回りか反時計回りかを大まかに判断できる
    std::string
    determineDominantTurn(const std::vector<std::string> &directions);

    // 方向配列を文字列に変換する (例: "Right", "Left" -> "RL")
    std::string directionsToString(const std::vector<std::string> &directions);

    std::vector<std::vector<Point>> findCycles();

    std::vector<std::vector<Point>> findClockwiseCycles();

    void removeEdge(Edge e);

  private:
    void backtrack(Point u, std::vector<Point> &path, std::set<Edge> &visited,
                   std::vector<std::vector<Point>> &results);
};