/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for using the Graph and Matrix names
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>
#include <algorithm>

using namespace std;
using namespace pigo;

int gm_back_forth(string dir_path) {
    Graph g {dir_path + "/ba_100_14_1.el"};
    Matrix m {dir_path + "/ba_100_14_1.el"};

    // Compare
    EQ(g.m(), m.m());
    EQ(g.n(), m.n());

    // Cleanup
    g.free();
    m.free();
    return 0;
}

int read_neighs(string dir_path) {
    Graph g {dir_path + "/ba_100_14_1.el"};

    size_t ctr = 0;
    for (auto v : g.neighbors(93)) {
        if (v != 95 && v != 96) EQ(0,1);
        if (++ctr > 2) EQ(0,1);
    }

    g.free();
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(gm_back_forth, dir_path);
    TEST(read_neighs, dir_path);

    return pass;
}

