#include "WeightedGridGraph.hpp"
#include "foldsToEdges.h"
#include "ftcp.h"
#include "loopToFolds.h"
#include <algorithm>
#include <iostream>
#include <tuple>
#include <array>
#include <vector>
#include <string>

using namespace std;

// 文字列の先頭n文字を末尾に移動させる関数
static string rotateString(string s, int n)
{
    if (s.empty())
        return s;
    n %= s.length();
    return s.substr(n) + s.substr(0, n);
}

struct Point
{
    int x;
    int y;
};

void countAndPrintTurns(const WeightedGridGraph &graph,
                        const vector<vector<int>> &circuits)
{
    for (size_t i = 0; i < circuits.size(); ++i)
    {
        WeightedGridGraph::TurnCounts counts = graph.getTurnCounts(circuits[i]);
        cout << "Circuit " << i << " Turns: "
             << "Right: " << counts.right << ", "
             << "Left: " << counts.left << ", "
             << "Straight: " << counts.straight << ", "
             << "U-Turn: " << counts.uTurn << endl;
    }
}

void testConnected()
{
    cout << "--- Test Case 1: Connected Graph ---" << endl;
    // 3x3 のグリッドグラフ
    // (0,0)-(1,0)-(2,0)
    //   |           | (weight 0)
    // (0,1)       (2,1)
    //   |
    // (0,2)-(1,2)-(2,2)
    WeightedGridGraph graph(3, 3);

    // 連結成分 (重み2)
    graph.addEdge(0, 0, 1, 0, 2);
    graph.addEdge(1, 0, 2, 0, 2);
    graph.addEdge(0, 0, 0, 1, 2);
    graph.addEdge(0, 1, 0, 2, 2);
    graph.addEdge(0, 2, 1, 2, 2);
    graph.addEdge(1, 2, 2, 2, 2);

    // 孤立点に近い頂点 (2,1) を重み0の辺で接続
    // 重み0の辺しか持たない頂点は、連結性判定において無視されるはず
    graph.addEdge(2, 0, 2, 1, 0);

    cout << "Graph structure:" << endl;
    graph.print();

    if (graph.isConnectedIgnoringZeroWeightEdges())
    {
        cout << "Result: Connected (Correct)" << endl;
    }
    else
    {
        cout << "Result: NOT Connected (Incorrect)" << endl;
    }
    cout << endl;
}

void testDisconnected()
{
    cout << "--- Test Case 2: Disconnected Graph ---" << endl;
    // 3x3 のグリッドグラフ
    // 左上と右下に重みのある成分があり、間は重み0でつながっている
    WeightedGridGraph graph(3, 3);

    // 左上の成分 (重み2)
    graph.addEdge(0, 0, 0, 1, 2);
    graph.addEdge(0, 0, 1, 0, 2);

    // 右下の成分 (重み2)
    graph.addEdge(2, 2, 2, 1, 2);
    graph.addEdge(2, 2, 1, 2, 2);

    // 間をつなぐ辺 (重み0)
    // これによりグラフ全体としては連結だが、重み正の辺だけ見ると非連結
    graph.addEdge(1, 0, 1, 1, 0);
    graph.addEdge(1, 1, 1, 2, 0);

    cout << "Graph structure:" << endl;
    graph.print();

    if (graph.isConnectedIgnoringZeroWeightEdges())
    {
        cout << "Result: Connected (Incorrect)" << endl;
    }
    else
    {
        cout << "Result: NOT Connected (Correct)" << endl;
    }
    cout << endl;
}

void testFullGrid()
{
    cout << "--- Test Case 3: Full Grid Graph ---" << endl;
    // 3x2のグリッドグラフを作成 (width=3, height=2)
    WeightedGridGraph graph(3, 2);

    // 全ての隣接頂点間に重み1の辺を自動生成
    cout << "Initializing a full grid with weight 1..." << endl;
    graph.initializeGridEdges(1);

    cout << "Graph structure:" << endl;
    graph.print();

    if (graph.isConnectedIgnoringZeroWeightEdges())
    {
        cout << "Result: Connected (Correct)" << endl;
    }
    else
    {
        cout << "Result: NOT Connected (Incorrect)" << endl;
    }
    cout << endl;
}

void testEdgesToAdd()
{
    cout << "--- Test Case 4: Edges to Add (Manhattan Distance) ---" << endl;
    // 4x4 grid
    WeightedGridGraph graph(4, 4);

    // Component 1: (0,0)-(0,1)
    graph.addEdge(0, 0, 0, 1, 2);

    // Component 2: (0,3)-(1,3)  (Component 1から距離2離れている)
    graph.addEdge(0, 3, 1, 3, 2);

    // Component 3: (3,0)-(3,1) (Component 1から距離3, Component
    // 2から距離4離れている)
    graph.addEdge(3, 0, 3, 1, 2);

    cout << "Graph structure:" << endl;
    graph.print();

    int edgesNeeded = graph.getMinEdgesToConnect();
    // 予想: Comp1-Comp2 (dist 2) + Comp1-Comp3 (dist 3) = 5 edges
    cout << "Edges needed to connect: " << edgesNeeded << endl;
    cout << endl;
}

