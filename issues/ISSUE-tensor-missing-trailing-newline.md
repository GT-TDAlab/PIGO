# Files not ending in a newline are read incorrectly

Two distinct bugs, both caused by the same root cause: the entry readers use
`r.good()` (i.e. `d < end`) to decide whether an entry was *successfully* read,
but `good()` actually reports whether there is *more input remaining*. On the
final line of a file that does not end in `'\n'`, those two conditions differ.

| Class | Symptom | Severity |
|---|---|---|
| `Tensor` (`weighted=false`) | heap-buffer-overflow, then `m()` reports one entry too many | memory corruption |
| `COO` (unweighted) | last entry silently set to garbage (uninitialized / `0 -> 0`) | silent data corruption |

Both reproduce single-threaded (`OMP_NUM_THREADS=1`), so this is not a
parallel-decomposition issue.

## Root cause

`FileReader::good()` is defined as:

```cpp
bool good() { return d < end; }
```

An entry reader finishes by consuming its integers and then advancing:

```cpp
r.move_to_eol();        // stops AT the '\n', or at end if there is none
r.move_to_next_int();   // moves past it to the next digit, or to end
```

When the file ends without a trailing newline, the reader consumes the last
line's integers and `d` reaches `end`. So `good()` becomes false — but the
entry *was* fully present and valid. The readers interpret "no input left" as
"the read failed", and the two passes disagree about whether that final entry
counts.

The two-pass structure is what turns that disagreement into a bug. Pass 1
counts entries to size the allocation; pass 2 writes them. They must agree
exactly.

### Bug 1 — `Tensor`: heap-buffer-overflow

`include/pigo/impl/tensor.impl.hpp`. The count-only reader (`order_` = number
of columns, `wgt=false`):

```cpp
for (O idx = 0; idx < order_; ++idx) {
    ...
    r.read_int<L>();
    if (!r.good()) return;        // <-- returns WITHOUT ++coord_pos
}
r.move_to_eol();
r.move_to_next_int();
++coord_pos;
```

and the setting reader:

```cpp
for (O idx = 0; idx < order_; ++idx) {
    L v = r.read_int<L>();
    if (!r.good()) return;        // <-- returns, but AFTER set_value_ of
    set_value_(c_, coord_pos*order_+idx, v);   //   earlier idx values
}
```

The asymmetry: the check sits *before* `set_value_`, so pass 1 aborts the
final entry without counting it, while pass 2 has already written
`order_ - 1` of its coordinates into a buffer sized for one entry fewer.

`read_int` consumes the final digits and leaves `d == end`, so `good()` is
false on the *last* coordinate of the last line. Pass 1 undercounts by one;
`allocate_()` therefore allocates `order_*(m-1)`; pass 2 writes at
`coord_pos*order_+idx` past the end of that buffer.

Confirmed with AddressSanitizer on a 3-line, 4-column file with no trailing
newline:

```
==655==ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 8 at 0x503000000060
    #0 pigo::Tensor<...>::read_el_(pigo::FileReader&)
0x503000000060 is located 0 bytes after 32-byte region
```

Without ASan it silently corrupts the heap; one probe aborted inside glibc:

```
Fatal glibc error: malloc.c:2599 (sysmalloc): assertion failed
```

The reported size is also wrong — `m()` ends up one greater than pass 1's
count, since pass 2 increments `coord_pos` past the allocated extent. This is
very likely the "bug in tensor size" item in the project's notes.

The weighted path (`weighted=true`) is unaffected: it ends each entry with
`read_wgt_`, whose `move_to_next_int` leaves `d` positioned *on* the final
integer rather than past it, so `good()` stays true.

### Bug 2 — `COO`: silent garbage in the last entry

`include/pigo/impl/coo.impl.hpp`. The unweighted count-only specialization
does not parse the coordinates at all — it just skips a line:

```cpp
static inline void op_(S&, S&, WS&, size_t &coord_pos, FileReader &r, L&, L&) {
    r.move_to_next_int();
    if (!r.good()) return;
    ...
    ++coord_pos;
}
```

`move_to_next_int()` from the start of `x` lands on `y`, which is not at
`end`, so `good()` is still true and pass 1 *does* count the final entry —
correctly. But the setting specialization reads *both* integers:

```cpp
L x = r.read_int<L>();
r.move_to_next_int();
L y = r.read_int<L>();
read_wgt_<wgt, W, WS, false>(coord_pos, w_, r);
if (!r.good()) return;            // <-- returns BEFORE set_value_
set_value_(x_, coord_pos, x);
set_value_(y_, coord_pos, y);
```

After reading `y` on the last line, `d == end`, so this returns *before*
storing `x` and `y`. Pass 1 counted the entry and sized the buffer for it, so
there is no overflow — the slot is simply never written, and the caller sees
whatever the allocation happened to contain.

So the count is right and the data is wrong, which makes this the more
dangerous of the two: there is no crash to notice.

Reproduced with a 3-edge file `0 1\n1 2\n2 3` (no trailing newline):

```
m=3
    0 -> 1
    1 -> 2
    0 -> 0      <-- should be 2 -> 3
```

The same pattern appears in the general (`sym`/`ut`/`sl`) specializations,
which place `if (!r.good()) return;` before their `set_value_` calls too.

## Fix

Stop using "is there input left" as a proxy for "did the read succeed".

The entry readers already know how many integers an entry requires, so they can
just check whether they got them. `FileReader` is extended with:

```cpp
/** @brief Return whether the reader has any input remaining */
bool at_end() { return d >= end; }
```

and each reader tracks whether it consumed a complete entry, treating "reached
end of input having parsed a full entry" as success and only bailing out when
an entry is genuinely truncated. Concretely:

- `Tensor`: hoist `set_value_` so a coordinate is stored as soon as it is
  parsed, and decide entry validity by counting parsed coordinates
  (`idx == order_`) rather than by `good()`. Both passes then agree.
- `COO`: move the `if (!r.good()) return;` guard to *after* the `set_value_`
  calls, matching the count-only pass, which already counts the entry.

A complete entry at end-of-input is then counted and stored by both passes,
and a genuinely truncated trailing line is consistently rejected by both.

### Why not just "append a newline if missing"

Requiring a trailing newline would be an API-visible input restriction, and a
memory-corrupting one when violated. Files without a trailing newline are
completely ordinary; `COO`'s count pass and every other reader already handle
them. Rejecting them explicitly would also be a breaking change for callers
who currently read such files successfully.

## Regression tests

Added to `tests/tensor/test_tensor.cpp` and `tests/coo/test_coo_read.cpp`,
with data files that deliberately lack a final newline. Since git and many
editors normalize trailing newlines, the tests **write their own input files at
runtime** rather than relying on a checked-in fixture keeping its exact final
byte.

Covered:
- unweighted `Tensor`, 4 columns, no trailing newline: correct `m()`, `order()`
  and all coordinates (the ASan crash case)
- unweighted `Tensor`, order 1, no trailing newline
- weighted `Tensor`, no trailing newline (guards the currently-working path)
- unweighted `COO`, no trailing newline: final edge has correct endpoints
  (the silent-corruption case)
- both with and without a trailing newline, asserting identical results
- multi-threaded as well as single-threaded

## Scope / affected versions

Present in v0.6 and in `release/pigo.hpp` at `v0.6-5-gcf37ee5`. `Tensor` was
introduced in v0.6, so the overflow dates from that release; the `COO` path is
older.

Not affected: binary readers (`read_bin_`), the matrix-market reader (it has an
explicit entry count), and the weighted `Tensor` path.
