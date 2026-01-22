#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// --- 1. Common Types ---
using Node = int;
using Edge = std::pair<Node, Node>;
using EdgeList = std::vector<Edge>;

// --- 2. PathManager ---
class PathManager
{
private:
    int width = 9;

    // 頂点ペア (u, v)をキーとして、最短経路のリストを保存するキャッシュ
    map<pair<Node, Node>, vector<EdgeList>> path_cache;

    // 辺を（小さい頂点、大きい頂点）で正規化して作成
    Edge make_normalized_edge(Node u, Node v)
    {
        return {min(u, v), max(u, v)};
    }

public:
    // マンハッタン距離を計算
    int get_dist(Node u, Node v)
    {
        int x1 = u % width;
        int y1 = u / width;
        int x2 = v % width;
        int y2 = v / width;
        return abs(x1 - x2) + abs(y1 - y2);
    }

    // 2点間のすべての最短経路（EdgeList）を返す
    vector<EdgeList> get_all_shortest_paths(Node start, Node target)
    {
        // キャッシュがあればそれを返す（向きを考慮してペアをソートして管理）
        Node u = min(start, target);
        Node v = max(start, target);
        if (path_cache.count({u, v}))
            return path_cache[{u, v}];

        vector<EdgeList> results;
        EdgeList current_path;

        // ヘルパー関数で再帰探索
        generate_paths_recursive(u, v, current_path, results);

        path_cache[{u, v}] = results;
        return results;
    }

private:
    void generate_paths_recursive(Node curr, Node target, EdgeList &path, vector<EdgeList> &results)
    {
        if (curr == target)
        {
            results.push_back(path);
            return;
        }

        int curX = curr % width;
        int curY = curr / width;
        int tarX = target % width;
        int tarY = target / width;

        // X方向に近づく
        if (curX != tarX)
        {
            int next = curr + (tarX > curX ? 1 : -1);
            path.push_back(make_normalized_edge(curr, next));
            generate_paths_recursive(next, target, path, results);
            path.pop_back();
        }

        // Y方向に近づく
        if (curY != tarY)
        {
            int next = curr + (tarY > curY ? width : -width);
            path.push_back(make_normalized_edge(curr, next));
            generate_paths_recursive(next, target, path, results);
            path.pop_back();
        }
    }
};

// --- 3. ComponentManager ---

class UnionFind
{
private:
    std::vector<int> parent;
    std::vector<int> rank;
    int num_sets; // 連結成分の数

public:
    // nは頂点数（9x9なら81）
    UnionFind(int n) : parent(n), rank(n, 0), num_sets(n)
    {
        // 最初は全頂点が自分自身を親（独立した集合）とする
        std::iota(parent.begin(), parent.end(), 0);
    }

    // どの集合に属しているか（代表元）を探す
    int find(int i)
    {
        if (parent[i] == i)
            return i;
        // 経路圧縮：探索ついでに親を代表元に直接繋ぎ直す
        return parent[i] = find(parent[i]);
    }

    // 2つの集合を合併する
    bool unite(int i, int j)
    {
        int root_i = find(i);
        int root_j = find(j);

        if (root_i != root_j)
        {
            // ランク（木の高さ）が低い方を高い方に繋ぐ
            if (rank[root_i] < rank[root_j])
            {
                parent[root_i] = root_j;
            }
            else
            {
                parent[root_j] = root_i;
                if (rank[root_i] == rank[root_j])
                    rank[root_i]++;
            }
            num_sets--; // 合併したので集合の数が減る
            return true;
        }
        return false;
    }

    // 同じ集合に属しているか判定
    bool same(int i, int j)
    {
        return find(i) == find(j);
    }

    // 現在の独立した集合の数を返す
    int count() const
    {
        return num_sets;
    }
};

struct Component
{
    int id;
    vector<Node> nodes;
    EdgeList edges;
};

struct Bridge
{
    int comp1_id, comp2_id;
    int dist;
    vector<pair<Node, Node>> best_pairs;
};

