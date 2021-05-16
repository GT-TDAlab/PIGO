/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for reading rectangular CSRs
 */

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

int rect_csr(string dir_path) {
    CSR<> g { dir_path + "/rect.graph" };
    EQ(g.nrows(), 3);
    EQ(g.ncols(), 6);
    g.free();
    return 0;
}

int rect_coo(string dir_path) {
    COO<> c { dir_path + "/rect.mtx" };
    EQ(c.nrows(), 3);
    EQ(c.ncols(), 6);
    CSR<> g { c };
    EQ(g.nrows(), 3);
    EQ(g.ncols(), 6);
    c.free();
    g.free();
    return 0;
}

int bigempty(string dir_path) {
    CSR<> g { dir_path + "/bigempty.mtx" };
    EQ(g.nrows(), 5001);
    EQ(g.ncols(), 5001);
    EQ(g.n(), 5001);
    EQ(g.m(), 1);

    g.free();
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(rect_csr, dir_path);
    TEST(rect_coo, dir_path);
    TEST(bigempty, dir_path);

    return pass;
}
