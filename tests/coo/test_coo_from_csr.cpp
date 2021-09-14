/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for creating COOs from CSRs with different
 * options
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int sym() {
    COO<> coo { 6, 6, 6, 5 };
    auto& x = coo.x(); auto& y = coo.y();

    size_t i = 0;
    x[i] = 5; y[i] = 2; ++i;
    x[i] = 5; y[i] = 1; ++i;
    x[i] = 5; y[i] = 0; ++i;
    x[i] = 3; y[i] = 5; ++i;
    x[i] = 5; y[i] = 3; ++i;

    CSR<> csr { coo };
    csr.sort();

    COO<int, int, int*, true> sym_coo { csr };

    EQ(sym_coo.n(), 6);
    EQ(sym_coo.m(), 10);

    auto& xn = sym_coo.x(); auto& yn = sym_coo.y();
    i = 0;
    EQ(xn[i], 5); EQ(yn[i++], 3);
    EQ(xn[i], 3); EQ(yn[i++], 5);
    EQ(xn[i], 0); EQ(yn[i++], 5);
    EQ(xn[i], 5); EQ(yn[i++], 0);
    EQ(xn[i], 1); EQ(yn[i++], 5);
    EQ(xn[i], 5); EQ(yn[i++], 1);
    EQ(xn[i], 2); EQ(yn[i++], 5);
    EQ(xn[i], 5); EQ(yn[i++], 2);
    EQ(xn[i], 3); EQ(yn[i++], 5);
    EQ(xn[i], 5); EQ(yn[i++], 3);

    sym_coo.free();
    csr.free();
    coo.free();

    return 0;
}

int sym_ut() {
    COO<> coo { 6, 6, 6, 5 };
    auto& x = coo.x(); auto& y = coo.y();

    size_t i = 0;
    x[i] = 5; y[i] = 2; ++i;
    x[i] = 5; y[i] = 1; ++i;
    x[i] = 5; y[i] = 0; ++i;
    x[i] = 3; y[i] = 5; ++i;
    x[i] = 5; y[i] = 3; ++i;

    CSR<> csr { coo };
    csr.sort();

    COO<int, int, int*, true, true> sym_coo { csr };

    EQ(sym_coo.n(), 6);
    EQ(sym_coo.m(), 5);

    auto& xn = sym_coo.x(); auto& yn = sym_coo.y();
    i = 0;
    EQ(xn[i], 3); EQ(yn[i++], 5);
    EQ(xn[i], 0); EQ(yn[i++], 5);
    EQ(xn[i], 1); EQ(yn[i++], 5);
    EQ(xn[i], 2); EQ(yn[i++], 5);
    EQ(xn[i], 3); EQ(yn[i++], 5);

    sym_coo.free();
    csr.free();
    coo.free();

    return 0;
}

int sl() {
    COO<> coo { 6, 6, 6, 5 };
    auto& x = coo.x(); auto& y = coo.y();

    size_t i = 0;
    x[i] = 5; y[i] = 2; ++i;
    x[i] = 5; y[i] = 1; ++i;
    x[i] = 5; y[i] = 0; ++i;
    x[i] = 3; y[i] = 5; ++i;
    x[i] = 5; y[i] = 3; ++i;

    CSR<> csr { coo };
    csr.sort();

    try {
        COO<int, int, int*, true, true, true> sym_coo { csr };
        EQ(1, 0);
        sym_coo.free();
    } catch(...) {
        EQ(1, 1);
    }

    csr.free();
    coo.free();

    return 0;
}

int ut() {
    COO<> coo { 6, 6, 6, 5 };
    auto& x = coo.x(); auto& y = coo.y();

    size_t i = 0;
    x[i] = 5; y[i] = 2; ++i;
    x[i] = 5; y[i] = 1; ++i;
    x[i] = 5; y[i] = 0; ++i;
    x[i] = 3; y[i] = 5; ++i;
    x[i] = 5; y[i] = 3; ++i;

    CSR<> csr { coo };
    csr.sort();

    try {
        COO<int, int, int*, false, true> sym_coo { csr };
        EQ(1, 0);
        sym_coo.free();
    } catch(...) {
        EQ(1, 1);
    }

    csr.free();
    coo.free();

    return 0;
}


int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    TEST(sym);
    TEST(sym_ut);
    TEST(sl);
    TEST(ut);

    return pass;
}

