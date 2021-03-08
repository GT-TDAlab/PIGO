/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for reading binary entries in files
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

#include "tests.hpp"
#include "pigo.hpp"

using namespace std;
using namespace pigo;

int wrong_allocator() {
    try {
        float x;
        detail::allocate_mem_(x, 1);
        EQ(1, 0);
    } catch (Error&) {
    }
    return 0;
}

int main() {
    int pass = 0;

    TEST(wrong_allocator);

    return pass;
}
