/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for reading native CSR ASCII files
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>

using namespace std;
using namespace pigo;

int read_mval(string dir_path) {
    CSR<> csr {dir_path + "/triangle2m.graph"};

    // Verify the contents
    EQ(csr.n(), 3);
    EQ(csr.m(), 6);

    auto end = csr.endpoints();
    auto off = csr.offsets();

    size_t v = 0;
    EQ(off[v++], 0);
    EQ(off[v++], 2);
    EQ(off[v++], 4);
    EQ(off[v++], 6);

    size_t e = 0;
    EQ(end[e++], 1);
    EQ(end[e++], 2);
    EQ(end[e++], 0);
    EQ(end[e++], 2);
    EQ(end[e++], 0);
    EQ(end[e++], 1);

    csr.free();
    return 0;
}

int read_graph(string dir_path) {
    CSR<> csr {dir_path + "/triangle.graph"};

    // Verify the contents
    EQ(csr.n(), 3);
    EQ(csr.m(), 6);

    auto end = csr.endpoints();
    auto off = csr.offsets();

    size_t v = 0;
    EQ(off[v++], 0);
    EQ(off[v++], 2);
    EQ(off[v++], 4);
    EQ(off[v++], 6);

    size_t e = 0;
    EQ(end[e++], 1);
    EQ(end[e++], 2);
    EQ(end[e++], 0);
    EQ(end[e++], 2);
    EQ(end[e++], 0);
    EQ(end[e++], 1);

    csr.free();
    return 0;
}

int read_graph_messy(string dir_path) {
    CSR<> csr {dir_path + "/triangle-messy.graph"};

    // Verify the contents
    EQ(csr.n(), 3);
    EQ(csr.m(), 6);

    auto end = csr.endpoints();
    auto off = csr.offsets();

    size_t v = 0;
    EQ(off[v++], 0);
    EQ(off[v++], 2);
    EQ(off[v++], 4);
    EQ(off[v++], 6);

    size_t e = 0;
    EQ(end[e++], 1);
    EQ(end[e++], 2);
    EQ(end[e++], 0);
    EQ(end[e++], 2);
    EQ(end[e++], 0);
    EQ(end[e++], 1);

    csr.free();
    return 0;
}

int fail_graph_bad(string dir_path) {
    try {
        CSR<uint8_t, uint8_t, shared_ptr<uint8_t>, shared_ptr<uint8_t>>
            csr {dir_path + "/triangle-bad.graph"};
        EQ(0, 1);
    } catch (Error&) { }
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_graph, dir_path);
    TEST(read_mval, dir_path);
    TEST(read_graph_messy, dir_path);
    TEST(fail_graph_bad, dir_path);

    return pass;
}