// MSTの結果を保持する構造体
struct MSTFramework
{
    vector<int> bridge_indices; // 使うBridgeのid
    int total_dist;
};

class ComponentManager
{
public:
    // 辺集合Fから連結成分を抽出する
    vector<Component> extract_components(const EdgeList &F)
    {
        if (F.empty())
            return {};

        // 1. Fに含まれる全頂点を特定
        set<Node> node_set;
        for (const auto &e : F)
        {
            node_set.insert(e.first);
            node_set.insert(e.second);
        }

        // 2. Union-Findで連結判定
        UnionFind uf(81);
        for (const auto &e : F)
        {
            uf.unite(e.first, e.second);
        }

        // 3. 代表元ごとに頂点と辺をグループ化
        map<int, vector<Node>> group_nodes;
        for (Node v : node_set)
        {
            group_nodes[uf.find(v)].push_back(v);
        }

        map<int, EdgeList> group_edges;
        for (const auto &e : F)
        {
            group_edges[uf.find(e.first)].push_back(e);
        }

        // 4. Component構造体のリストに変換
        vector<Component> components;
        int comp_id = 0;
        for (const auto &[root, nodes] : group_nodes)
        {
            components.push_back({comp_id++, nodes, group_edges[root]});
        }

        return components;
    }

    // コンポーネント間の全ペアについて最短距離と頂点ペアを計算する
    vector<Bridge> compute_all_bridges(const vector<Component> &comps, PathManager &pm)
    {
        vector<Bridge> bridges;
        int n = comps.size();

        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                Bridge bridge;
                bridge.comp1_id = comps[i].id;
                bridge.comp2_id = comps[j].id;
                bridge.dist = 1e9; // 十分に大きな値で初期化

                // comps[i]の全頂点 u とcomps[j]の全頂点 v を総当たり
                for (Node u : comps[i].nodes)
                {
                    for (Node v : comps[j].nodes)
                    {
                        int d = pm.get_dist(u, v);
                        if (d < bridge.dist)
                        {
                            bridge.dist = d;
                            bridge.best_pairs.clear();
                            bridge.best_pairs.push_back({u, v});
                        }
                        else if (d == bridge.dist)
                        {
                            bridge.best_pairs.push_back({u, v});
                        }
                    }
                }
                bridges.push_back(bridge);
            }
        }
        return bridges;
    }

    // 最小全域木となる橋の選び方をresultに格納する
    vector<MSTFramework> find_all_msts_brute_force(const std::vector<Bridge> &bridges, int num_comps)
    {
        if (num_comps <= 1)
            return {{{}, 0}}; // すでに連結

        int total_bridge_candidates = bridges.size(); // 橋の候補数
        int k = num_comps - 1;                        // 連結に必要な橋の数
        int min_total_dist = 1e9;
        std::vector<MSTFramework> results;

        // 選ぶ・選ばないを示すフラグ配列 (0: 選ばない, 1: 選ぶ)
        // 例: 10本の候補から4本選ぶ場合、0が6個、1が4個
        std::vector<int> selection_flags(total_bridge_candidates, 0);
        for (int i = 0; i < k; ++i)
        {
            selection_flags[total_bridge_candidates - 1 - i] = 1;
        }

        // next_permutation を使って全ての組み合わせを試す
        do
        {
            int current_dist = 0;
            UnionFind uf(num_comps); // コンポーネントID (0 ~ num_comps-1) を頂点として扱う
            std::vector<int> chosen_indices;

            for (int i = 0; i < total_bridge_candidates; ++i)
            {
                if (selection_flags[i] == 1)
                {
                    current_dist += bridges[i].dist;
                    uf.unite(bridges[i].comp1_id, bridges[i].comp2_id);
                    chosen_indices.push_back(i);
                }
            }

            // 判定1: 全てのコンポーネントが1つの集合になったか
            // (k本の橋を架けて閉路がなければ必ず連結になる)
            bool is_connected = (uf.count() == 1);

            if (is_connected)
            {
                if (current_dist < min_total_dist)
                {
                    min_total_dist = current_dist;
                    results.clear();
                    results.push_back({chosen_indices, current_dist});
                }
                else if (current_dist == min_total_dist)
                {
                    results.push_back({chosen_indices, current_dist});
                }
            }
        } while (std::next_permutation(selection_flags.begin(), selection_flags.end()));

        return results;
    }
};

