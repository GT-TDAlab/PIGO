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

int read_write_small() {
    // Create a small "file"
    char file[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    // Read and write values
    int32_t v1 = -12345;
    int16_t v2 = 213;
    float v3 = 555.5;
    char v4 = 'X';
    FilePos fp = file;
    write(fp, v1);
    write(fp, v2);
    write(fp, v3);
    write(fp, v4);

    EQ(fp, file+sizeof(file));

    fp = file;
    int32_t r1 = read<int32_t>(fp);
    int16_t r2 = read<int16_t>(fp);
    float r3 = read<float>(fp);
    char r4 = read<char>(fp);

    EQ(v1, r1);
    EQ(v2, r2);
    EQ(v3, r3);
    EQ(v4, r4);

    return 0;
}

int file_writing() {
    vector<float> dat;
    dat.reserve(500);
    dat.push_back(1.234);
    for (size_t ctr = 1; ctr < 500; ++ctr)
        dat.push_back(dat[ctr-1]*1.15+5.555);

    {
        File f(".TMP.fwriting-test.dat", WRITE, 500*sizeof(float));
        for (auto val : dat)
            f.write(val);
    }

    {
        File f(".TMP.fwriting-test.dat", READ);
        for (auto val : dat) {
            EQ(f.read<float>(), val);
        }
    }

    return 0;
}

int file_reading(string dir_path) {
    ROFile f(dir_path+"/file.txt");

    const char res[] = "Written by GPT2, BASED on a novel by GPT5, this book covers the events of the last three years of life, and their impact on modern science and technology.";

    int check = strncmp(res, f.fp(), strlen(res));
    EQ(check, 0);

    return 0;
}

int file_size(string dir_path) {
    ROFile f(dir_path+"/file.txt");

    EQ(f.size(), 155);

    return 0;
}

int main(int argc, char **argv) {
    int pass = 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " test-source-path" << endl;
        return 1;
    }

    string dir_path = string(argv[1]) + "/data";

    TEST(read_write_small);
    TEST(file_writing);
    TEST(file_reading, dir_path);
    TEST(file_size, dir_path);

    return pass;
}
