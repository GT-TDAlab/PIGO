#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
using namespace std;

#include "pigo.hpp"
using namespace pigo;

// Setup a simple timer
#include <omp.h>
double g_t_;
inline void tlog_init() {
    cerr << fixed << setprecision(2);
    g_t_ = omp_get_wtime();
}
inline void tlog(string msg) {
    cerr << omp_get_wtime()-g_t_ << " sec " << msg << endl;
    tlog_init();
}

int main(int argc, char** argv) {
    // Open and load the file
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " in-file out-file" << endl;
        return 1;
    }

    tlog_init();

    COO<uint64_t, uint64_t, shared_ptr<uint64_t>,
            true, false, false> coo { argv[1] };
    tlog("read input into coo");

    CSR<uint64_t, uint64_t,
            shared_ptr<uint64_t>, shared_ptr<uint64_t>> csr { coo };
    tlog("built csr");

    csr.sort();
    tlog("sort csr");

    coo.free();
    tlog("free read input coo");

    COO<uint64_t, uint64_t, shared_ptr<uint64_t>> out_coo { csr };

    csr.free();
    tlog("free csr");

    out_coo.write(argv[2]);
    tlog("wrote output file");

    out_coo.free();
    tlog("free out coo");

    return 0;
}