void testPathToConnect()
{
    cout << "--- Test Case 5: Path to Connect ---" << endl;
    WeightedGridGraph graph(5, 5);

    // Component 1: (0,0)
    graph.addEdge(0, 0, 1, 0, 2);

    // Component 2: (4,4)
    graph.addEdge(4, 4, 3, 4, 2);

    // Component 3: (0,4)
    graph.addEdge(0, 4, 1, 4, 2);

    auto edges = graph.getEdgesToConnect();
    cout << "Edges to add:" << endl;
    for (const auto &edge : edges)
    {
        cout << "(" << get<0>(edge) << "," << get<1>(edge) << ") - ("
             << get<2>(edge) << "," << get<3>(edge) << ")" << endl;
    }
    cout << "Total added edges: " << edges.size() << endl;
    cout << endl;
}

void testDotArtBoundary()
{
    cout << "--- Test Case 6: Dot Art Boundary ---" << endl;
    // 3x3 のドット絵
    // 0 1 0
    // 1 1 1
    // 0 1 0
    vector<vector<int>> dotArt = {{0, 1, 0}, {1, 1, 1}, {0, 1, 0}};

    int rows = dotArt.size();
    int cols = dotArt[0].size();

    // グリッドグラフの頂点数は (cols+1) x (rows+1)
    WeightedGridGraph graph(cols + 1, rows + 1);

    graph.initializeFromDotArt(dotArt);

    cout << "Graph structure (Boundaries):" << endl;
    graph.print();

    if (graph.isConnectedIgnoringZeroWeightEdges())
    {
        cout << "Result: Connected" << endl;
    }
    else
    {
        cout << "Result: NOT Connected" << endl;
    }
    cout << endl;
}

void testEulerTurns()
{
    cout << "--- Test Case 7: Euler Turns ---" << endl;
    // 2x2 Grid
    // (0,0)-(1,0)
    //   |     |
    // (0,1)-(1,1)
    WeightedGridGraph graph(2, 2);
    graph.addEdge(0, 0, 1, 0, 1);
    graph.addEdge(1, 0, 1, 1, 1);
    graph.addEdge(1, 1, 0, 1, 1);
    graph.addEdge(0, 1, 0, 0, 1);

    // Path: 0->1->3->2->0 (indices: 0, 1, 3, 2, 0)
    // (0,0)->(1,0)->(1,1)->(0,1)->(0,0)

    auto circuits = graph.findAllEulerianCircuits(0);
    if (!circuits.empty())
    {
        auto path = circuits[0];
        auto turns = graph.convertPathToTurns(path);
        cout << "Path: ";
        for (int v : path)
            cout << v << " ";
        cout << endl;

        cout << "Turns: ";
        for (const auto &t : turns)
            cout << t << " ";
        cout << endl;

        countAndPrintTurns(graph, circuits);
    }
    cout << endl;
}

void testDotArt8x8()
{
    cout << "--- Test Case 8: 8x8 Dot Art (Hollow Square) ---" << endl;
    // 8x8 のドット絵（中抜きの四角形）を作成
    // 1 1 1 1 1 1 1 1
    // 1 0 0 0 0 0 0 1
    // ...
    // 1 1 1 1 1 1 1 1

    int rows = 8;
    int cols = 8;
    vector<vector<int>> dotArt(rows, vector<int>(cols, 0));

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            // 外周を1（黒）、内部を0（白）にする
            if (r == 0 || r == rows - 1 || c == 0 || c == cols - 1)
            {
                dotArt[r][c] = 1;
            }
        }
    }

    // グリッドグラフの頂点数は (cols+1) x (rows+1) = 9x9
    WeightedGridGraph graph(cols + 1, rows + 1);

    // dot絵からグリッドグラフを生成
    graph.initializeFromDotArt(dotArt);
    cout << "Graph structure (Dot Art):" << endl;
    graph.print();

    // 連結にするために必要な辺集合を取得
    auto addEdges = graph.getEdgesToConnect();

    // 橋となる辺を重み2で追加
    for (auto &edge : addEdges)
    {
        int x1 = get<0>(edge);
        int y1 = get<1>(edge);
        int x2 = get<2>(edge);
        int y2 = get<3>(edge);
        graph.addEdge(x1, y1, x2, y2, 2);
        cout << "(" << x1 << "," << y1 << ") - (" << x2 << "," << y2 << ")"
             << endl;
    }

    // オイラー回路を取得
    auto circuits = graph.findAllEulerianCircuits(0);
    if (!circuits.empty())
    {
        auto path = circuits[0];
        for (auto v : path)
        {
            int x = v % (cols + 1);
            int y = v / (cols + 1);
            cout << "(" << x << "," << y << ") ";
        }
        cout << endl;
        countAndPrintTurns(graph, circuits);
    }
}

