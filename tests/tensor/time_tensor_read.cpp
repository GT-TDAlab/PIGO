/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains a test harness for timing tensor reads
 */

#include "pigo.hpp"

#include <iostream>
#include <iomanip>
#include <omp.h>

using namespace std;
using namespace pigo;

int main(int argc, char **argv) {
    double start, end;
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " input-file" << endl;
        return 1;
    }

    string input_file = string(argv[1]);

    start = omp_get_wtime();
    Tensor<uint32_t, uint64_t> t { input_file };
    end = omp_get_wtime();

    cerr << "- TIMING -------------------------------" << endl;
    cerr << "Tensor load time: " << fixed << setprecision(5) << (end-start) << " sec" << endl;

    start = omp_get_wtime();
    auto maxes = t.max_labels();
    end = omp_get_wtime();
    cerr << "Compute max labels time: " << (end-start) << " sec" << endl;

    cerr << endl <<"- DATASET INFOMATION -------------------" << endl;
    cerr << "Order = " << t.order() << endl;
    cerr << "Number non-zeros = " << t.m() << endl;
    cerr << "Max labels =";
    for (auto m : maxes)
        cerr << " " << m;
    cerr << endl;

    t.free();

    return 0;
}
