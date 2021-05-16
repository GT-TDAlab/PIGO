/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing creating a binary DiGraph
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

    double rstart = omp_get_wtime();
    DiGraph<> g { input_file };
    double rend = omp_get_wtime();

    double wstart = omp_get_wtime();
    g.save(output_file);
    double wend = omp_get_wtime();

    g.free();

    cerr << "[read] " << (rend-rstart) << " sec" << endl;
    cerr << "[write] " << (wend-wstart) << " sec" << endl;

    return 0;
}
