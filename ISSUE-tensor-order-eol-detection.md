# Tensor order detection is wrong at end-of-file and for CRLF files

## Summary

`Tensor` determines its order (number of coordinate columns) by reading the
first line of the file with `FileReader::count_spaces_to_eol()`. That function
recognizes only `\n`, `%` and `#` as ending a line — not end-of-file and not a
carriage return. As a result:

- A `.tns` file whose **first line has no trailing newline** reports one column
  too many. For a single-line file this also makes it read as **empty**
  (`m() == 0`).
- A **CRLF-terminated** file (`\r\n`) reports one column too many on *every*
  line, since `\r` is never treated as end-of-line.
- For a **weighted** single-column file, the miscount drives `order_` to 0,
  which triggers a separate crash in `Tensor::write` (see
  `ISSUE-tensor-write-order-zero.md`).

`COO`/`.el` reading does not use `count_spaces_to_eol` and is unaffected.

## Mechanism

`count_spaces_to_eol()` (`include/pigo/impl/pigo.impl.hpp`) counts one separator
after each integer, then un-counts the trailing one when the next character
ends the line:

```cpp
while (*d == ' ') { ++d; };
if (*d == '\n') { --space_ct; break; }      // only '\n'
if (*d == '%' || *d == '#') { --space_ct; move_to_eol(); break; }
```

End-of-file (`d == end`) and `\r` are not in that set, so the trailing separator
is left counted. `Tensor::read_el_` then uses this count directly as `order_`
(adding one for the unweighted case), so the order is one too high. With the
order wrong, the two-pass reader's per-line stride no longer matches the data
and `m()` collapses (to 0 for a one-line file).

Measured on the prior code:

| input | reported order | correct |
|---|---|---|
| `"1 2 3 4"` (no newline) | 5 | 4 |
| `"19"` (no newline) | 2 → reads empty | 1 |
| `"1 2 3\r\n4 5 6\r\n"` | 4 | 3 |

The existing no-trailing-newline tests all have a `\n` on the *first* line, so
they exercised the entry readers but never `count_spaces_to_eol`, which reads
the first line only — hence this went uncaught.

## Fix

Treat end-of-file and `\r` as end-of-line everywhere the function decides
whether a separator was trailing, and guard the space-skip loop against reading
past `end`:

```cpp
if (d >= end || *d == '\n' || *d == '\r') break;   // before counting a separator
...
while (d < end && *d == ' ') { ++d; };
if (d >= end || *d == '\n' || *d == '\r') { --space_ct; break; }  // un-count
```

Coordinate values are unaffected because `read_int` already stops at any
non-digit, so `\r` never leaks into the last integer — this fix is purely about
the separator count that yields the order.

## Scope

- Fix is confined to `FileReader::count_spaces_to_eol` in
  `include/pigo/impl/pigo.impl.hpp`, regenerated into `release/pigo.hpp`.
- `COO`/edge-list reading does not call this function and is unchanged.
- Regression test `tests/tensor/test_tensor.cpp::read_order_eol_variants`
  covers a single-line no-newline file, a CRLF file, and a CRLF file with no
  final newline (order, count, and read-back values). It fails on the prior
  code (order one too high) and passes on the fix.
