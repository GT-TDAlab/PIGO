# Changelog
This file documents PIGO's changes.

## [Unreleased]
### Added (major)
- Support for signed and unsigned integral and floating point weights
- Add the Matrix class, which is non-symmetric by default, and added
  SymMatrix for the specialized symmetric case (represented only by
  a CSR).
- Added a DiGraph (directed graph) class, similar to a Matrix but with
  graph-specific naming and in-edge and out-edge iterators.

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

### Breaking changes (minor)
- Removed the Matrix alias due to adding full (non-symmetric) Matrix
  support

### Fixed
- Fixed support for converting from shared_ptr COOs to CSRs
- Fixed passing template arguments from CSR to COOs

## [0.4] - 2021-02-17
Initial public release.
