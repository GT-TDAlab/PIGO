/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading COO files
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int read_clean_el_rptr(string dir_path) {
    COO<> c { dir_path + "/clean.el", EDGE_LIST };

    // Ensure that the count is correct
    EQ(c.m(), 7);
    EQ(c.n(), 10);

    // Ensure the values are correct
    auto x = c.x();
    auto y = c.y();

    size_t pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 3); EQ(y[pos++], 4);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    // Cleanup
    free(x);
    free(y);
    return 0;
}

int read_clean_el_vec(string dir_path) {
    COO<uint32_t, uint32_t, vector<uint32_t> >
        c { dir_path + "/clean.el", EDGE_LIST };

    // Ensure that the count is correct
    EQ(c.m(), 7);
    EQ(c.n(), 10);

    auto x = c.x();
    auto y = c.y();

    EQ(x.size(), 7);
    EQ(y.size(), 7);

    size_t pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 3); EQ(y[pos++], 4);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);

    return 0;
}

int read_clean_el_sptr(string dir_path) {
    COO<uint32_t, uint32_t, shared_ptr<uint32_t> >
        c { dir_path + "/clean.el", EDGE_LIST };

    // Ensure that the count is correct
    EQ(c.m(), 7);
    EQ(c.n(), 10);

    auto x = c.x().get();
    auto y = c.y().get();

    size_t pos = 0;
    EQ(x[pos], 0); EQ(y[pos++], 1);
    EQ(x[pos], 1); EQ(y[pos++], 2);
    EQ(x[pos], 2); EQ(y[pos++], 3);
    EQ(x[pos], 3); EQ(y[pos++], 4);
    EQ(x[pos], 4); EQ(y[pos++], 5);
    EQ(x[pos], 5); EQ(y[pos++], 0);
    EQ(x[pos], 2); EQ(y[pos++], 9);


    return 0;
}

int read_dirty_clean(string dir_path) {
    COO<> with { dir_path + "/with-comments.el", EDGE_LIST };
    COO<> without { dir_path + "/without-comments.el", EDGE_LIST };

    EQ(with.n(), without.n());
    EQ(with.m(), without.m());

    auto wx = with.x(); auto wy = with.y();
    auto ox = without.x(); auto oy = without.y();

    for (size_t i = 0; i < with.m(); ++i) {
        EQ(wx[i], ox[i]);
        EQ(wy[i], oy[i]);
    }

    free(wx); free(wy);
    free(ox); free(oy);
    
    return 0;
}

int read_from_csr_bin(string dir_path) {
    {
        COO<> coo { dir_path + "/ba_100_14_1.csr.pigo" };
        EQ(coo.n(), 100);
        EQ(coo.m(), 1204);
        coo.free();
    }

    try {
        COO<uint8_t> coo { dir_path + "/ba_100_14_1.csr.pigo" };
        EQ(0, 1);
    } catch(Error&) { }
    return 0;
}

int read_from_csr_graph(string dir_path) {
    COO<> coo { dir_path + "/example.graph" };
    EQ(coo.n(), 10);
    EQ(coo.m(), 15);
    coo.free();
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_clean_el_rptr, dir_path);
    TEST(read_clean_el_sptr, dir_path);
    TEST(read_clean_el_vec, dir_path);
    TEST(read_dirty_clean, dir_path);
    TEST(read_from_csr_bin, dir_path);
    TEST(read_from_csr_graph, dir_path);

    return pass;
}
