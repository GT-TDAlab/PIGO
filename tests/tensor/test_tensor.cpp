/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for reading tensor files
 */

#include "tests.hpp"
#include "pigo.hpp"

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

    Tensor<uint64_t, uint64_t, shared_ptr<uint64_t>, double, shared_ptr<double>> r2 { ".test.out.ascii" };

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
        Tensor<uint64_t, uint64_t, shared_ptr<uint64_t>, double, shared_ptr<double>> r2 { ".test.out.bin" };
        EQ(1, 0);
    } catch(...) { }

    Tensor<int, int, shared_ptr<int>, float, vector<float>> r2 { ".test.out.bin" };

    EQ(r.order(), r2.order());
    EQ(r.m(), r2.m());
    for (int idx = 0; idx < r2.order()*r2.m(); ++idx)
        EQ(r.c()[idx], r2.c().get()[idx]);

    for (int idx = 0; idx < r2.m(); ++idx)
        FEQ(r.w()[idx], r2.w()[idx]);

    auto ml1 = r.max_labels();
    auto ml2 = r2.max_labels();
    NOPRINT_EQ(ml1, ml2);

    r.free();
    r2.free();
    return 0;
}

int no_weight(string dir_path) {
    Tensor<int, int, int*, float, float*, false> t { dir_path + "/noweight.tns" };
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

int max_labs(string dir_path) {
    Tensor<int,int,vector<int>,double,vector<double>> t { dir_path + "/test.tns" };

    EQ(t.m(), 5);
    EQ(t.order(), 4);
    auto labs = t.max_labels();
    EQ(labs.size(), 4);
    EQ(labs[0], 3); EQ(labs[1], 3); EQ(labs[2], 2); EQ(labs[3], 2);

    t.free();
    return 0;
}

int order_one_no_weight(string dir_path) {
    Tensor<int,int,shared_ptr<int>,float,float*,false> t { dir_path + "/ord1.tns" };
    EQ(t.m(), 20);
    EQ(t.order(), 1);
    int* c = t.c().get();
    EQ(c[0], 19);
    EQ(c[19], 18);
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
    TEST(max_labs, dir_path);
    TEST(order_one_no_weight, dir_path);

    return pass;
}
