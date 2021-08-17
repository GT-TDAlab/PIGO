/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing CSR reads and sorting
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

    double t = omp_get_wtime();
    Graph g { input_file };
    cerr << "[CSR read] " << (omp_get_wtime()-t) << " sec" << endl;

    t = omp_get_wtime();
    g.sort();
    cerr << "[CSR sort] " << (omp_get_wtime()-t) << " sec" << endl;

    g.free();

    return 0;
}
