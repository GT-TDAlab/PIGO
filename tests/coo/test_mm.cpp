/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading matrix market COO files
 */

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

int read_simple(string dir_path) {
    COO<> c { dir_path + "/sparse.mtx", MATRIX_MARKET };

    // Ensure that the count is correct
    EQ(c.m(), 3);
    EQ(c.n(), 4);
    EQ(c.nrows(), 3);
    EQ(c.ncols(), 4);

    auto x = c.x();
    auto y = c.y();

    size_t ctr = 0;
    EQ(x[ctr], 1); EQ(y[ctr++], 3);
    EQ(x[ctr], 2); EQ(y[ctr++], 3);
    EQ(x[ctr], 2); EQ(y[ctr++], 1);

    c.free();

    return 0;
}

int fail_bad_label(string dir_path) {
    try {
        COO<> c { dir_path + "/sparse-bad.mtx", MATRIX_MARKET };
        EQ(0, 1);
    } catch (Error &e) {
    }

    try {
        COO<> c { dir_path + "/partial.mtx", MATRIX_MARKET };
        EQ(0, 1);
    } catch (Error &e) {
    }
    return 0;
}

int fail_read_dense(string dir_path) {
    try {
        COO<> c { dir_path + "/dense.mtx", MATRIX_MARKET };
        EQ(0, 1);
    } catch (NotYetImplemented &e) {
    }
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_simple, dir_path);
    TEST(fail_bad_label, dir_path);
    TEST(fail_read_dense, dir_path);

    return pass;
}
