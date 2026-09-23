/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading COO files
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace pigo;

/** Writes contents to fn verbatim, with no newline appended
 *
 * Tests for missing-trailing-newline handling have to build their input at
 * runtime: git and most editors normalize a final newline onto checked-in
 * files, so a fixture could not be relied on to keep its exact last byte.
 */
static void write_exact(const string& fn, const string& contents) {
    ofstream f { fn.c_str(), ios::out | ios::binary | ios::trunc };
    f.write(contents.c_str(), contents.size());
}

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

/** A file whose last line has no trailing newline reads identically to one
 * that does.
 *
 * Regression test: the last entry used to be counted but never stored,
 * leaving its slot uninitialized (it typically read back as 0 -> 0).
 */
int read_no_trailing_newline(string) {
    const string body = "0 1\n1 2\n2 3";
    write_exact(".test.no_trail.el", body);
    write_exact(".test.with_trail.el", body + "\n");

    COO<uint64_t, uint64_t, vector<uint64_t> >
        without { ".test.no_trail.el", EDGE_LIST };
    COO<uint64_t, uint64_t, vector<uint64_t> >
        with { ".test.with_trail.el", EDGE_LIST };

    EQ(without.m(), 3);
    EQ(with.m(), without.m());
    EQ(with.n(), without.n());

    // The final edge must be the real one, not an unwritten slot
    EQ(without.x()[2], 2);
    EQ(without.y()[2], 3);

    for (size_t i = 0; i < with.m(); ++i) {
        EQ(without.x()[i], with.x()[i]);
        EQ(without.y()[i], with.y()[i]);
    }

    remove(".test.no_trail.el");
    remove(".test.with_trail.el");
    return 0;
}

/** As above, but weighted, and with a single-line file (no newline at all) */
int read_no_trailing_newline_weighted(string) {
    write_exact(".test.no_trail_w.el", "0 1 5\n1 2 7");
    write_exact(".test.one_line.el", "3 4");

    COO<uint64_t, uint64_t, vector<uint64_t>, false, false, false, true,
        float, vector<float> > w { ".test.no_trail_w.el", EDGE_LIST };
    EQ(w.m(), 2);
    EQ(w.x()[1], 1);
    EQ(w.y()[1], 2);
    FEQ(w.w()[1], 7.0);

    COO<uint64_t, uint64_t, vector<uint64_t> >
        one { ".test.one_line.el", EDGE_LIST };
    EQ(one.m(), 1);
    EQ(one.x()[0], 3);
    EQ(one.y()[0], 4);

    remove(".test.no_trail_w.el");
    remove(".test.one_line.el");
    return 0;
}

/** A binary COO whose header declares more entries than the file holds must
 * be rejected, not read out of bounds.
 *
 * Regression test: read_bin_ trusts the m_ from the header and hands
 * sizeof(L)*m_ straight to parallel_read, which memcpy's that many bytes out
 * of the mapped file with no bounds check. A corrupt or truncated header with
 * an oversized m_ therefore read past the end of the mapping (a SEGV under
 * ASan). parallel_read now verifies the requested region fits in the file.
 */
int read_bin_corrupt_header(string) {
    // Vector storage so allocations are freed even when the constructor throws
    // mid-read (raw-pointer storage would leak the partial allocation, since a
    // failed constructor's destructor never runs).
    typedef COO<uint32_t, uint32_t, vector<uint32_t> > VCOO;

    // Produce a valid binary COO from a small edge list
    write_exact(".test.corrupt.el", "0 1\n1 2\n2 3\n");
    VCOO real { ".test.corrupt.el", EDGE_LIST };
    real.save(".test.corrupt.coo");

    // Load the bytes and overwrite m_ with an enormous value. The layout is
    // header ("PIGO-COO-v1", 11 bytes) + L_size + O_size (2) + nrows + ncols +
    // n (3 * sizeof(uint32_t)) = offset 25, where m_ (uint32_t) lives.
    string data;
    {
        ifstream f { ".test.corrupt.coo", ios::binary };
        data.assign((istreambuf_iterator<char>(f)),
                istreambuf_iterator<char>());
    }
    uint32_t huge = 100000000u;   // 1e8 entries -> far beyond the file
    memcpy(&data[25], &huge, sizeof(huge));
    write_exact(".test.corrupt.bad.coo", data);

    // Reading it must throw rather than crash
    bool threw = false;
    try {
        VCOO bad { ".test.corrupt.bad.coo", PIGO_COO_BIN };
        (void)bad;
    } catch (const Error&) {
        threw = true;
    }
    EQ(threw, true);

    real.free();
    remove(".test.corrupt.el");
    remove(".test.corrupt.coo");
    remove(".test.corrupt.bad.coo");
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
    TEST(read_no_trailing_newline, dir_path);
    TEST(read_no_trailing_newline_weighted, dir_path);
    TEST(read_bin_corrupt_header, dir_path);

    return pass;
}
