/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * A simple test system, support equality checking and test execution
 */

#include <cstdlib>
#include <cmath>
#include <iostream>

/**
 * @file tests.hpp
 * @brief This file contains helper macros for running simple unit tests
 */

/** The floating point equivalence requirement */
#define F_EPS 1e-8

#define EQ(x,y)                                                                         \
    do {                                                                                \
        if ((x) != (y)) {                                                               \
            std::cerr << "FAILURE: Expected " << (y) << " instead got " << (x)          \
                << " for " << #x << " in " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1;                                                                   \
            }                                                                           \
    } while(0);

#define NOPRINT_EQ(x,y)                                                                 \
    do {                                                                                \
        if ((x) != (y)) {                                                               \
            std::cerr << "FAILURE: values not equal"                                    \
                << " for " << #x << " in " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1;                                                                   \
            }                                                                           \
    } while(0);

#define FEQ(x,y)                                                                        \
    do {                                                                                \
        if (std::fabs((x) - (y)) > F_EPS) {                                             \
            std::cerr << "FAILURE: Expected " << (y) << " instead got " << (x)          \
                << " for " << #x << " in " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1;                                                                   \
            }                                                                           \
    } while(0);

#define TEST(f, ...)                                             \
    do {                                                         \
        int pass_ = f(__VA_ARGS__);                              \
        if (pass_ != 0) {                                        \
                std::cerr << "TEST FAILED: " << #f << std::endl; \
            } else {                                             \
                std::cout << "TEST PASSED: " << #f << std::endl; \
            }                                                    \
            pass |= pass_;                                       \
    } while(0);

#define COMMA ,
