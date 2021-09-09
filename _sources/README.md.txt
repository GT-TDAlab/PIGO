# PIGO: a Parallel Graph Input and Output library

[![Build Status](https://app.travis-ci.com/GT-TDAlab/PIGO.svg?branch=master)](https://app.travis-ci.com/github/GT-TDAlab/PIGO)

PIGO is a library built to assist you with common sparse graph or matrix
input, output, and preprocessing. It supports easily loading a variety of
graph formats rapidly in parallel, performing standard preprocessing, and
saving results and intermediate representations.

PIGO is built in C++ and its releases are single header-only files.  It is
designed to be simple to integrate into many projects.

💻 **Source Code:** [http://github.com/GT-TDAlab/PIGO]  
📘 **Documentation:** [http://gt-tdalab.github.io/PIGO/]

## Quick Start Guide

First, download the PIGO release `pigo.hpp`. To compile a PIGO
application, you'll need to use OpenMP and ensure `pigo.hpp` is in
a directory where the compiler can find it.

Here we provide a simple example program. Create the file `example.cpp` in
the same directory as `pigo.hpp`. Add the following to it:

```C++
#include "pigo.hpp"
#include <iostream>
using namespace std;
using namespace pigo;
int main(int argc, char** argv) {
    if (argc != 2) {
    	cerr << "Usage: " << argv[0] << " filename" << endl;
    	return 1;
    }
    
    Graph g { argv[1] };
    cout << "number of vertices: " << g.n() << endl;
    cout << "number of edges: " << g.m() << endl;
    
    cout << "vertex 0's neighbors:\n";
    for (auto n : g.neighbors(0))
        cout << n << endl;
    return 0;
}
```

Finally, compile the program with
`g++ -O3 -o example example.cpp -fopenmp`.

You can now run the example program to print out the size of the graph and
the neighbors of vertex 0.

For example, create the file `toy.el`:

```
0 4
4 3
0 5
0 9
```

`./example toy.el` should now print:

```
number of vertices: 10
number of edges: 4
vertex 0's neighbors:
4
5
9
```

## Documentation

The documentation relies on the following dependencies:
* [Python 3](https://www.python.org/downloads/)
* [Pip](https://pip.pypa.io/en/stable/installing/)
* [Doxygen](https://www.doxygen.nl/download.html)

To install additional Python dependencies, run
`pip install -r docs/requirements.txt`.

Next, the documentation can be built by running
`cd build && cmake .. && make docs`.
The documentation will then be in `build/docs/sphinx`.

## Running Tests

Unit tests are controlled using CMake. To use them, you will need a modern
version of CMake. You can do the following:
`cd build && cmake .. && make -j 8 && ctest`.

## License

Please see [`LICENSE.md`](LICENSE.md) for the license.

## Contributors

* [Kasimir Gabert](https://kasimir.co)
* [Umit V. Catalyurek](https://cc.gatech.edu/~umit)

## Citation

If you use this work in an academic setting, or are inspired by it, please
consider citing the following (BibTeX):

```bibtex
@inproceedings {Gabert21-GrAPL,
    title = {{PIGO}: A Parallel Graph Input/Output Library},
    author = {Kasimir Gabert and \"Umit V. \c{C}ataly\"{u}rek},
    booktitle = {2021 IEEE International Parallel and Distributed Processing Symposium Workshops (IPDPSW)},
    pages = {276--279},
    year = {2021},
    organization = {IEEE},
    doi = { 10.1109/IPDPSW52791.2021.00050 }
}
```

## Contact

For questions or support please either open an issue or contact
<tdalab@cc.gatech.edu>.
