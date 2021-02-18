/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for converting back and forth between CSR and
 * COO
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>
#include <algorithm>

using namespace std;
using namespace pigo;

struct coosorter {
    private:
        uint32_t* x;
        uint32_t* y;
    public:
        coosorter(uint32_t* x, uint32_t* y) : x(x), y(y) { }
        bool operator()(size_t i, size_t j) const {
            if (x[i] < x[j]) return true;
            if (x[i] > x[j]) return false;
            return y[i] < y[j];
        }
};

int back_forth(string dir_path) {
    COO<> coo { dir_path + "/ba_100_14_1.el", EDGE_LIST };

    // Convert to a CSR
    CSR<> csr { coo };

    // Convert back to new COO
    COO<> coo2 { csr };

    // Compare
    EQ(coo.m(), coo2.m());
    EQ(coo.m(), csr.m());
    EQ(coo.n(), csr.n());
    EQ(coo.n(), coo2.n());

    auto x1 = coo.x(); auto y1 = coo.y();
    auto x2 = coo2.x(); auto y2 = coo2.y();

    // Sort them by indicies, and then compare the indices
    // Not memory efficient
    size_t* idx1 = new size_t[coo.m()];
    for (size_t i = 0; i < coo.m(); ++i)
        idx1[i] = i;

    size_t* idx2 = new size_t[coo.m()];
    for (size_t i = 0; i < coo.m(); ++i)
        idx2[i] = i;

    std::sort(idx1, idx1+coo.m(), coosorter(x1, y1));
    std::sort(idx2, idx2+coo.m(), coosorter(x2, y2));

    for (size_t i = 0; i < coo.m(); ++i) {
        EQ(coo.x()[idx1[i]], coo2.x()[idx2[i]]);
        EQ(coo.y()[idx1[i]], coo2.y()[idx2[i]]);
    }

    // Cleanup
    coo.free();
    csr.free();
    coo2.free();
    delete [] idx1;
    delete [] idx2;
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(back_forth, dir_path);

    return pass;
}
