/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for reading tensor files
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <vector>

using namespace std;
using namespace pigo;

int read_tensor(string dir_path) {
    Tensor<> t { dir_path + "/test.tns" };
    EQ(t.order(), 4);
    EQ(t.m(), 5);
    auto& c = t.c();
    size_t idx = 0;
    EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 1);
    EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 2); EQ(c[idx++], 1);
    EQ(c[idx++], 2); EQ(c[idx++], 2); EQ(c[idx++], 2); EQ(c[idx++], 1);
    EQ(c[idx++], 2); EQ(c[idx++], 3); EQ(c[idx++], 1); EQ(c[idx++], 2);
    EQ(c[idx++], 3); EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 2);

    auto& w = t.w();
    idx = 0;
    FEQ(w[idx++], 1.0);
    FEQ(w[idx++], 2.0);
    FEQ(w[idx++], 3.0);
    FEQ(w[idx++], 4.0);
    FEQ(w[idx++], 5.0);

    t.free();
    return 0;
}

int write_tensor(string dir_path) {
    Tensor<int,int,vector<int>> r { dir_path + "/test.tns" };
    r.write(".test.out.ascii");

    Tensor<uint64_t, uint64_t, shared_ptr<uint64_t>, true, double, shared_ptr<double>> r2 { ".test.out.ascii" };

    EQ((uint64_t)r.order(), r2.order());
    EQ((uint64_t)r.m(), r2.m());
    for (size_t idx = 0; idx < r2.order()*r2.m(); ++idx)
        EQ((uint64_t)r.c()[idx], r2.c().get()[idx]);

    for (size_t idx = 0; idx < r2.m(); ++idx)
        FEQ((double)r.w()[idx], r2.w().get()[idx]);

    r.free();
    r2.free();
    return 0;
}

int write_bin(string dir_path) {
    Tensor<int,int,vector<int>> r { dir_path + "/test.tns" };
    r.save(".test.out.bin");

    try {
        Tensor<uint64_t, uint64_t, shared_ptr<uint64_t>, true, double, shared_ptr<double>> r2 { ".test.out.bin" };
        EQ(1, 0);
    } catch(...) { }

    Tensor<int, int, shared_ptr<int>, true, float, vector<float>> r2 { ".test.out.bin" };

    EQ(r.order(), r2.order());
    EQ(r.m(), r2.m());
    for (int idx = 0; idx < r2.order()*r2.m(); ++idx)
        EQ(r.c()[idx], r2.c().get()[idx]);

    for (int idx = 0; idx < r2.m(); ++idx)
        FEQ(r.w()[idx], r2.w()[idx]);

    r.free();
    r2.free();
    return 0;
}

int no_weight(string dir_path) {
    Tensor<int, int, int*, false> t { dir_path + "/noweight.tns" };
    EQ(t.order(), 4);
    EQ(t.m(), 5);
    auto& c = t.c();
    size_t idx = 0;
    EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 1);
    EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 2); EQ(c[idx++], 1);
    EQ(c[idx++], 2); EQ(c[idx++], 2); EQ(c[idx++], 2); EQ(c[idx++], 1);
    EQ(c[idx++], 2); EQ(c[idx++], 3); EQ(c[idx++], 1); EQ(c[idx++], 2);
    EQ(c[idx++], 3); EQ(c[idx++], 1); EQ(c[idx++], 1); EQ(c[idx++], 2);

    t.free();
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_tensor, dir_path);
    TEST(write_tensor, dir_path);
    TEST(write_bin, dir_path);
    TEST(no_weight, dir_path);

    return pass;
}
