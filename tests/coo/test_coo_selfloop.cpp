/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading COO files with and without self
 * loops
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int read_with_sl(string dir_path) {
    COO<int, int, vector<int>> c { dir_path + "/selfloop.el" };
    EQ(c.m(), 10);
    EQ(c.n(), 4);

    vector<int> valid_x = {0, 0, 0, 1, 1, 2, 2, 3, 3, 3};
    vector<int> valid_y = {1, 2, 3, 0, 1, 1, 2, 0, 2, 3};

    NOPRINT_EQ(valid_x, c.x());
    NOPRINT_EQ(valid_y, c.y());

    return 0;
}

int read_without_sl(string dir_path) {
    COO<int, int, vector<int>, false, false, true> c { dir_path + "/selfloop.el" };
    EQ(c.m(), 7);
    EQ(c.n(), 4);

    vector<int> valid_x = {0, 0, 0, 1, 2, 3, 3};
    vector<int> valid_y = {1, 2, 3, 0, 1, 0, 2};

    NOPRINT_EQ(valid_x, c.x());
    NOPRINT_EQ(valid_y, c.y());

    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_with_sl, dir_path);
    TEST(read_without_sl, dir_path);

    return pass;
}
