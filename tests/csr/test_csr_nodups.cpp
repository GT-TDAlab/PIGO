/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains tests for reading CSRs and removing multi-edges
 */

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

int remove_multedge(string dir_path) {
    CSR<> csr {dir_path + "/dupedge.el"};

    EQ(csr.m(), 11);
    EQ(csr.n(), 9);

    auto nodups = csr.new_csr_without_dups();

    csr.free();

    EQ(nodups.m(), 10);
    EQ(nodups.n(), 9);

    auto e = nodups.endpoints();
    auto o = nodups.offsets();

    size_t ctr = 0;
    EQ(o[ctr++], 0);
    EQ(o[ctr++], 2);
    EQ(o[ctr++], 2);
    EQ(o[ctr++], 5);
    EQ(o[ctr++], 6);
    EQ(o[ctr++], 8);
    EQ(o[ctr++], 9);
    EQ(o[ctr++], 10);
    EQ(o[ctr++], 10);

    ctr = 0;
    EQ(e[ctr++], 1);
    EQ(e[ctr++], 8);
    EQ(e[ctr++], 3);
    EQ(e[ctr++], 4);
    EQ(e[ctr++], 5);
    EQ(e[ctr++], 3);
    EQ(e[ctr++], 1);
    EQ(e[ctr++], 5);
    EQ(e[ctr++], 2);
    EQ(e[ctr++], 8);

    nodups.free();

    return 0;
}

int remove_multedge_sptr(string dir_path) {
    CSR<int64_t, int64_t> csr {dir_path + "/dupedge.el"};

    EQ(csr.m(), 11);
    EQ(csr.n(), 9);

    auto nodups = csr.new_csr_without_dups<uint32_t, uint32_t, shared_ptr<uint32_t>, shared_ptr<uint32_t>>();

    csr.free();

    EQ(nodups.m(), 10);
    EQ(nodups.n(), 9);

    auto e = nodups.endpoints().get();
    auto o = nodups.offsets().get();

    size_t ctr = 0;
    EQ(o[ctr++], 0);
    EQ(o[ctr++], 2);
    EQ(o[ctr++], 2);
    EQ(o[ctr++], 5);
    EQ(o[ctr++], 6);
    EQ(o[ctr++], 8);
    EQ(o[ctr++], 9);
    EQ(o[ctr++], 10);
    EQ(o[ctr++], 10);

    ctr = 0;
    EQ(e[ctr++], 1);
    EQ(e[ctr++], 8);
    EQ(e[ctr++], 3);
    EQ(e[ctr++], 4);
    EQ(e[ctr++], 5);
    EQ(e[ctr++], 3);
    EQ(e[ctr++], 1);
    EQ(e[ctr++], 5);
    EQ(e[ctr++], 2);
    EQ(e[ctr++], 8);

    nodups.free();

    return 0;
}

int remove_multedge_wgt(string dir_path) {
    WCSRPtr<int, size_t, double> csr {dir_path + "/dupedge.el"};

    EQ(csr.m(), 11);
    EQ(csr.n(), 9);

    auto nodups = csr.new_csr_without_dups<uint32_t, uint32_t, shared_ptr<uint32_t>, shared_ptr<uint32_t>, true, float, shared_ptr<float>>();

    csr.free();

    EQ(nodups.m(), 10);
    EQ(nodups.n(), 9);

    auto e = nodups.endpoints().get();
    auto o = nodups.offsets().get();
    auto w = nodups.weights().get();

    size_t ctr = 0;
    EQ(o[ctr++], 0);
    EQ(o[ctr++], 2);
    EQ(o[ctr++], 2);
    EQ(o[ctr++], 5);
    EQ(o[ctr++], 6);
    EQ(o[ctr++], 8);
    EQ(o[ctr++], 9);
    EQ(o[ctr++], 10);
    EQ(o[ctr++], 10);

    ctr = 0;
    EQ(e[ctr++], 1);
    EQ(e[ctr++], 8);
    EQ(e[ctr++], 3);
    EQ(e[ctr++], 4);
    EQ(e[ctr++], 5);
    EQ(e[ctr++], 3);
    EQ(e[ctr++], 1);
    EQ(e[ctr++], 5);
    EQ(e[ctr++], 2);
    EQ(e[ctr++], 8);

    ctr = 0;
    FEQ(w[ctr], 0.2f); ++ctr;
    FEQ(w[ctr], 0.1f); ++ctr;
    FEQ(w[ctr], 0.6f); ++ctr;
    FEQ(w[ctr], 0.5f); ++ctr;
    FEQ(w[ctr], 0.4f); ++ctr;
    FEQ(w[ctr], -0.2f); ++ctr;
    FEQ(w[ctr], 0.3f); ++ctr;
    FEQ(w[ctr], 0.7f); ++ctr;
    FEQ(w[ctr], -0.1f); ++ctr;
    FEQ(w[ctr], 0.8f); ++ctr;

    nodups.free();

    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(remove_multedge, dir_path);
    TEST(remove_multedge_sptr, dir_path);
    TEST(remove_multedge_wgt, dir_path);

    return pass;
}
