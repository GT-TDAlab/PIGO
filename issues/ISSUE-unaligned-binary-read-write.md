# Misaligned load/store in binary `read<T>` / `write<T>` (undefined behavior)

## Summary

`pigo::read<T>` and `pigo::write<T>` in `include/pigo/impl/pigo.impl.hpp`
dereference a `char*` cast to `T*` at an arbitrary byte offset:

```cpp
template<class T> inline T read(FilePos &fp) {
    T res = *(T*)fp;          // <-- misaligned load
    fp += sizeof(T);
    return res;
}
template<class T> inline void write(FilePos &fp, T val) {
    *(T*)fp = val;            // <-- misaligned store
    fp += sizeof(T);
}
```

`fp` points into an mmap'd (or freshly allocated) file buffer. The PIGO binary
formats write a **variable-length header string immediately followed by packed
multi-byte integers with no padding**. For example, `COO::save`
(`coo.impl.hpp:506-533`) emits:

```
[header string ...][uint8 L_size][uint8 O_size][L nrows][L ncols][L n][O m][...data]
```

The header string length is not a multiple of `alignof(L)` in general, so by the
time `read_bin_` reaches `f.read<L>()` for `nrows_`, `fp` is not `L`-aligned.
`*(T*)fp` is then a **misaligned load** — undefined behavior in C++ (`[expr.ref]`
requires the pointer to point to a suitably-aligned object). The same applies to
`write<T>` in `save`, and to CSR, DiGraph, and Tensor binary headers, which share
the identical `File::read<T>()` / `WFile::write<T>()` path.

## How it manifests

Undefined behavior; specifically:

- On x86-64 (which tolerates unaligned access in hardware) the code *happens* to
  work, so it went unnoticed. But it is still UB by the language standard, and a
  compiler is permitted to assume alignment and miscompile.
- Under `-fsanitize=undefined` the alignment checker reports it, e.g.:

  ```
  include/pigo/impl/pigo.impl.hpp:168:11: runtime error: load of misaligned
  address 0x... for type 'unsigned int', which requires 4 byte alignment
  ```

- On strict-alignment targets (some ARM/MIPS configurations, or x86 with
  `-mstrict-align`) it can **fault at runtime (SIGBUS)**.

### Reproduction

Build the test suite with UBSan and no recovery, then run it:

```
cmake -S . -B build-ub -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all"
cmake --build build-ub && (cd build-ub && ctest)
```

Every test that round-trips a binary file fails on the misaligned load:
`test_file_io`, `test_csr_bin`, `test_csr_weighted`, `test_digraph_bin`,
`test_coo_read`, `test_coo_write`, `test_weight_mtx` (7 of 26). The failure is
pre-existing at upstream `cf37ee5` — it is not introduced by any recent change;
it was simply never exercised under UBSan.

## Root cause

Reading/writing a multi-byte scalar through a reinterpreted `char*` that is not
guaranteed to satisfy `alignof(T)`. Correct code must not assume the buffer offset
is aligned.

## Fix

Use `std::memcpy` to move the bytes. `memcpy` has no alignment requirement and is
the standard, well-defined way to read/write a possibly-unaligned scalar;
mainstream compilers lower a fixed-size `memcpy` to a single (unaligned) load/store
on architectures that permit one, so there is **no runtime cost** on x86-64 and
**correct behavior** everywhere.

```cpp
template<class T> inline T read(FilePos &fp) {
    T res;
    std::memcpy(&res, fp, sizeof(T));
    fp += sizeof(T);
    return res;
}
template<class T> inline void write(FilePos &fp, T val) {
    std::memcpy(const_cast<char*>(fp), &val, sizeof(T));
    fp += sizeof(T);
}
```

`<cstring>` is already included by `pigo.impl.hpp`. `FilePos` is `const char*`, so
`write` casts away const to the writable buffer exactly as the original `(T*)fp`
store did (the buffer is writable in the `WFile` path; the const on `FilePos` is a
pre-existing type choice, not affected by this fix).

## Why not "align the format instead"

Padding the binary header to align every field would change the on-disk format and
break compatibility with existing PIGO binary files. The `memcpy` fix keeps the
format byte-for-byte identical and only corrects *how* the bytes are read, so old
files still load and files written by the fixed code are still readable by any
consumer that reads them correctly.

## Scope

- Fix is entirely within `read<T>` / `write<T>` in
  `include/pigo/impl/pigo.impl.hpp`, regenerated into `release/pigo.hpp`.
- Affects all binary readers/writers (COO, CSR, DiGraph, Tensor) uniformly, since
  they all funnel through these two functions.
- The ASCII write path (`write_ascii`, `pigo.impl.hpp:289`, a `char*` store of one
  byte) is alignment-1 and unaffected.
- Independent of the trailing-newline fix in
  `ISSUE-tensor-missing-trailing-newline.md` (that one is in the text edge-list
  path; this one is in the binary path).

## Regression test

Building the suite with `-fsanitize=undefined -fno-sanitize-recover=all` and
observing all 26 tests pass is itself the regression guard: on unfixed code the 7
binary tests abort on the misaligned load, and on fixed code they pass. Adding
UBSan to the CI/sanitizer build makes this permanent.
