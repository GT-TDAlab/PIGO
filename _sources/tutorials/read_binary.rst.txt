Read Binary Files
=================

In this tutorial file we show to how to read a binary file in parallel
with PIGO.

The Setup
---------

Suppose we are reading a dense array of integers from a binary file.
The file format is simple: first the number of integers are stored (as an
unsigned integer) and then, one after another, the integers.

We have an existing program that reads such arrays and the goal is to
modify it to read with PIGO to take advantage of parallel improvements.

The full format is as follows::

    <size:uint64_t>
    <data:int32*>

On Linux with bash, we can generate a random large file with the following
commands:


Loading the Vector
------------------

We want to build a program that loads the vector to then perform some
action on it.

Following is the existing program that we start with (`readbin.cpp`)::

    #include <iostream>
    #include <fstream>
    #include <cstdint>
    using namespace std;

    void do_something(uint64_t size, int32_t* vec) { }

    int main(int argc, char** argv) {
        // Open and load the file
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " file" << endl;
            return 1;
        }

        // Read the input file
        double t = omp_get_wtime();
        auto fin = fstream(argv[1], ios::in | ios::binary);
        uint64_t size;
        fin.read((char*)&size, sizeof(size))
        int32_t* vec = new int32_t[size];
        fin.read((char*)vec, sizeof(int32_t)*size);
        fin.close();
        cerr << "[read] " << omp_get_wtime()-t << end;

        do_something(size, vec);

        delete [] vec;
        return 0;
    }

This program can be compiled with `g++ -O3 -a readbin readbin.cpp`.

When it is run on a large file, say with 2.6 billion we get the following:
