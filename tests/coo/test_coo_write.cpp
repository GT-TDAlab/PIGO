/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for writing COOs
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int use_shared_ptr(string dir_path) {
    COO<int, int, shared_ptr<int>> coo { dir_path + "/with-comments.el" };

    coo.write(".tmp.test_coo_write.out");

    // Ensure that the file size is correct
    File f { ".tmp.test_coo_write.out", READ };
    EQ(f.size(), 48);

    coo.free();
    return 0;
}

int use_def(string dir_path) {
    COO<> coo { dir_path + "/with-comments.el" };

    coo.write(".tmp.test_coo_write.out");

    // Ensure that the file size is correct
    File f { ".tmp.test_coo_write.out", READ };
    EQ(f.size(), 48);

    coo.free();
    return 0;
}

int weights(string dir_path) {
    COO<int, int, vector<int>, false, false, false, true, double, vector<double>> coo { dir_path + "/weighted.mtx" };

    coo.write(".tmp.test_coo_write.out");

    // Confirm the write by reading
    COO<> c_no_w {".tmp.test_coo_write.out"};
    EQ((int)c_no_w.m(), coo.m());
    EQ((int)c_no_w.n(), coo.n());
    c_no_w.free();

    COO<int, int, vector<int>, false, false, false, true, double, vector<double>> coo_r { ".tmp.test_coo_write.out" };
    EQ(coo_r.m(), coo.m());
    for (int e = 0; e < coo.m(); ++e) {
        EQ(coo.x()[e], coo_r.x()[e]);
        EQ(coo.y()[e], coo_r.y()[e]);
        FEQD(coo.w()[e], coo_r.w()[e], 1e-6);
    }

    coo.free();
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(use_shared_ptr, dir_path);
    TEST(use_def, dir_path);
    TEST(weights, dir_path);

    return pass;
}
