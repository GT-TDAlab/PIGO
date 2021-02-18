/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing CSR reads and binary
 * writes
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

    double cstart = omp_get_wtime();
    COO<> coo { input_file };
    double cend = omp_get_wtime();

    double start = omp_get_wtime();
    Graph g { coo };
    double end = omp_get_wtime();

    double wstart = omp_get_wtime();
    g.save(output_file);
    double wend = omp_get_wtime();

    coo.free();
    g.free();

    cerr << "[COO read] " << (cend-cstart) << " sec" << endl;
    cerr << "[CSR conv] " << (end-start) << " sec" << endl;
    cerr << "[CSR write] " << (wend-wstart) << " sec" << endl;

    return 0;
}
