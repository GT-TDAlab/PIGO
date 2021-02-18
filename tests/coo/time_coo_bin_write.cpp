/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing COO reads
 */

#include "pigo.hpp"

#include <iostream>
#include <omp.h>

using namespace std;
using namespace pigo;

int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input-file output-file" << endl;
        return 1;
    }

    string input_file = string(argv[1]);
    string output_file = string(argv[2]);

    double start = omp_get_wtime();
    COO<uint64_t, uint64_t> c { input_file };
    double end = omp_get_wtime();

    double wstart = omp_get_wtime();
    c.save(output_file);
    double wend = omp_get_wtime();

    cerr << "m: " << c.m() << endl;
    cerr << "n: " << c.n() << endl;

    c.free();

    cerr << "[COO read] " << (end-start) << " sec" << endl;
    cerr << "[COO write] " << (wend-wstart) << " sec" << endl;

    return 0;
}
