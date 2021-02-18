/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing COO into CSR reads
 */

#include "pigo.hpp"

#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;
using namespace pigo;

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " input-file" << endl;
        return 1;
    }

    string input_file = string(argv[1]);

    double start = omp_get_wtime();
    COO<uint32_t, uint32_t, vector<uint32_t>> c { input_file, EDGE_LIST };
    double end = omp_get_wtime();
    cerr << "[COO read] " << (end-start) << " sec" << endl;

    start = omp_get_wtime();
    CSR<> csr { c };
    end = omp_get_wtime();

    cerr << "[CSR conv] " << (end-start) << " sec" << endl;

    c.free();
    csr.free();

    return 0;
}