void testTurnCounts()
{
    cout << "--- Test Case 9: Turn Counts Verification ---" << endl;
    WeightedGridGraph graph(3, 3);
    // Path: (0,0)->(1,0)->(2,0)->(2,1)->(1,1)->(1,0)->(0,0)
    // Indices: 0 -> 1 -> 2 -> 5 -> 4 -> 1 -> 0
    vector<int> path = {0, 1, 2, 5, 4, 1, 0};

    WeightedGridGraph::TurnCounts counts = graph.getTurnCounts(path);

    cout << "Path: (0,0)->(1,0)->(2,0)->(2,1)->(1,1)->(1,0)->(0,0)" << endl;
    cout << "Expected: Right: 3, Left: 1, Straight: 1, U-Turn: 1" << endl;
    cout << "Actual:   Right: " << counts.right << ", Left: " << counts.left
         << ", Straight: " << counts.straight << ", U-Turn: " << counts.uTurn
         << endl;

    if (counts.right == 3 && counts.left == 1 && counts.straight == 1 &&
        counts.uTurn == 1)
    {
        cout << "Result: PASS" << endl;
    }
    else
    {
        cout << "Result: FAIL" << endl;
    }
    cout << endl;
}

void testCrossroadStraight()
{
    cout << "--- Test Case 10: Crossroad Straight Verification ---" << endl;
    // 3x3 Grid
    // (1,1) is the center.
    // Create a figure-8 graph centered at (1,1).
    // (1,0)-(1,1)-(1,2) vertical line
    // (0,1)-(1,1)-(2,1) horizontal line
    // Connect ends to form loops:
    // (1,0)-(0,0)-(0,1)
    // (1,2)-(2,2)-(2,1)

    WeightedGridGraph graph(3, 3);
    // Cross at (1,1)
    graph.addEdge(1, 0, 1, 1, 1);
    graph.addEdge(1, 1, 1, 2, 1);
    graph.addEdge(0, 1, 1, 1, 1);
    graph.addEdge(1, 1, 2, 1, 1);

    // Close loops
    graph.addEdge(1, 0, 0, 0, 1);
    graph.addEdge(0, 0, 0, 1, 1);

    graph.addEdge(1, 2, 2, 2, 1);
    graph.addEdge(2, 2, 2, 1, 1);

    // Circuit 1: Straight at (1,1)
    // Path: 4 -> 1 -> 0 -> 3 -> 4 -> 5 -> 8 -> 7 -> 4
    vector<int> straightPath = {4, 1, 0, 3, 4, 5, 8, 7, 4};

    // Circuit 2: Turn at (1,1)
    // Path: 4 -> 1 -> 0 -> 3 -> 4 -> 7 -> 8 -> 5 -> 4
    vector<int> turnPath = {4, 1, 0, 3, 4, 7, 8, 5, 4};

    cout << "Testing Straight Path..." << endl;
    if (graph.isStraightAtCrossroads(straightPath))
    {
        cout << "Result: Straight (PASS)" << endl;
    }
    else
    {
        cout << "Result: Not Straight (FAIL)" << endl;
    }

    cout << "Testing Turn Path..." << endl;
    if (!graph.isStraightAtCrossroads(turnPath))
    {
        cout << "Result: Not Straight (PASS)" << endl;
    }
    else
    {
        cout << "Result: Straight (FAIL)" << endl;
    }
    cout << endl;
}

