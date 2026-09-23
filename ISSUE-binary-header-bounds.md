# Binary reads trust the header's size and can read out of bounds

## Summary

When reading a PIGO binary file (`COO`, `CSR`, `DiGraph`, `Tensor`), the entry
count (`m_`, and the analogous sizes) is taken directly from the file header
and multiplied by the element size to decide how many bytes to copy out of the
memory-mapped file. `File::parallel_read` then `memcpy`s that many bytes with
no check that they actually lie within the file. A corrupt or truncated header
declaring more data than the file holds therefore reads past the end of the
mapping -- a segfault (observed under AddressSanitizer as a SEGV in `memcpy`),
or silently returns adjacent memory.

## Mechanism

`COO::read_bin_` (`include/pigo/impl/coo.impl.hpp`), and the equivalent
`CSR::read_bin_` and `Tensor::read_bin_`, do:

```cpp
m_ = f.read<O>();                 // trusted straight from the header
allocate_();
char* vx = detail::get_raw_data_<S>(x_);
size_t vx_size = sizeof(L)*m_;
f.parallel_read(vx, vx_size);     // memcpy of vx_size bytes from the mapping
```

`File::parallel_read` forwarded to `::pigo::parallel_read`, which splits the
region across threads and `memcpy`s each slice from `fp_ + start_pos` -- with no
comparison against the file's actual size. So an oversized `m_` copies from
well past `data_ + size_`.

Reproduced by saving a valid `COO` binary, overwriting the 4-byte `m_` field in
the header with `1e8`, and reading it back: AddressSanitizer reports a SEGV
(read memory access) inside `memcpy`, called from `parallel_read` via
`read_bin_`.

## Fix

Bounds-check in `File::parallel_read`, which is the single choke point every
binary reader funnels through and the only place that knows both the current
position `fp_` and the mapping extent `[data_, data_ + size_)`:

```cpp
size_t remaining = (size_t)(data_ + size_ - fp_);
if (v_size > remaining)
    throw Error("File is too small for the data it declares");
::pigo::parallel_read(fp_, v, v_size);
```

This covers `COO`, `CSR`, `DiGraph`, and `Tensor` binary reads at once, turning
a memory-safety violation into a thrown `pigo::Error`. Valid files are
unaffected: their declared sizes are exactly what the writer laid down, so
`v_size <= remaining` always holds.

## Scope

- Fix is confined to `File::parallel_read` in
  `include/pigo/impl/pigo.impl.hpp`, regenerated into `release/pigo.hpp`.
- `parallel_write` is not changed: it writes into a file the library itself
  sized in the same call path, not into attacker-controlled bytes.
- Regression test `tests/coo/test_coo_read.cpp::read_bin_corrupt_header` writes
  a valid binary `COO`, corrupts its `m_` to an enormous value, and asserts the
  read throws `pigo::Error`. It crashes on the prior code (OOB `memcpy`) and
  passes on the fix. Vector storage is used so the partially-built object frees
  cleanly when the constructor throws.
