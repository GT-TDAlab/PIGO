/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 * Copyright (c) 2023 Kasimir Gabert
 *
 * This contains test files for reading tensor files
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <cstdio>
#include <fstream>
#include <string>

using namespace std;
using namespace pigo;

/** Writes contents to fn verbatim, with no newline appended
 *
 * Tests for missing-trailing-newline handling have to build their input at
 * runtime: git and most editors normalize a final newline onto checked-in
 * files, so a fixture could not be relied on to keep its exact last byte.
 */
static void write_exact(const string& fn, const string& contents) {
    ofstream f { fn.c_str(), ios::out | ios::binary | ios::trunc };
    f.write(contents.c_str(), contents.size());
}

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

    {
        ROFile ro {".test.out.bin"};
        string hdr_str { Tensor<>::tensor_file_header };
        size_t hdr_size = hdr_str.size() + sizeof(uint8_t)*3 + sizeof(int)*2;
        size_t dat_size = sizeof(int)*r.order() + sizeof(float);
        EQ(ro.size(), hdr_size+dat_size*r.m());
    }

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

/** An unweighted tensor file whose last line has no trailing newline reads
 * identically to one that does.
 *
 * Regression test: the counting pass used to treat "no input remaining" as
 * a failed read and skip the final entry, while the setting pass had already
 * written part of it. That undersized the allocation and overflowed it --
 * a heap-buffer-overflow, and an m() one too large.
 */
int read_no_trailing_newline(string) {
    const string body = "0 1 500 1600000000\n1 2 700 1600000060\n2 3 900 1600000120";
    write_exact(".test.tns_no_trail.tns", body);
    write_exact(".test.tns_with_trail.tns", body + "\n");

    typedef Tensor<uint64_t, uint64_t, vector<uint64_t>, float, vector<float>,
            false> UT;
    UT without { ".test.tns_no_trail.tns", EDGE_LIST };
    UT with { ".test.tns_with_trail.tns", EDGE_LIST };

    EQ(without.order(), 4);
    EQ(without.m(), 3);
    EQ(with.order(), without.order());
    EQ(with.m(), without.m());

    // The final entry must be present and correct, not truncated or garbage
    EQ(without.c()[8], 2);
    EQ(without.c()[9], 3);
    EQ(without.c()[10], 900);
    EQ(without.c()[11], 1600000120);

    for (size_t i = 0; i < without.order()*without.m(); ++i)
        EQ(without.c()[i], with.c()[i]);

    remove(".test.tns_no_trail.tns");
    remove(".test.tns_with_trail.tns");
    return 0;
}

/** As above for order-1 unweighted, and for the weighted path */
int read_no_trailing_newline_variants(string) {
    write_exact(".test.tns_ord1.tns", "7\n8\n9");
    Tensor<uint64_t, uint64_t, vector<uint64_t>, float, vector<float>, false>
        ord1 { ".test.tns_ord1.tns", EDGE_LIST };
    EQ(ord1.order(), 1);
    EQ(ord1.m(), 3);
    EQ(ord1.c()[0], 7);
    EQ(ord1.c()[2], 9);

    // The weighted path was already correct; keep it covered so it stays so
    write_exact(".test.tns_wgt.tns", "1 1 1 2.5\n2 2 2 3.5");
    Tensor<uint64_t, uint64_t, vector<uint64_t>, double, vector<double> >
        wgt { ".test.tns_wgt.tns", EDGE_LIST };
    EQ(wgt.order(), 3);
    EQ(wgt.m(), 2);
    EQ(wgt.c()[5], 2);
    FEQ(wgt.w()[1], 3.5);

    remove(".test.tns_ord1.tns");
    remove(".test.tns_wgt.tns");
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
    TEST(read_no_trailing_newline, dir_path);
    TEST(read_no_trailing_newline_variants, dir_path);

    return pass;
}
