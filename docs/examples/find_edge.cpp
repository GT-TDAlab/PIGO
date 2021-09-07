
#include <iostream>
#include <string>

#include "pigo.hpp"

#include <omp.h>

using namespace std;
using namespace pigo;

using vertex_t = int64_t;
using edge_t = int64_t;

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " input-graph src dst" << endl;
        return 1;
    }

    vertex_t src = stoll(argv[2]);
    vertex_t dst = stoll(argv[3]);

    double t = omp_get_wtime();
    BaseGraph<vertex_t, edge_t> g { argv[1] };
    cerr << "read graph : " << omp_get_wtime()-t << endl;
    t = omp_get_wtime();

    if (src >= g.n() || dst >= g.n())
        throw runtime_error("Improper src, dst");

    g.sort();
    cerr << "sort graph : " << omp_get_wtime()-t << endl;
    t = omp_get_wtime();

    // Find the edge, using std::binary_search
    auto& endpoints = g.endpoints();
    auto n_start = g.neighbor_start(src);
    auto n_end = g.neighbor_end(src);
    if (binary_search(endpoints+n_start, endpoints+n_end, dst)) {
        cout << ">> edge found" << endl;
        return 0;
    } else {
        cout << ">> NOT found" << endl;
        return 2;
    }

    g.free();
}
