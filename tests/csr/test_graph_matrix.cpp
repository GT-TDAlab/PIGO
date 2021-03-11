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
    SymMatrix<> m {dir_path + "/ba_100_14_1.el"};

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

int matrix(string dir_path) {
    Matrix<> m {dir_path + "/begin.el"};
    auto csr = m.csr();
    auto csc = m.csc();

    auto row_endpoints = csr.endpoints();
    auto row_offsets = csr.offsets();

    auto col_endpoints = csc.endpoints();
    auto col_offsets = csc.offsets();

    EQ(m.nrows(), 10);
    EQ(m.ncols(), 8);
    EQ(csr.m(), csc.m());

    EQ(row_offsets[1]-row_offsets[0], 3);
    EQ(col_offsets[1]-col_offsets[0], 0);

    EQ(row_endpoints[row_offsets[5]], 6);
    EQ(col_endpoints[col_offsets[7]], 3);

    m.free();
    return 0;
}

int digraph(string dir_path) {
    DiGraph<> d {dir_path + "/begin.el"};

    EQ(d.m(), 9);
    EQ(d.n(), 10);
    EQ(d.nrows(), 10);
    EQ(d.ncols(), 8);

    EQ(d.out().degree(0), 3);
    EQ(d.in().degree(0), 0);

    EQ(*(d.out().neighbors(5).begin()), 6);
    EQ(*(d.in().neighbors(7).begin()), 3);

    d.free();
    return 0;
}

int matrix_raw(string dir_path) {
    COO<> c {dir_path + "/begin.el"};
    Matrix<> m {c}; 
    auto csr = m.csr();
    auto csc = m.csc();

    auto row_endpoints = csr.endpoints();
    auto row_offsets = csr.offsets();

    auto col_endpoints = csc.endpoints();
    auto col_offsets = csc.offsets();

    EQ(m.nrows(), 10);
    EQ(m.ncols(), 8);
    EQ(csr.m(), csc.m());

    EQ(row_offsets[1]-row_offsets[0], 3);
    EQ(col_offsets[1]-col_offsets[0], 0);

    EQ(row_endpoints[row_offsets[5]], 6);
    EQ(col_endpoints[col_offsets[7]], 3);

    m.free();
    c.free();
    return 0;
}

int digraph_raw(string dir_path) {
    COO<> c {dir_path + "/begin.el"};
    DiGraph<> d {c}; 

    EQ(d.m(), 9);
    EQ(d.n(), 10);
    EQ(d.nrows(), 10);
    EQ(d.ncols(), 8);

    EQ(d.out().degree(0), 3);
    EQ(d.in().degree(0), 0);

    EQ(*(d.out().neighbors(5).begin()), 6);
    EQ(*(d.in().neighbors(7).begin()), 3);

    d.free();
    c.free();
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
    TEST(matrix, dir_path);
    TEST(digraph, dir_path);
    TEST(matrix_raw, dir_path);
    TEST(digraph_raw, dir_path);

    return pass;
}