int distance(int x1, int y1, int x2, int y2)
{
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

int OUTER_X[32] = {
    0, 1, 2, 3, 4, 5, 6, 7, //
    8, 8, 8, 8, 8, 8, 8, 8, //
    8, 7, 6, 5, 4, 3, 2, 1, //
    0, 0, 0, 0, 0, 0, 0, 0  //
};

int OUTER_Y[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, //
    0, 1, 2, 3, 4, 5, 6, 7, //
    8, 8, 8, 8, 8, 8, 8, 8, //
    8, 7, 6, 5, 4, 3, 2, 1  //
};

bool can_slide(const vector<Point> &circuit, int step)
{
    for (int i = 0; i < 32; i++)
    {
        for (int j = i + 1; j < 32; j++)
        {
            // 展開図上でのiとjの距離の計算
            int x1 = OUTER_X[(i - 4 + 32) % 32];
            int y1 = OUTER_Y[(i - 4 + 32) % 32];
            int x2 = OUTER_X[(j - 4 + 32) % 32];
            int y2 = OUTER_Y[(j - 4 + 32) % 32];
            int dist1 = distance(x1, y1, x2, y2);

            // サーキット上でのiとjの距離の計算
            int p1 = i;
            int p2 = j;
            int x3 = circuit[p1].x;
            int y3 = circuit[p1].y;
            int x4 = circuit[p2].x;
            int y4 = circuit[p2].y;
            int dist2 = distance(x3, y3, x4, y4);

            // 展開図上での距離とサーキット上での距離の比較
            if (dist1 < dist2)
            {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    Counter cpFinder(7);

    // 8x8の飛び地のあるドット絵の例
    string dotstr = "00000000"
                    "01011100"
                    "00110100"
                    "00011110"
                    "01111100"
                    "00000000"
                    "00000000"
                    "00000000";

    WeightedGridGraph graph(dotstr);

    // 連結成分をつなぐための辺を追加（重み2で往復分）
    // 1通りのつなぎ方しか検証していない TODO
    auto addedEdges = graph.getEdgesToConnect();
    for (const auto &edge : addedEdges)
    {
        graph.addEdge(get<0>(edge), get<1>(edge), get<2>(edge), get<3>(edge),
                      2);
    }

    // 開始点を探す（次数が0でない最初の頂点）
    int startNode = -1;
    for (int i = 0; i < graph.getVertexCount(); ++i)
    {
        if (!graph.getNeighbors(i).empty())
        {
            bool hasEdge = false;
            for (auto &e : graph.getNeighbors(i))
            {
                if (e.weight > 0)
                {
                    hasEdge = true;
                    break;
                }
            }
            if (hasEdge)
            {
                startNode = i;
                break;
            }
        }
    }

    if (startNode != -1)
    {
        auto circuits = graph.findAllEulerianCircuits(startNode);

        if (!circuits.empty())
        {
            cout << "Filtering circuits..." << endl;
            int matchCount = 0;
            for (const auto &circuit : circuits)
            {
                WeightedGridGraph::TurnCounts counts =
                    graph.getTurnCounts(circuit);

                // 条件1: RightのほうがLeftよりもちょうど4回多い
                bool cond1 = (counts.right == counts.left + 4);

                // 条件2: 十字路で直進しない
                bool cond2 = graph.isNeverStraightAtCrossroads(circuit);

                if (cond1 && cond2)
                {
                    matchCount++;
                    cout << "Matched Circuit " << matchCount << ":" << endl;
                    countAndPrintTurns(graph, {circuit});
                    cout << endl;

                    // サーキットを方向の列に変換
                    auto turns = graph.convertPathToTurns(circuit);

                    // turnsを1つの文字列に変換
                    string turnString = "";
                    for (const auto &t : turns)
                    {
                        if (t == "Right")
                            turnString += "R";
                        else if (t == "Left")
                            turnString += "L";
                        else if (t == "Straight")
                            turnString += "S";
                        else if (t == "U-Turn")
                            turnString += "X";
                    }

                    // 8通りのズラシ表現のうち、可能なものだけを抜き出す
                    array<string, 8> turnstrs;
                    for (int i = 0; i < 8; i++)
                    {
                        turnstrs[i] = rotateString(turnString, i);
                    }

                    // LR形式を折り割当形式に変換する
                    vector<vector<int>> folds;
                    for (auto &turnstr : turnstrs)
                    {
                        vector<vector<int>> tmp = createAllFolds(turnstr);
                        folds.insert(folds.end(), tmp.begin(), tmp.end());
                    }

                    cout << "Generated " << folds.size() << " folds." << endl;

                    int skip = 5;
                    // foldsが平坦折り可能か
                    for (size_t k = 0; k < folds.size(); k += skip)
                    {
                        const auto &fold = folds[k];
                        array<int, 32> foldArr;
                        copy_n(fold.begin(), 32, foldArr.begin());

                        vector<int> edges = create_edges_by_folds(foldArr);
                        string cpStr = cpFinder.edges_to_cpstr(edges);
                        if (cpStr != "No CP")
                        {
                            cout << "Found CP: " << cpStr << endl;
                            return 0;
                        }
                    }
                }
            }

            if (matchCount == 0)
            {
                cout << "No circuits found meeting the conditions." << endl;
            }
        }
        else
        {
            cout << "Failed to find an Eulerian circuit." << endl;
        }
    }
    else
    {
        cout << "The graph contains no edges." << endl;
    }
    cout << "No CP" << endl;

    return 0;
}