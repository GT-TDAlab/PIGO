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
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input-file output-file" << endl;
        return 1;
    }

    string input_file = string(argv[1]);
    string output_file = string(argv[2]);

    double clk = omp_get_wtime();
    Tensor<uint32_t, uint64_t> t { input_file };
    cerr << "Tensor load time: " << (omp_get_wtime()-clk) << " sec" << endl;

    clk = omp_get_wtime();
    t.save(output_file);
    cerr << "Tensor binary write time: " << (omp_get_wtime()-clk) << " sec" << endl;

    t.free();

    return 0;
}
