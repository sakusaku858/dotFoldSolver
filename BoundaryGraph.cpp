#include "BoundaryGraph.h"
#include <algorithm>

bool Point::operator<(const Point &other) const {
    if (x != other.x)
        return x < other.x;
    return y < other.y;
}

bool Point::operator==(const Point &other) const {
    return x == other.x && y == other.y;
}

bool Edge::operator<(const Edge &o) const {
    Point a1 = std::min(p1, p2), a2 = std::max(p1, p2);
    Point b1 = std::min(o.p1, o.p2), b2 = std::max(o.p1, o.p2);
    return std::tie(a1.x, a1.y, a2.x, a2.y) < std::tie(b1.x, b1.y, b2.x, b2.y);
}

void BoundaryGraph::addEdge(int x1, int y1, int x2, int y2) {
    Point a = {x1, y1};
    Point b = {x2, y2};
    adj[a].push_back(b);
    adj[b].push_back(a);
    total_edges++;
}

void BoundaryGraph::removeEdge(Edge e) {
    Point a = e.p1;
    Point b = e.p2;
    bool removed = false;

    if (adj.count(a)) {
        auto &neighbors = adj[a];
        auto it = std::find(neighbors.begin(), neighbors.end(), b);
        if (it != neighbors.end()) {
            neighbors.erase(it);
            removed = true;
        }
        if (neighbors.empty()) {
            adj.erase(a);
        }
    }

    if (adj.count(b)) {
        auto &neighbors = adj[b];
        auto it = std::find(neighbors.begin(), neighbors.end(), a);
        if (it != neighbors.end()) {
            neighbors.erase(it);
        }
        if (neighbors.empty()) {
            adj.erase(b);
        }
    }

    if (removed) {
        total_edges--;
    }
}

void BoundaryGraph::build(const std::vector<std::vector<int>> &grid) {
    int rows = 8;
    int cols = 8;

    for (int r = 0; r <= rows; ++r) {
        for (int c = 0; c <= cols; ++c) {
            // 現在の格子点 (c, r) から右方向と下方向の境界をチェック

            // 1. 横方向の境界 (c, r) --- (c+1, r)
            // 上のマスと下のマスの色が異なれば辺を張る
            if (c < cols) {
                int up = (r == 0) ? 0 : grid[r - 1][c];
                int down = (r == rows) ? 0 : grid[r][c];
                if (up != down) {
                    addEdge(c, r, c + 1, r);
                }
            }

            // 2. 縦方向の境界 (c, r) --- (c, r+1)
            // 左のマスと右のマスの色が異なれば辺を張る
            if (r < rows) {
                int left = (c == 0) ? 0 : grid[r][c - 1];
                int right = (c == cols) ? 0 : grid[r][c];
                if (left != right) {
                    addEdge(c, r, c, r + 1);
                }
            }
        }
    }
}

void BoundaryGraph::dfs(const Point &u, std::set<Point> &visited) {
    visited.insert(u);
    for (const auto &v : adj[u]) {
        if (visited.find(v) == visited.end()) {
            dfs(v, visited);
        }
    }
}

bool BoundaryGraph::isConnected() {
    if (adj.empty())
        return true; // 境界線がない（全て白など）場合は連結とみなす

    std::set<Point> visited;
    auto startNode = adj.begin()->first; // 最初の頂点から開始
    dfs(startNode, visited);

    // 訪問した頂点数と、グラフに存在する頂点数が一致するか
    return visited.size() == adj.size();
}

std::vector<std::string>
BoundaryGraph::getPathDirections(const std::vector<Point> &path) {
    std::vector<std::string> directions;

    if (path.size() < 2)
        return directions;

    size_t n = path.size() - 1;

    for (size_t i = 0; i < n; ++i) {
        Point p_prev = path[(i - 1 + n) % n];
        Point p_curr = path[i];
        Point p_next = path[(i + 1) % n];

        int dx_in = p_curr.x - p_prev.x;
        int dy_in = p_curr.y - p_prev.y;
        int dx_out = p_next.x - p_curr.x;
        int dy_out = p_next.y - p_curr.y;

        int cp = dx_in * dy_out - dy_in * dx_out;

        if (cp > 0)
            directions.push_back("Right");
        else if (cp < 0)
            directions.push_back("Left");
        else
            directions.push_back("Straight");
    }
    return directions;
}

std::string BoundaryGraph::determineDominantTurn(
    const std::vector<std::string> &directions) {
    int right_turns = 0;
    int left_turns = 0;

    for (const auto &dir : directions) {
        if (dir == "Right")
            right_turns++;
        else if (dir == "Left")
            left_turns++;
    }

    if (right_turns > left_turns)
        return "Clockwise";
    else if (left_turns > right_turns)
        return "Counter-Clockwise";
    else if (right_turns == 0 && left_turns == 0)
        return "Indeterminate (No turns)";
    return "Equal";
}

std::string
BoundaryGraph::directionsToString(const std::vector<std::string> &directions) {
    std::string str = "";
    for (const auto &dir : directions) {
        if (!dir.empty())
            str += dir[0];
    }
    return str;
}

std::vector<std::vector<Point>> BoundaryGraph::findCycles() {
    std::vector<Point> path;
    std::set<Edge> visited;
    std::vector<std::vector<Point>> results;

    if (adj.empty())
        return results;

    Point start = adj.begin()->first;
    path.push_back(start);

    backtrack(start, path, visited, results);
    return results;
}

std::vector<std::vector<Point>> BoundaryGraph::findClockwiseCycles() {
    std::vector<std::vector<Point>> all_cycles = findCycles();
    std::vector<std::vector<Point>> cw_cycles;

    for (const auto &cycle : all_cycles) {
        std::vector<std::string> dirs = getPathDirections(cycle);
        if (determineDominantTurn(dirs) == "Clockwise") {
            cw_cycles.push_back(cycle);
        }
    }
    return cw_cycles;
}

void BoundaryGraph::backtrack(Point u, std::vector<Point> &path,
                              std::set<Edge> &visited,
                              std::vector<std::vector<Point>> &results) {
    if (visited.size() == (size_t)total_edges) {
        if (u == path[0]) {
            results.push_back(path);
        }
        return;
    }

    for (const auto &v : adj[u]) {
        Edge e = {u, v};
        if (visited.find(e) == visited.end()) {
            if (path.size() >= 2 && adj[u].size() == 4) {
                Point prev = path[path.size() - 2];
                if ((prev.x == v.x) || (prev.y == v.y)) {
                    continue;
                }
            }

            visited.insert(e);
            path.push_back(v);
            backtrack(v, path, visited, results);
            path.pop_back();
            visited.erase(e);
        }
    }
}