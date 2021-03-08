/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading COO files in a symmetric mode
 */

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

int read_clean(string dir_path) {
    COO<int, int, int*, true> c { dir_path + "/clean.el" };

    // Ensure that the count is correct
    EQ(c.m(), 14);
    EQ(c.n(), 10);

    // Ensure the values are correct
    auto x = c.x();
    auto y = c.y();

    size_t pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(y[pos], 0); EQ(x[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(y[pos], 1); EQ(x[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(y[pos], 2); EQ(x[pos++], 3);
    EQ(x[pos], 3); EQ(y[pos++], 4);
    EQ(y[pos], 3); EQ(x[pos++], 4);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(y[pos], 4); EQ(x[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(y[pos], 5); EQ(x[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);
    EQ(y[pos], 2); EQ(x[pos++], 9);

    // Cleanup
    c.free();
    return 0;
}

int read_with_sl(string dir_path) {
    COO<int, int, vector<int>, true> c { dir_path + "/selfloop.el" };
    EQ(c.m(), 20);
    EQ(c.n(), 4);

    vector<int> valid_x = {0,1, 0,2, 0,3, 1,0, 1,1, 2,1, 2,2, 3,0, 3,2, 3,3};
    vector<int> valid_y = {1,0, 2,0, 3,0, 0,1, 1,1, 1,2, 2,2, 0,3, 2,3, 3,3};

    NOPRINT_EQ(valid_x, c.x());
    NOPRINT_EQ(valid_y, c.y());

    return 0;
}

int read_without_sl(string dir_path) {
    COO<int, int, vector<int>, true, false, true> c { dir_path + "/selfloop.el" };
    EQ(c.m(), 14);
    EQ(c.n(), 4);

    vector<int> valid_x = {0,1, 0,2, 0,3, 1,0, 2,1, 3,0, 3,2};
    vector<int> valid_y = {1,0, 2,0, 3,0, 0,1, 1,2, 0,3, 2,3};

    NOPRINT_EQ(valid_x, c.x());
    NOPRINT_EQ(valid_y, c.y());

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

    TEST(read_clean, dir_path);
    TEST(read_with_sl, dir_path);
    TEST(read_without_sl, dir_path);

    return pass;
}

