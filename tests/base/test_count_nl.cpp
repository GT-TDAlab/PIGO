/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2023 Kasimir Gabert
 *
 * This file contains tests for reading binary entries in files
 */

#include <iostream>
#include <vector>

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

using TT = Tensor<size_t, size_t, vector<size_t>, float, float*, false>;

int count_nl(string dir_path) {
    ROFile f {dir_path+"/nl.txt"};
    auto r = f.reader();
    TT nls = r.find_offsets<TT>('\n');

    auto& nls_offs = nls.c();
    EQ(nls_offs.size(), 22);
    EQ(nls_offs[0], 48);
    EQ(nls_offs[20], 514);
    EQ(nls_offs[21], 515);
    
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(count_nl, dir_path);

    return pass;
}
