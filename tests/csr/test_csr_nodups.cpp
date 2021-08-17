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

    return pass;
}
