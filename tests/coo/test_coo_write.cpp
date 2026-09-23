/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for writing COOs
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <cstdint>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace pigo;

/** Reads a whole file back as a string, for byte-exact output checks. */
static string read_whole(const string& fn) {
    ifstream f { fn.c_str(), ios::in | ios::binary };
    ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

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

int neg_weight(string dir_path) {
    WCOOPtr<int, size_t, int> coo { dir_path + "/intweight.mtx" };

    coo.write(".tmp.test_coo_write.out");

    // Confirm the write by reading
    COO<> c_no_w {".tmp.test_coo_write.out"};
    EQ((size_t)c_no_w.m(), coo.m());
    EQ((int)c_no_w.n(), coo.n());
    c_no_w.free();

    WCOO<int, size_t, vector<int>, int, vector<int>> coo_r { ".tmp.test_coo_write.out" };
    EQ(coo_r.m(), coo.m());
    for (size_t e = 0; e < coo.m(); ++e) {
        EQ(coo.x()[e], coo_r.x()[e]);
        EQ(coo.y()[e], coo_r.y()[e]);
        EQ(coo.w()[e], coo_r.w()[e]);
    }

    coo.free();

    return 0;
}

/** Writing the most-negative signed value must emit its correct decimal.
 *
 * get_positive(T_MIN) used to negate within the signed type, which overflows
 * back to a negative value (int8_t -128 -> -128). write_ascii then took
 * `obj % 10` of a negative number -- emitting non-digit bytes -- and derived a
 * digit count that disagreed with the sizing pass, so the file was both
 * corrupt and, in bulk, overrun. Here the write is checked byte-for-byte.
 */
int signed_min_ascii(string) {
    // int8_t: -128 is the most-negative value; -1 exercises an ordinary
    // negative alongside it.
    COO<int8_t, int, vector<int8_t> > coo(1, 1, 2, 2);
    coo.x()[0] = (int8_t)-128; coo.y()[0] = (int8_t)-128;
    coo.x()[1] = (int8_t)-1;   coo.y()[1] = (int8_t)-1;

    coo.write(".tmp.test_coo_write_negmin.out");
    EQ(read_whole(".tmp.test_coo_write_negmin.out"), string("-128 -128\n-1 -1\n"));
    coo.free();

    // int16_t and int32_t most-negative values, read back through PIGO.
    COO<int32_t, int, vector<int32_t> > c32(1, 1, 2, 2);
    c32.x()[0] = (int32_t)(-2147483647 - 1); c32.y()[0] = 0;
    c32.x()[1] = -1;                         c32.y()[1] = 1;
    c32.write(".tmp.test_coo_write_negmin32.out");
    EQ(read_whole(".tmp.test_coo_write_negmin32.out"),
       string("-2147483648 0\n-1 1\n"));
    c32.free();

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
    TEST(neg_weight, dir_path)
    TEST(signed_min_ascii, dir_path)

    return pass;
}
