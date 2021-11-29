/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing tensor reads
 */

#include "pigo.hpp"

#include <iostream>
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
    Tensor<uint64_t, uint64_t> t { input_file };
    double end = omp_get_wtime();

    t.free();

    cerr << "Tensor load time: " << (end-start) << " sec" << endl;

    return 0;
}
