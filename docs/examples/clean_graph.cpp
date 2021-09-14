#include <iostream>
#include <memory>
#include <string>
using namespace std;

#include "pigo.hpp"
using namespace pigo;

// Setup a simple timer
#include <omp.h>
double g_t_;
inline void tlog_init() { g_t_ = omp_get_wtime(); }
inline void tlog(string msg) {
    cerr << omp_get_wtime()-g_t_ << " " << msg << endl;
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
            true, true, true> coo { argv[1] };
    tlog("read input into coo");

    CSR<uint64_t, uint64_t,
            shared_ptr<uint64_t>, shared_ptr<uint64_t>> csr { coo };
    tlog("built csr");

    coo.free();
    tlog("free read input coo");

    auto clean = csr.new_csr_without_dups();
    tlog("built clean csr");

    csr.free();
    tlog("free original csr");

    COO<uint64_t, uint64_t, shared_ptr<uint64_t>> out_coo { clean };
    tlog("built output coo");

    clean.free();
    tlog("free clean csr");

    out_coo.write(argv[2]);
    tlog("wrote output file");

    out_coo.free();
    tlog("free out coo");

    return 0;
}
