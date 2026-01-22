#ifndef FOLDSTOEDGES_H
#define FOLDSTOEDGES_H

#include <array>
#include <vector>
using namespace std;

vector<int> create_edges_by_folds(array<int, 32> &folds);
array<int, 398> create_edges_by_folds_arr(array<int, 32> &folds);

int get_edge_from_fold(int f, int n);

#endif // FOLDSTOEDGES_H