// --- 4. EulerSolver ---

using MultiEdgeMap = std::map<Edge, int>; // 各辺の使用回数
std::vector<MultiEdgeMap> final_variants;

class EulerSolver
{
public:
    // 最終結果の保存先
    std::vector<std::vector<Node>> all_solutions;

private:
    // 探索用の作業変数
    MultiEdgeMap current_graph;
    int target_steps;
    std::vector<Node> current_path;

public:
    // 処理の開始
    void solve(const EdgeList &F,
               const std::vector<MSTFramework> &msts,
               const std::vector<Bridge> &bridges,
               PathManager &pm)
    {

        // 過去の探索結果をクリア
        all_solutions.clear();

        // 1. 辺の向きを正規化してカウント1でベースマップに登録
        MultiEdgeMap base_map;
        for (const auto &e : F)
        {
            Edge normalized_e = std::minmax(e.first, e.second);
            base_map[normalized_e]++;
        }

        // 2.見つかっている最短のMSTパターン（橋の架け方）を一つずつ試す
        for (const auto &mst : msts)
        {
            // この中で「どの最短経路を通るか」を決める次の再帰へ
            generate_variations(0, mst, bridges, pm, base_map);
        }
    }

private:
    // すべての橋について具体的な経路を選ぶ &
    // 得られたグラフについて実際の歩き方を探す
    void generate_variations(
        int bridge_idx,
        const MSTFramework &mst,
        const std::vector<Bridge> &bridges,
        PathManager &pm,
        MultiEdgeMap &current_map)
    {

        // 全ての「橋」について具体的な経路を選び終わった場合
        if (bridge_idx == mst.bridge_indices.size())
        {
            // --- ここで一筆書きの準備 ---
            // どこから歩き始めても閉路なので、存在する最初の頂点から開始する
            Node start_node = current_map.begin()->first.first;

            // 延べ何歩歩く必要があるか計算
            int total_steps = 0;
            for (auto const &[edge, count] : current_map)
                total_steps += count;

            std::vector<Node> path;
            path.push_back(start_node);

            find_circuits(start_node, current_map, path, total_steps);
            return;
        }

        // まだ選んでいない「橋」を取り出す
        int b_idx = mst.bridge_indices[bridge_idx];
        const Bridge &bridge = bridges[b_idx];

        // その橋を実現する「頂点ペア」をすべて試す
        for (const auto &p : bridge.best_pairs)
        {
            // そのペア間の「最短経路」をすべて試す
            auto paths = pm.get_all_shortest_paths(p.first, p.second);

            for (const auto &path_edges : paths)
            {
                // 最短経路を「往復分」追加 (+2)
                for (const auto &e : path_edges)
                    current_map[e] += 2;

                // 次の橋の経路を選びに進む
                generate_variations(bridge_idx + 1, mst, bridges, pm, current_map);

                // 探索が終わったら元に戻す
                for (const auto &e : path_edges)
                {
                    current_map[e] -= 2;
                }
            }
        }
    }

