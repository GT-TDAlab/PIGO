/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing COO writes
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

    double t = omp_get_wtime();
    COO<> c { input_file };
    cerr << "[load] " << omp_get_wtime()-t << endl;

    t = omp_get_wtime();
    c.write(output_file);
    cerr << "[write] " << omp_get_wtime()-t << endl;

    c.free();

    return 0;
}
