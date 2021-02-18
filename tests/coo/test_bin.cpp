/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading and writing binary PIGO files
 */

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

int write_bin(string dir_path) {
    // Read a text COO and write it out to a binary
    COO<uint8_t, uint8_t> c { dir_path + "/clean.el", EDGE_LIST };
    c.save(".test.write_bin.pigo");

    // Next, re-read it and check for equivalence
    COO<uint8_t, uint8_t, vector<uint8_t>> c2 { ".test.write_bin.pigo" };
    EQ(c.n(), c2.n());
    EQ(c.m(), c2.m());

    auto cx = c.x(); auto cy = c.y();
    auto c2x = c2.x(); auto c2y = c2.y();

    for (size_t i = 0; i < c.m(); ++i) {
        EQ(cx[i], c2x[i]);
        EQ(cy[i], c2y[i]);
    }

    c.free();

    return 0;
}

int read_bin(string dir_path) {
    // Read a binary file
    COO<uint8_t, uint8_t> bin { dir_path + "/clean.bin" };

    // Read the known source file
    COO<uint8_t, uint8_t> el { dir_path + "/clean.el" };

    // Compare
    EQ(bin.m(), el.m());
    EQ(bin.n(), el.n());

    auto bx = bin.x(); auto by = bin.y();
    auto ex = el.x(); auto ey = el.y();

    for (size_t i = 0; i < bin.m(); ++i) {
        EQ(bx[i], ex[i]);
        EQ(by[i], ey[i]);
    }

    bin.free();
    el.free();

    return 0;
}

int fail_diff_bin(string dir_path) {
    // Read a binary file
    try {
        COO<uint16_t, uint8_t, shared_ptr<uint16_t>> bin { dir_path + "/clean.bin" };
        EQ(1, 0);
    } catch(Error&) { }
    try {
        COO<uint8_t, uint16_t, shared_ptr<uint8_t>> bin { dir_path + "/clean.bin" };
        EQ(1, 0);
    } catch(Error&) { }
    try {
        COO<uint32_t, uint32_t, shared_ptr<uint32_t>> bin { dir_path + "/clean.bin" };
        EQ(1, 0);
    } catch(Error&) { }
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(write_bin, dir_path);
    TEST(read_bin, dir_path);
    TEST(fail_diff_bin, dir_path);

    return pass;
}
