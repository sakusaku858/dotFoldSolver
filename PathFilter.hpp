#ifndef PATH_FILTER_HPP
#define PATH_FILTER_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

using Node = int;
using Path = std::vector<Node>;

class PathFilter
{
    std::vector<Path> all_paths;
    std::vector<Path> filtered_paths;

    bool must_turn_at_intersection(const Path &path);
    bool has_right_turns_excess_4(const Path &path);
    bool check_distance_constraint(const Path &path);
    Path rotate_path(const Path &path, int offset);

public:
    void load_path();
    void filter();
};

#endif