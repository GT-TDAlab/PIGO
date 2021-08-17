/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains tests for sorting CSRs
 */

#include "tests.hpp"
#include "pigo.hpp"
#include <vector>

using namespace std;
using namespace pigo;

int sort_small_vec(string dir_path) {
    CSR<int, int, vector<int>, vector<int>> sort { };
    CSR<int, int, vector<int>, vector<int>> orig { dir_path + "/base1.graph" };

    EQ(orig.n(), 10);
    EQ(orig.m(), 20);

    sort = orig;

    auto & o_end = orig.endpoints();
    auto & s_end = sort.endpoints();
    auto & o_off = orig.offsets();
    auto & s_off = sort.offsets();

    NOPRINT_EQ(o_end, s_end);
    NOPRINT_EQ(o_off, s_off);

    sort.sort();

    NOPRINT_NEQ(o_end, s_end);
    NOPRINT_EQ(o_off, s_off);

    EQ(s_end[0], 3);
    EQ(s_end[2], 5);
    std::swap(s_end[0], s_end[2]);

    NOPRINT_EQ(o_end, s_end);

    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(sort_small_vec, dir_path);

    return pass;
}

