# `File` move constructor leaves the moved-to object broken and double-frees

## Summary

`File`'s move constructor copied only two of the class's four members. It left
the file position `fp_` uninitialized and did not clear the moved-from object's
mapping pointer, so:

- reading through a moved-to `File` dereferenced an uninitialized `fp_`
  (garbage pointer -- a crash, or a null-pointer deref under UBSan), and
- destructing both the source and the destination `munmap`'d the same region
  twice (a double free of the mapping).

The move-assignment operator cleared the source but likewise did not carry
`fp_` (or `fn_`) across.

## Mechanism

`File` owns `char* data_`, `size_t size_`, `FilePos fp_`, and
`std::string fn_`, with a destructor that `munmap`s `data_` when non-null.

The move constructor (`include/pigo.hpp`) was:

```cpp
File(File &&o) {
    data_ = o.data_;
    size_ = o.size_;
    // fp_ never set; o.data_ never cleared
}
```

`fp_` is where reads and writes advance (`File::read`/`File::write` operate on
`fp_`). Leaving it uninitialized means the moved-to `File` cannot be read from.
And because `o.data_` still points at the mapping, `o`'s destructor unmaps it
in addition to the destination's -- a double `munmap` of the same address.

The move-assignment operator (`pigo.impl.hpp`) cleared `o.data_` but did not
copy `fp_`/`fn_`:

```cpp
data_ = o.data_; size_ = o.size_; o.data_ = nullptr;   // fp_, fn_ not moved
```

Reproduced by move-constructing a `File` from an open `ROFile` and reading the
first value through it: under `-fsanitize=undefined`, a
"null pointer passed as argument 2" runtime error at the read (an
uninitialized `fp_` that happened to be null); without sanitizers this is a
segfault, and the double `munmap` corrupts the mapping bookkeeping regardless.

## Fix

Transfer all members and null the source's mapping in the move constructor,
and carry `fp_`/`fn_` in the move-assignment operator:

```cpp
File(File &&o) {
    data_ = o.data_;
    size_ = o.size_;
    fp_ = o.fp_;
    fn_ = std::move(o.fn_);
    o.data_ = nullptr;
}
```

```cpp
// operator=(File&&): after copying data_/size_
fp_ = o.fp_;
fn_ = std::move(o.fn_);
o.data_ = nullptr;
```

## Scope

- Fix touches the move constructor in `include/pigo.hpp` and the move
  assignment in `include/pigo/impl/pigo.impl.hpp`, regenerated into
  `release/pigo.hpp`.
- `ROFile`/`WFile` inherit these and are covered by the same fix.
- Regression test `tests/base/test_file_io.cpp::move_semantics` move-constructs
  and move-assigns a `File`, reads known values through the destination (which
  requires `fp_` to have been carried), and lets both objects destruct (which
  would double-`munmap` before the fix). It faults on the prior code and passes
  on the fix.
