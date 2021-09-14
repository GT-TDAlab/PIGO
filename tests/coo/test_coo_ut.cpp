/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading COO files and only keeping the
 * upper triangle entries (col >= src).
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int ut(string dir_path) {
    COO<int, int, vector<int>, false, true> c { dir_path + "/ut.el" };

    EQ(c.m(), 8);
    EQ(c.n(), 7);

    vector<int> vx = {5,4,4,3,1,1,0,0};
    vector<int> vy = {6,4,4,4,2,1,2,1};

    NOPRINT_EQ(c.x(), vx);
    NOPRINT_EQ(c.y(), vy);

    return 0;
}

int ut_sym(string dir_path) {
    COO<int, int, vector<int>, true, true> c { dir_path + "/ut.el" };

    EQ(c.m(), 14);
    EQ(c.n(), 10);

    vector<int> vx = {5,4,4,3,5,1,1,0,0,0,8,7,6,5};
    vector<int> vy = {6,4,4,4,6,2,1,1,2,1,9,8,7,6};

    NOPRINT_EQ(c.x(), vx);
    NOPRINT_EQ(c.y(), vy);

    return 0;
}

int ut_sym_rsl(string dir_path) {
    COO<int, int, vector<int>, true, true, true> c { dir_path + "/ut.el" };

    c.write("xx");

    EQ(c.m(), 11);
    EQ(c.n(), 10);

    vector<int> vx = {5,3,5,1,0,0,0,8,7,6,5};
    vector<int> vy = {6,4,6,2,1,2,1,9,8,7,6};

    NOPRINT_EQ(c.x(), vx);
    NOPRINT_EQ(c.y(), vy);

    return 0;
}

int ut_rsl(string dir_path) {
    COO<int, int, vector<int>, false, true, true> c { dir_path + "/ut.el" };

    EQ(c.m(), 5);
    EQ(c.n(), 7);

    vector<int> vx = {5,3,1,0,0};
    vector<int> vy = {6,4,2,2,1};

    NOPRINT_EQ(c.x(), vx);
    NOPRINT_EQ(c.y(), vy);

    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(ut, dir_path);
    TEST(ut_sym, dir_path);
    TEST(ut_sym_rsl, dir_path);
    TEST(ut_rsl, dir_path);

    return pass;
}
