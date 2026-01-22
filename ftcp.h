#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

class Counter {
    int w;
    int mate_size;
    unsigned long long state_size;
    std::vector<std::vector<int>> tileConditon;
    std::uint32_t d_mask[49];
    std::uint32_t tile_4_edges[49];
    int cell_x[49];
    int cell_y[49];

  public:
    Counter(int width);
    void setTileCondition(int cell, int tile, int value);
    int get_binary_digit(unsigned long long n, int d);
    unsigned long long set_bit(unsigned long long n, int d, int v);
    int get_index(int dir, int x);
    bool can_put(int cell, int tile, unsigned long long mate);
    unsigned long long put(int cell, int take, unsigned long long mate);
    unsigned long long count();
    bool hasCP();
    std::string findCP();
    std::string to_str(int a);
    void setTileCondition(std::vector<std::vector<int>> &preEdges);
    std::string edges_to_cpstr(std::vector<int> &innerVerticesState);
};

void PrintMemoryUsage();
std::string GetExeDirectory();
void writeToFile(std::string output_path, std::string output_txt);
void print_edges_state(std::vector<int> edges);
std::string folds_to_cpstr(std::array<int, 32> &folds);