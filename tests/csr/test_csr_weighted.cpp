/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for weighted CSRs
 */

#include "tests.hpp"
#include "pigo.hpp"
#include <algorithm>

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

template<class T, class W>
struct coosorter {
    private:
        T* x;
        T* y;
        W* w;
    public:
        coosorter(T* x, T* y, W* w) : x(x), y(y), w(w) { }
        bool operator()(size_t i, size_t j) const {
            if (x[i] < x[j]) return true;
            if (x[i] > x[j]) return false;
            if (y[i] < y[j]) return true;
            if (y[i] > y[j]) return false;
            return w[i] < w[j];
        }
};

int fail_load_graph(string dir_path) {
    try {
        CSR<int, int, vector<int>, vector<int>, true, float, vector<float>>
            csr { dir_path + "/base1.graph" };
        EQ(1, 0);
    } catch (NotYetImplemented&) {}
    return 0;
}

int load_weighted(string dir_path) {
    CSR<int, int, vector<int>, vector<int>, true, float, vector<float>>
        csr { dir_path + "/../../coo/data/weighted.mtx" };

    EQ(csr.n(), 6);
    EQ(csr.m(), 6);

    auto w = csr.weights();
    FEQ(w[0], -0.04321);

    return 0;
}

int read_write_weighted(string dir_path) {
    CSR<short, uint64_t, vector<short>, vector<uint64_t>, true, double, vector<double>>
        csr_w { dir_path + "/../../coo/data/weighted.mtx" };
    csr_w.save(".csr.load_weighted.pigo");

    CSR<short, uint64_t, vector<short>, vector<uint64_t>, true, double, vector<double>>
        csr_r { ".csr.load_weighted.pigo" };

    EQ(csr_r.n(), csr_w.n());
    EQ(csr_r.m(), csr_w.m());

    NOPRINT_EQ(csr_r.endpoints(), csr_w.endpoints());
    NOPRINT_EQ(csr_r.weights(), csr_w.weights());
    NOPRINT_EQ(csr_r.offsets(), csr_w.offsets());

    auto w = csr_r.weights();
    FEQ(w[0], -0.04321);
    FEQ(w[1], -6.6e-10);

    return 0;
}

int w_back_forth(string dir_path) {
    WCOO<short, short, vector<short>, float, vector<float>>
        coo_start { dir_path + "/../../coo/data/weighted.mtx" };

    CSR<uint32_t, uint32_t, vector<uint32_t>, shared_ptr<uint32_t>,
        true, double, shared_ptr<double>>
            csr { coo_start };

    WCOO<short, short, vector<short>, float, vector<float>>
        coo_end { csr };

    auto x1 = coo_start.x().data(); auto y1 = coo_start.y().data(); auto w1 = coo_start.w().data();
    auto x2 = coo_end.x().data(); auto y2 = coo_end.y().data(); auto w2 = coo_end.w().data();

    // Sort them by indicies, and then compare the indices
    // Not memory efficient
    size_t* idx1 = new size_t[coo_start.m()];
    for (short i = 0; i < coo_start.m(); ++i)
        idx1[i] = i;

    size_t* idx2 = new size_t[coo_end.m()];
    for (short i = 0; i < coo_end.m(); ++i)
        idx2[i] = i;

    std::sort(idx1, idx1+coo_start.m(), coosorter<short, float>(x1, y1, w1));
    std::sort(idx2, idx2+coo_start.m(), coosorter<short, float>(x2, y2, w2));

    for (short i = 0; i < coo_start.m(); ++i) {
        EQ(coo_start.x()[idx1[i]], coo_end.x()[idx2[i]]);
        EQ(coo_start.y()[idx1[i]], coo_end.y()[idx2[i]]);
        EQ(coo_start.w()[idx1[i]], coo_end.w()[idx2[i]]);
    }

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

    TEST(fail_load_graph, dir_path);
    TEST(load_weighted, dir_path);
    TEST(read_write_weighted, dir_path);
    TEST(w_back_forth, dir_path);

    return pass;
}
