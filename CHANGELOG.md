# Changelog
This file documents PIGO's changes.

## [Unreleased]
### Added (minor)
- Support for getting the offsets of a character in a FileReader, for example
  to find offsets for all newlines in file
- Adjusted the binary tensor magic string to be one character shorter,
  enabling better padding.

### Fixed
- Fixed a bug which caused saved binary tensor files to be too large.
- Fixed #4, where writes with floating point values had reduced parallelism.
  Instead of using the C++ standard library to convert a floating point number
  to a string, where there is a dependence on the global locale which prevents
  effective parallelism, a public domain implementation STB (from
  https://github.com/nothings/stb) was modified and is now included with PIGO.
- Clarifying that PIGO runs with C++11 in the README.

## [0.6] - 2022-03-24
### Added (major)
- Support for reading and writing Tensors, which are coordinate lists
  where the coordinates are densely packed row by row.

### Added (minor)
- Support for symmetrizing from a CSR to a COO
- Support for computing the maximum label values per dimension in Tensors
- Support for compiling into a sequential version without OpenMP

### Fixed
- Fixed a bug in determining the order of certain tensors from ASCII files

## [0.5] - 2021-09-13
### Added (major)
- Support for signed and unsigned integral and floating point weights
- Add the Matrix class, which is non-symmetric by default, and added
  SymMatrix for the specialized symmetric case (represented only by
  a CSR).
- Added a DiGraph (directed graph) class, similar to a Matrix but with
  graph-specific naming and in-edge and out-edge iterators.
- Added support for writing out COOs to ASCII files
- Added support for upper-triangle handling in COO template parameters
- Added support for removing multiple edges by a CSR generation method,
  new_csr_without_dups

### Added (minor)
- Support for easily computing the vertex degree (non-zeros in a row)
- Support for changing label and ordinal types between CSR and COOs
- Automatic detection and appropriate handling of 1-based `.graph` files
- Test file for removing self loops
- Support for symmetrizing inputs, along with a timer and test
- Support for returning the number of rows and columns for rectangular CSRs
- Support for transposing COOs
- Support for copying COOs
- Support for (weighted) graphs
- Support for changing the number of rows, cols, and labels in COOs
- Support for reading and writing binary directed graphs
- Added a release generation script to create a single header file
- Added support for sorting CSRs
- Support for creating a fixed-sized uninitialized COO
- Added an example for finding an edge in a graph
- Added const to size returning methods in COO and CSR

### Breaking changes (minor)
- Removed the Matrix alias due to adding full (non-symmetric) Matrix
  support
- Changed symmetrizing to not introduce multiedge diagonals
- Removed the option for the COO to remove multi edges

### Fixed
- Fixed support for converting from shared_ptr COOs to CSRs
- Fixed passing template arguments from CSR to COOs
- Fixed setting n for matrix market files instead of just nrows/ncols
- Fixed symmetrizing with weights
- Fixed a bug where nrows and ncols were not saved in CSR binary files
- Fixed CMake to ensure anything that uses PIGO is linked with OpenMP
- Fixed the detection of Linux-specific file opening options to not detect GCC

## [0.4] - 2021-02-17
Initial public release.
