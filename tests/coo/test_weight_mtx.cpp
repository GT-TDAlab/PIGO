/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading weighted matrix market files
 */

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

int read_weight(string dir_path) {
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

    c.free();

    return 0;
}

int read_int_weight(string dir_path) {
    WCOO<int, int, shared_ptr<int>, int, shared_ptr<int>>
        c { dir_path + "/intweight.mtx" };

    EQ(c.m(), 5);
    EQ(c.n(), 6);

    auto x = c.x().get();
    auto y = c.y().get();
    auto w = c.w().get();

    size_t ctr = 0;
    EQ(x[ctr], 5); EQ(y[ctr], 1); EQ(w[ctr++], 4);
    EQ(x[ctr], 4); EQ(y[ctr], 1); EQ(w[ctr++], 1);
    EQ(x[ctr], 4); EQ(y[ctr], 2); EQ(w[ctr++], 3);
    EQ(x[ctr], 4); EQ(y[ctr], 1); EQ(w[ctr++], 12);
    EQ(x[ctr], 2); EQ(y[ctr], 1); EQ(w[ctr++], -10);

    c.free();

    return 0;
}

int read_uint_weight(string dir_path) {
    WCOO<int, int, shared_ptr<int>, uint64_t, shared_ptr<uint64_t>>
        c { dir_path + "/intweight.mtx" };

    EQ(c.m(), 5);
    EQ(c.n(), 6);

    auto x = c.x().get();
    auto y = c.y().get();
    auto w = c.w().get();

    size_t ctr = 0;
    EQ(x[ctr], 5); EQ(y[ctr], 1); EQ(w[ctr++], 4);
    EQ(x[ctr], 4); EQ(y[ctr], 1); EQ(w[ctr++], 1);
    EQ(x[ctr], 4); EQ(y[ctr], 2); EQ(w[ctr++], 3);
    EQ(x[ctr], 4); EQ(y[ctr], 1); EQ(w[ctr++], 12);
    EQ(x[ctr], 2); EQ(y[ctr], 1); EQ(w[ctr++], 10);

    c.free();

    return 0;
}

int save_load_weight(string dir_path) {
    WCOO<int, int, vector<int>,
        float, vector<float>> c { dir_path + "/weighted.mtx", MATRIX_MARKET };
    c.save(".weighted_coo.pigo");

    WCOO<int, int, vector<int>,
        float, vector<float>> c_load { ".weighted_coo.pigo" };

    // Ensure that the count is correct
    EQ(c.m(), c_load.m());
    EQ(c.n(), c_load.n());

    NOPRINT_EQ(c.x(), c_load.x());
    NOPRINT_EQ(c.y(), c_load.y());
    NOPRINT_EQ(c.w(), c_load.w());

    EQ(c_load.m(), 6);
    EQ(c_load.x()[4], 2); EQ(c_load.y()[4], 1); FEQ(c_load.w()[4], -0.04321);

    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_weight, dir_path);
    TEST(read_int_weight, dir_path);
    TEST(read_uint_weight, dir_path);
    TEST(save_load_weight, dir_path);

    return pass;
}