    /**
     * @brief オイラー回路（一筆書き）をバックトラッキングで探索する
     * @param u 現在の頂点
     * @param counts 辺の残り通過可能回数（書き換えながら探索）
     * @param path 現在までの歩みの記録（頂点のリスト）
     * @param remaining_edges 残りの総辺数（0になったらゴール）
     */
    void find_circuits(Node u,
                       MultiEdgeMap &counts,
                       std::vector<Node> &path,
                       int remaining_edges)
    {
        // 全ての辺を使い切った場合
        if (remaining_edges == 0)
        {
            all_solutions.push_back(path);
            return;
        }

        // 現在の頂点 u に接続している辺をすべて調べる
        // ※ 9x9グリッドなので、上下左右の隣接ノードを直接チェックするのが速い
        static const int dx[] = {0, 0, 1, -1};
        static const int dy[] = {1, -1, 0, 0};

        int cx = u % 9;
        int cy = u / 9;

        for (int i = 0; i < 4; ++i)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (nx >= 0 && nx < 9 && ny >= 0 && ny < 9)
            {
                Node v = ny * 9 + nx;
                Edge e = std::minmax(u, v);

                // その道がまだ通れる（カウントが残っている）場合
                if (counts.count(e) && counts[e] > 0)
                {
                    // 1. 進む
                    counts[e]--;
                    path.push_back(v);

                    // 2. 次の地点で再帰探索
                    find_circuits(v, counts, path, remaining_edges - 1);

                    // 3. 戻る（バックトラッキング：状態を復元）
                    path.pop_back();
                    counts[e]++;
                }
            }
        }
    }

public:
    /**
     * @brief 探索されたすべての解をテキストファイルに書き出す
     * @param filename 出力ファイル名
     */
    void save_solutions_to_file(const std::string &filename)
    {
        std::ofstream ofs(filename);

        if (!ofs)
        {
            std::cerr << "Error: Cannot open file for writing: " << filename << std::endl;
            return;
        }

        for (auto &walk : all_solutions)
        {
            if (!walk.empty() && walk.front() == walk.back())
            {
                walk.pop_back();
            }

            for (size_t i = 0; i < walk.size(); ++i)
            {
                ofs << walk[i] << (i == walk.size() - 1 ? "" : " ");
            }
            ofs << "\n";
        }

        std::cout << "Successfully saved " << all_solutions.size() << " solutions to " << filename << std::endl;
    }
};

// --- テスト関数定義群 ---
void test_union_find();
void test_component_manager();
void test_bridge_calculation();
void test_mst_brute_force();

// --- 5. Main Control ---

/**
 * @brief テキストファイルから辺集合を読み込む
 * @param filename ファイル名
 * @return EdgeList 読み込んだ辺のリスト
 */
EdgeList load_edges_from_file(const std::string &filename)
{
    EdgeList edges;
    std::ifstream ifs(filename);

    if (!ifs)
    {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return edges;
    }

    int u, v;
    while (ifs >> u >> v)
    {
        // 0-80の範囲内かチェック（任意）
        if (u >= 0 && u < 81 && v >= 0 && v < 81)
        {
            edges.push_back({u, v});
        }
    }

    std::cout << "Loaded " << edges.size() << " edges from " << filename << std::endl;
    return edges;
}

/**
 * @brief メイン関数
 */
int main(int argc, char *argv[])
{
    bool run_tests = false; // テストを実行するかどうかのフラグ
    if (run_tests)
    {
        test_union_find();
        test_component_manager();
        test_bridge_calculation();
        test_mst_brute_force();
        return 0;
    }

    string input_filename = "edges.txt";
    string output_filename = "solutions.txt";
    if (argc == 3)
    {
        input_filename = argv[1];
        output_filename = argv[2];
    }

    // 1. ファイルから辺を読み込む
    EdgeList F = load_edges_from_file(input_filename);

    if (F.empty())
    {
        std::cerr << "No edges loaded. Exiting..." << std::endl;
        return 1;
    }

    // 2. 各マネージャーの初期化
    PathManager pm;
    ComponentManager cm;
    EulerSolver solver;

    // 3. コンポーネントの抽出とブリッジの計算
    auto comps = cm.extract_components(F);
    auto bridges = cm.compute_all_bridges(comps, pm);

    // 4. MST (最短の繋ぎ方) の全列挙
    auto msts = cm.find_all_msts_brute_force(bridges, comps.size());

    // 5. オイラー回路の探索
    solver.solve(F, msts, bridges, pm);

    // 6. 結果の出力
    std::cout << "Found " << solver.all_solutions.size() << " shortest walks." << std::endl;

    if (!solver.all_solutions.empty())
    {
        // 結果をファイルに保存
        solver.save_solutions_to_file(output_filename);
    }

    return 0;
}

