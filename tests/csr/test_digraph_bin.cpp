/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for reading and writing DiGraphs directly
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int write_bin(string dir_path) {
    // Create a DiGraph first
    DiGraph<> write { dir_path + "/gnp_100_2.el" };

    EQ(write.n(), 100);
    EQ(write.m(), 2002);

    write.save(".test.dig_bin.pigo");

    // Now, read it and compare
    DiGraph<> read { ".test.dig_bin.pigo" };

    EQ(read.n(), write.n());
    EQ(read.m(), write.m());

    auto wie = write.in().endpoints();
    auto rie = read.in().endpoints();
    auto wio = write.in().offsets();
    auto rio = read.in().offsets();

    auto woe = write.out().endpoints();
    auto roe = read.out().endpoints();
    auto woo = write.out().offsets();
    auto roo = read.out().offsets();

    for (size_t v = 0; v < read.n()+1; ++v) {
        EQ(wio[v], rio[v]);
        EQ(woo[v], roo[v]);
    }

    for (size_t e = 0; e < read.m(); ++e) {
        EQ(wie[e], rie[e]);
        EQ(woe[e], roe[e]);
    }

    write.free();
    read.free();

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

    return pass;
}
