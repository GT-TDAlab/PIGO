/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for transposing COOs
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int read_copy(string dir_path) {
    COO<> c { dir_path + "/clean.el" };

    // Ensure that the count is correct
    EQ(c.m(), 7);
    EQ(c.n(), 10);

    auto x = c.x();
    auto y = c.y();

    size_t pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 3); EQ(y[pos++], 4);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    // Create a copy
    COO<> c2 = c;

    c2.transpose();

    auto x2 = c2.x();
    auto y2 = c2.y();

    x = c.x();
    y = c.y();

    x[3] = 0;
    y[3] = 1;

    pos = 0;
    EQ(y2[pos], 0); EQ(x2[pos++], 1);
    EQ(y2[pos], 1); EQ(x2[pos++], 2);
    EQ(y2[pos], 2); EQ(x2[pos++], 3);
    EQ(y2[pos], 3); EQ(x2[pos++], 4);
    EQ(y2[pos], 4); EQ(x2[pos++], 5);
    EQ(y2[pos], 5); EQ(x2[pos++], 0);
    EQ(y2[pos], 2); EQ(x2[pos++], 9);

    pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    c.free();
    c2.free();

    return 0;
}

int read_vec_trans(string dir_path) {
    COO<uint32_t, uint32_t, vector<uint32_t> >
        c { dir_path + "/clean.el" };

    c.transpose();

    // Ensure that the count is correct
    EQ(c.m(), 7);
    EQ(c.n(), 10);

    auto x = c.x();
    auto y = c.y();

    EQ(x.size(), 7);
    EQ(y.size(), 7);

    size_t pos = 0;
    EQ(y[pos], 0); EQ(x[pos++], 1);
    EQ(y[pos], 1); EQ(x[pos++], 2);
    EQ(y[pos], 2); EQ(x[pos++], 3);
    EQ(y[pos], 3); EQ(x[pos++], 4);
    EQ(y[pos], 4); EQ(x[pos++], 5);
    EQ(y[pos], 5); EQ(x[pos++], 0);
    EQ(y[pos], 2); EQ(x[pos++], 9);

    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_copy, dir_path);
    TEST(read_vec_trans, dir_path);

    return pass;
}

