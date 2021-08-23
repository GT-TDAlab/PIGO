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

    <size:uint64_t><data:int32>...

On Linux with bash, we can generate a random large file with the following
commands **(Warning: this will create a 3.8 GB file. Adjust the numbers if
necessary.)**:

.. code-block:: bash

    perl -e 'print(pack('Q',999817214))' > ints.bin
    dd if=/dev/urandom of=ints.bin seek=1 bs=8 count=131071
    dd if=/dev/urandom of=ints.bin seek=1 bs=1048576 count=3813

Loading the Vector
------------------

We want to build a program that loads the vector to then perform some
action on it.

Following is the existing program that we start with (`readbin.cpp`)

.. code-block:: C++

    #include <iostream>
    #include <fstream>
    #include <cstdint>
    #include <omp.h>
    using namespace std;

    void do_something(uint64_t size, int32_t* vec) { }

    int main(int argc, char** argv) {
        // Open and load the file
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " file" << endl;
            return 1;
        }

        double t = omp_get_wtime();
        uint64_t size;

        // Open the input file
        auto fin = fstream(argv[1], ios::in | ios::binary);
        // Read the expected size of the file
        fin.read((char*)&size, sizeof(size));
        // Allocate the vector
        int32_t* vec = new int32_t[size];
        // Copy the integers
        fin.read((char*)vec, sizeof(int32_t)*size);

        cerr << "[read] " << omp_get_wtime()-t << endl;

        do_something(size, vec);

        // Cleanup
        delete [] vec;
        fin.close();
        return 0;
    }

This program can be compiled on Linux with
``g++ -std=c++11 -O3 -o readbin readbin.cpp -fopenmp``
and on Mac, after ``brew install libomp``, with
``clang++ -std=c++11 -O3 -o readbin readbin.cpp -Xclang -fopenmp /usr/local/lib/libomp.dylib``.

Loading Faster
--------------

We can use PIGO to load in parallel instead.
Ensure that ``pigo.hpp`` is in the same folder.

PIGO has support for reading and writing binary files through the
``FileReader``.  This class ``ROFile`` can be used for read-only
files.  First, create an ``ROFile`` with the filename to open using
``ROFile fin { argv[1] };``.  Next, the ``uint64_t size`` can be read
out by using the ``read`` method of the ``ROFile``, ``size
= fin.read<uint64_t>();`` Finally, the vector can be populated by replacing
the read with a ``parallel_read``, which takes the destination and the
number of bytes to read.

The modified program is as follows:

.. code-block:: C++

    #include <iostream>
    #include <fstream>
    #include <cstdint>
    #include <omp.h>
    using namespace std;

    #include "pigo.hpp"
    using namespace pigo;

    void do_something(uint64_t size, int32_t* vec) { }

    int main(int argc, char** argv) {
        // Open and load the file
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " file" << endl;
            return 1;
        }

        // Read the input file
        double t = omp_get_wtime();
        ROFile fin { argv[1] };
        uint64_t size = fin.read<uint64_t>();
        int32_t* vec = new int32_t[size];
        fin.parallel_read((char*)vec, sizeof(int32_t)*size);
        cerr << "[read] " << omp_get_wtime()-t << endl;

        do_something(size, vec);

        delete [] vec;
        return 0;
    }

Performance Comparison
----------------------

Following are the results in seconds between the original, ``fread``
version and the PIGO version. (C) means the cache is cold and (W) means
warm.

================  =========  ====  =======
System            ``fread``  PIGO  Speedup
================  =========  ====  =======
Laptop (C)        6.44       6.39  1.01 x
Laptop (W)        3.25       1.48  2.20 x
Workstation (C)   4.22       1.50  2.81 x
Workstation (W)   2.32       0.26  8.92 x
Server (C)        6.53       1.53  4.27 x
Server (W)        1.73       0.18  9.61 x
================  =========  ====  =======