// --- 6. テスト関数群 ---

void test_union_find()
{
    UnionFind uf(81);

    // (0,1) と (1,2) を繋ぐ
    uf.unite(0, 1);
    uf.unite(1, 2);

    std::cout << "UF Test 1 (0-2 same?): " << (uf.same(0, 2) ? "Yes" : "No") << " (Expected: Yes)" << std::endl;
    std::cout << "UF Test 2 (0-3 same?): " << (uf.same(0, 3) ? "Yes" : "No") << " (Expected: No)" << std::endl;
}

void test_component_manager()
{
    EdgeList F = {
        {0, 1}, {1, 2}, {2, 0}, // 1つ目の三角形の塊
        {10, 11},
        {11, 12},
        {12, 10} // 2つ目の離れた三角形の塊
    };

    ComponentManager cm;
    auto comps = cm.extract_components(F);

    std::cout << "--- ComponentManager Test ---" << std::endl;
    std::cout << "Number of components: " << comps.size() << " (Expected: 2)" << std::endl;

    for (const auto &c : comps)
    {
        std::cout << "Component " << c.id << ": " << c.nodes.size() << " nodes, " << c.edges.size() << " edges."
                  << std::endl;
    }
}

void test_bridge_calculation()
{
    PathManager pm;
    ComponentManager cm;

    // F: (0,0)付近の辺と、(2,2)付近の辺（距離は 2+2=4 離れているはず）
    EdgeList F = {
        {0, 1}, {1, 10}, {10, 9}, {9, 0}, // 2x2の左上四角 (ID: 0,1,9,10)
        {20, 21},
        {21, 30},
        {30, 29},
        {29, 20} // 少し離れた 2x2の四角
    };

    auto comps = cm.extract_components(F);
    auto bridges = cm.compute_all_bridges(comps, pm);

    std::cout << "--- Bridge Test ---" << std::endl;
    for (const auto &b : bridges)
    {
        std::cout << "Bridge between Comp " << b.comp1_id << " and " << b.comp2_id << ": Min Dist = " << b.dist
                  << std::endl;
        std::cout << "Number of optimal vertex pairs: " << b.best_pairs.size() << std::endl;
    }
}

void test_mst_brute_force()
{
    PathManager pm;
    ComponentManager cm;

    // 3つの離れた「点（1頂点のみのコンポーネント）」を定義
    // 島0: (0,0) -> ID 0
    // 島1: (2,0) -> ID 2
    // 島2: (0,2) -> ID 18 (2*9 + 0)
    EdgeList F = {{0, 0}, {2, 2}, {18, 18}}; // ダミーの辺で頂点を認識させる
    // ※実際には extract_components は「辺」を元にするので、
    // 分かりやすく3つの独立した辺のセットを与えます。
    EdgeList F_test = {{0, 9}, {2, 11}, {18, 19}};

    auto comps = cm.extract_components(F_test);
    std::cout << "Number of components: " << comps.size() << " (Expected: 3)" << std::endl;

    auto bridges = cm.compute_all_bridges(comps, pm);
    auto msts = cm.find_all_msts_brute_force(bridges, comps.size());

    std::cout << "--- MST Brute Force Test ---" << std::endl;
    std::cout << "Number of MST patterns found: " << msts.size() << std::endl;

    for (const auto &mst : msts)
    {
        std::cout << "Total Distance: " << mst.total_dist << " (Expected: 3)" << std::endl;
        std::cout << "Bridges used: ";
        for (int idx : mst.bridge_indices)
        {
            std::cout << "(" << bridges[idx].comp1_id << "-" << bridges[idx].comp2_id << ") ";
        }
        std::cout << std::endl;
    }
}