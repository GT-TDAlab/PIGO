PIGO Tutorial
=============

This document provides several common uses of PIGO and slowly builds up to
demonstrate PIGO's features.

Each section is designed to be self contained, so feel free to jump around
or follow from the beginning to the end.

..  toctree::
    :glob:
    :maxdepth: 1

    tutorials/read_binary
    tutorials/write_binary
    tutorials/simple_graph_cleaner

Tutorial Evaluation Environment
-------------------------------

The tutorials have been tested on three systems:

Laptop
    An early 2015 MacBook Pro (Laptop) with Apple clang 12.0.0 and ``libomp`` installed from Homebrew.
Workstation
    A Dell Precision 7920 with a Samsung SSD 970 EVO Plus 1TB NVMe HDD running elementary OS 5.1.7 and Docker 20.10.7, compiling with g++ 10.2.1.
Server
    A Lenovo x3650 M5 with NVMe disks running Ubuntu 16.04.7, compiling with g++ 9.1.0.

In performance results, numbers reported are the average of three trials.
