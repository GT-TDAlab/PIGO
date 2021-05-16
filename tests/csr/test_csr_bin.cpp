/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for reading and writing CSR binary files
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int write_bin(string dir_path) {
    // Create a CSR first
    CSR<uint32_t, uint32_t, shared_ptr<uint32_t>,
        shared_ptr<uint32_t>>
        write { dir_path + "/ba_100_14_1.el" };

    EQ(write.n(), 100);
    EQ(write.m(), 1204);

    write.save(".test.csr_bin.pigo");

    // Now, read it and compare
    CSR<uint32_t, uint32_t, vector<uint32_t>>
        read { ".test.csr_bin.pigo" };

    EQ(read.n(), write.n());
    EQ(read.m(), write.m());

    auto we = write.endpoints().get();
    auto re = read.endpoints();
    auto wo = write.offsets().get();
    auto ro = read.offsets();

    for (size_t v = 0; v < read.n()+1; ++v)
        EQ(wo[v], ro[v]);

    // Note: this will free offsets, but not yet endpoints
    // Not recommended for any real use, but tests a partial
    // vector/partial raw pointer construction on line 29
    read.free();

    for (size_t e = 0; e < read.m(); ++e)
        EQ(we[e], re[e]);

    return 0;
}

int nrows(string dir_path) {
    // Create a CSR first
    CSR<uint32_t, uint32_t, shared_ptr<uint32_t>,
        shared_ptr<uint32_t>>
        write { dir_path + "/rect.mtx" };

    EQ(write.n(), 6);
    EQ(write.m(), 7);
    EQ(write.nrows(), 3);
    EQ(write.ncols(), 6);

    write.save(".test.csr_bin.pigo");

    // Now, read it and compare
    CSR<uint32_t, uint32_t, vector<uint32_t>>
        read { ".test.csr_bin.pigo" };

    EQ(read.n(), write.n());
    EQ(read.m(), write.m());
    EQ(read.nrows(), write.nrows());
    EQ(read.ncols(), write.ncols());

    auto we = write.endpoints().get();
    auto re = read.endpoints();
    auto wo = write.offsets().get();
    auto ro = read.offsets();

    for (size_t v = 0; v < read.n()+1; ++v)
        EQ(wo[v], ro[v]);

    // Note: this will free offsets, but not yet endpoints
    // Not recommended for any real use, but tests a partial
    // vector/partial raw pointer construction on line 29
    read.free();

    for (size_t e = 0; e < read.m(); ++e)
        EQ(we[e], re[e]);

    write.free();
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
    TEST(nrows, dir_path);

    return pass;
}
