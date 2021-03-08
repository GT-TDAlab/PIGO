/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains test files for reading and writing CSR binary files
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <memory>
#include <vector>

using namespace std;
using namespace pigo;

int base_one(string dir_path) {
    BaseGraph<int32_t, int32_t, vector<int32_t>, vector<int32_t>>
        g {dir_path+"/base1.graph"};
    cout << g.n() << endl;
    cout << g.m() << endl;
    EQ(g.degree(1), 3);
    for (auto v : g.neighbors(1))
        cout << v << endl;
    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(base_one, dir_path);

    return pass;
}
