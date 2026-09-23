# `parallel_write` takes a non-const source it never modifies

## Summary

`File::parallel_write` (and the underlying free function
`pigo::parallel_write`) declared their source-data parameter as `char*`, even
though the data is only read -- it is the source operand of a `memcpy` into the
output file. The non-const parameter forced callers holding read-only data to
cast away `const`, and prevented `save()`-style methods from accepting a
`const` source buffer.

## Mechanism

```cpp
void parallel_write(FilePos &fp, char* v, size_t v_size) {
    ...
    memcpy(wfp + start_pos, v + start_pos, my_data);   // v is only read
    ...
}
```

`v` is never written through; it is copied *out of*. Declaring it `char*` is
an over-broad requirement on the caller. Any attempt to pass a `const char*`
(for example a pointer into read-only storage, or the result of a const data
accessor) is rejected at compile time:

```
error: invalid conversion from 'const char*' to 'char*'
```

## Fix

Widen the parameter to `const char*` in both the `File` method and the free
function (and their declarations in `include/pigo.hpp`):

```cpp
void parallel_write(FilePos &fp, const char* v, size_t v_size);
void File::parallel_write(const char* v, size_t v_size);
```

This is source- and binary-compatible for existing callers: a `char*` argument
converts implicitly to `const char*`. The `memcpy` is unaffected -- its source
operand is already `const void*`. Only the caller-facing contract changes, now
correctly stating that the data is read, not modified.

## Scope

- Fix touches the two `parallel_write` declarations in `include/pigo.hpp` and
  their definitions in `include/pigo/impl/pigo.impl.hpp`, regenerated into
  `release/pigo.hpp`.
- `parallel_read` is unchanged: its buffer is genuinely written into.
- Making `save()` itself `const` is intentionally left out of scope: it would
  require const data accessors and const-qualifying the public API across every
  class, a broader (and potentially source-breaking) change with no runtime
  defect behind it. This fix corrects only the parameter that is provably
  read-only.
- Regression test `tests/base/test_file_io.cpp::parallel_write_const_source`
  passes a `const char*` buffer to `parallel_write` and reads the values back.
  It fails to compile against the prior `char*` signature and passes on the fix.
