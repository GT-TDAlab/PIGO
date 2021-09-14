Simple Graph Cleaner
====================

In this tutorial we develop a graph cleaning utility that takes in a graph
file and "cleans" it to be a simple graph.  That is, it will symmetrize
the graph, remove self loops and multiple edges, and output an edge list
with only one direction (the "upper" triangle).

Building the Tool
-----------------

For this example, we will not evaluate whether the input is already
symmetric or not.  Instead, we will perform an expensive approach of
always symmetrizing it. We start by loading the input into a ``COO``.  We
then use template parameter to indicate we want to symmetrize it and
remove self loops.

.. code-block:: C++

    COO<uint64_t, uint64_t, shared_ptr<uint64_t>,
            true, true, true> coo { argv[1] };

If we want the remove self loops and multi-edges from a directed graph, but not
symmetrize it, we would replace the first two ``true`` values with ``false``
(template parameters 4 and 5 would become false).

Once we have this COO, we can convert it into a CSR.  We will use the CSR
to then detect and remove redundant, multiple edges.

.. code-block:: C++

    CSR<uint64_t, uint64_t,
            shared_ptr<uint64_t>, shared_ptr<uint64_t>> csr { coo };

Next, we need to build a new CSR that does not have redundant edges.  Here
we use a provided method, ``new_csr_without_dups``, that generates a new,
clean CSR. Any new template parameters can be passed to this function for
the resulting CSR to take.

.. code-block:: C++

    auto clean = csr.new_csr_without_dups();

As a last step, we convert back to a new COO and write it out as an ascii
file using ``write``.

The full program is as follows:

.. code-block:: C++

    #include <iostream>
    #include <memory>
    #include <string>
    using namespace std;

    #include "pigo.hpp"
    using namespace pigo;

    // Setup a simple timer
    #include <omp.h>
    double g_t_;
    inline void tlog_init() { g_t_ = omp_get_wtime(); }
    inline void tlog(string msg) {
        cerr << omp_get_wtime()-g_t_ << " " << msg << endl;
        tlog_init();
    }

    int main(int argc, char** argv) {
        // Open and load the file
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " in-file out-file" << endl;
            return 1;
        }

        tlog_init();

        COO<uint64_t, uint64_t, shared_ptr<uint64_t>,
                true, true, true> coo { argv[1] };
        tlog("read input into coo");

        CSR<uint64_t, uint64_t,
                shared_ptr<uint64_t>, shared_ptr<uint64_t>> csr { coo };
        tlog("built csr");

        coo.free();
        tlog("free read input coo");

        auto clean = csr.new_csr_without_dups();
        tlog("built clean csr");

        csr.free();
        tlog("free original csr");

        COO<uint64_t, uint64_t, shared_ptr<uint64_t>> out_coo { clean };
        tlog("built output coo");

        clean.free();
        tlog("free clean csr");

        out_coo.write(argv[2]);
        tlog("wrote output file");

        out_coo.free();
        tlog("free out coo");

        return 0;
    }

Building
--------

After ensuring ``pigo.hpp`` is in the same directory, this program can be
compiled on Linux with
``g++ -std=c++11 -O3 -o clean_graph clean_graph.cpp -fopenmp``
and on Mac, after ``brew install libomp``, with
``clang++ -std=c++11 -O3 -o clean_graph clean_graph.cpp -Xclang -fopenmp /usr/local/lib/libomp.dylib``.

Evaluation
----------

Suppose that you have a messy input file, e.g., the following file:

.. code-block::

    5 6
    4 4
    4 4
    3 4
    6 5
    1 2
    1 1
    1 2
    1 2
    2 1
    1 0
    0 2
    0 1

Note that this file contains self loops and multiple edges, some of which
are present if the file is directed, and others are only present when it
is undirected (e.g., ``5 6`` and ``6 5``).

After running ``./clean_graph messy.el clean.el``, the following file is
saved:

.. code-block::

    0 1
    0 2
    1 2
    3 4
    5 6
