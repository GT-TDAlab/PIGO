/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for copying COOs
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

    EQ(c2.m(), 7);
    EQ(c2.n(), 10);

    auto x2 = c2.x();
    auto y2 = c2.y();

    x[3] = 0;
    y[3] = 0;

    pos = 0;
    EQ(x2[pos], 0); EQ(y2[pos++], 1);
    EQ(x2[pos], 1); EQ(y2[pos++], 2);
    EQ(x2[pos], 2); EQ(y2[pos++], 3);
    EQ(x2[pos], 3); EQ(y2[pos++], 4);
    EQ(x2[pos], 4); EQ(y2[pos++], 5);
    EQ(x2[pos], 5); EQ(y2[pos++], 0);
    EQ(x2[pos], 2); EQ(y2[pos++], 9);

    pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 0); EQ(y[pos++], 0);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    c.free();
    c2.free();

    return 0;
}

int read_copy_assign(string dir_path) {
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
    COO<> c2;
    c2 = c;

    EQ(c2.m(), 7);
    EQ(c2.n(), 10);

    auto x2 = c2.x();
    auto y2 = c2.y();

    x[3] = 0;
    y[3] = 0;

    pos = 0;
    EQ(x2[pos], 0); EQ(y2[pos++], 1);
    EQ(x2[pos], 1); EQ(y2[pos++], 2);
    EQ(x2[pos], 2); EQ(y2[pos++], 3);
    EQ(x2[pos], 3); EQ(y2[pos++], 4);
    EQ(x2[pos], 4); EQ(y2[pos++], 5);
    EQ(x2[pos], 5); EQ(y2[pos++], 0);
    EQ(x2[pos], 2); EQ(y2[pos++], 9);

    pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 0); EQ(y[pos++], 0);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    c.free();
    c2.free();

    return 0;
}

int read_copy_vec(string dir_path) {
    COO<int, int, vector<int>> c { dir_path + "/clean.el" };

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
    COO<int, int, vector<int>> c2 = c;

    EQ(c2.m(), 7);
    EQ(c2.n(), 10);

    auto x2 = c2.x();
    auto y2 = c2.y();

    x[3] = 0;
    y[3] = 0;

    pos = 0;
    EQ(x2[pos], 0); EQ(y2[pos++], 1);
    EQ(x2[pos], 1); EQ(y2[pos++], 2);
    EQ(x2[pos], 2); EQ(y2[pos++], 3);
    EQ(x2[pos], 3); EQ(y2[pos++], 4);
    EQ(x2[pos], 4); EQ(y2[pos++], 5);
    EQ(x2[pos], 5); EQ(y2[pos++], 0);
    EQ(x2[pos], 2); EQ(y2[pos++], 9);

    pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 0); EQ(y[pos++], 0);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    c.free();
    c2.free();

    return 0;
}

int read_copy_sptr(string dir_path) {
    COO<int, int, shared_ptr<int>> c { dir_path + "/clean.el" };

    // Ensure that the count is correct
    EQ(c.m(), 7);
    EQ(c.n(), 10);

    auto x = c.x().get();
    auto y = c.y().get();

    size_t pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 3); EQ(y[pos++], 4);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    // Create a copy
    COO<int, int, shared_ptr<int>> c2 = c;

    EQ(c2.m(), 7);
    EQ(c2.n(), 10);

    auto x2 = c2.x().get();
    auto y2 = c2.y().get();

    x[3] = 0;
    y[3] = 0;

    pos = 0;
    EQ(x2[pos], 0); EQ(y2[pos++], 1);
    EQ(x2[pos], 1); EQ(y2[pos++], 2);
    EQ(x2[pos], 2); EQ(y2[pos++], 3);
    EQ(x2[pos], 3); EQ(y2[pos++], 4);
    EQ(x2[pos], 4); EQ(y2[pos++], 5);
    EQ(x2[pos], 5); EQ(y2[pos++], 0);
    EQ(x2[pos], 2); EQ(y2[pos++], 9);

    pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 0); EQ(y[pos++], 0);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    c.free();
    c2.free();

    return 0;
}

int copy_weight(string dir_path) {
    WCOO<int, int, shared_ptr<int>,
        double, vector<double>> c { dir_path + "/weighted.mtx", MATRIX_MARKET };

    // Ensure that the count is correct
    EQ(c.m(), 6);
    EQ(c.n(), 6);

    auto x = c.x().get();
    auto y = c.y().get();
    auto w = c.w();

    size_t ctr = 0;
    EQ(x[ctr], 5); EQ(y[ctr], 1); FEQ(w[ctr++], 4.3333);
    EQ(x[ctr], 4); EQ(y[ctr], 1); FEQ(w[ctr++], 0.00003454);
    EQ(x[ctr], 4); EQ(y[ctr], 2); FEQ(w[ctr++], 3);
    EQ(x[ctr], 4); EQ(y[ctr], 1); FEQ(w[ctr++], 1e10);
    EQ(x[ctr], 2); EQ(y[ctr], 1); FEQ(w[ctr++], -0.04321);
    EQ(x[ctr], 3); EQ(y[ctr], 1); FEQ(w[ctr++], -6.6E-10);

    auto c2 = c;

    auto x2 = c2.x().get();
    auto y2 = c2.y().get();
    auto w2 = c2.w();

    x[3] = 0;
    y[3] = 0;
    w[3] = 0.;

    ctr = 0;
    EQ(x2[ctr], 5); EQ(y2[ctr], 1); FEQ(w2[ctr++], 4.3333);
    EQ(x2[ctr], 4); EQ(y2[ctr], 1); FEQ(w2[ctr++], 0.00003454);
    EQ(x2[ctr], 4); EQ(y2[ctr], 2); FEQ(w2[ctr++], 3);
    EQ(x2[ctr], 4); EQ(y2[ctr], 1); FEQ(w2[ctr++], 1e10);
    EQ(x2[ctr], 2); EQ(y2[ctr], 1); FEQ(w2[ctr++], -0.04321);
    EQ(x2[ctr], 3); EQ(y2[ctr], 1); FEQ(w2[ctr++], -6.6E-10);

    ctr = 0;
    EQ(x[ctr], 5); EQ(y[ctr], 1); FEQ(w[ctr++], 4.3333);
    EQ(x[ctr], 4); EQ(y[ctr], 1); FEQ(w[ctr++], 0.00003454);
    EQ(x[ctr], 4); EQ(y[ctr], 2); FEQ(w[ctr++], 3);
    EQ(x[ctr], 0); EQ(y[ctr], 0); FEQ(w[ctr++], 0.);
    EQ(x[ctr], 2); EQ(y[ctr], 1); FEQ(w[ctr++], -0.04321);
    EQ(x[ctr], 3); EQ(y[ctr], 1); FEQ(w[ctr++], -6.6E-10);

    c.free();
    c2.free();

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
    TEST(read_copy_assign, dir_path);
    TEST(read_copy_vec, dir_path);
    TEST(read_copy_sptr, dir_path);
    TEST(copy_weight, dir_path);

    return pass;
}
