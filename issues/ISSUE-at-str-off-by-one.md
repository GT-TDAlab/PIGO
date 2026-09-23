# `FileReader::at_str` fails to match a string ending at end-of-input

## Summary

`FileReader::at_str(s)` reports whether the reader's current position begins
with the string `s`. Its bounds check rejected the case where `s` ends exactly
at the end of the input, so a string that fills the reader to its final byte
was reported as absent even when it matched.

## Mechanism

`at_str` (`include/pigo/impl/pigo.impl.hpp`) compares the half-open range
`[d, d + s.size())` against `s`:

```cpp
if (d + s.size() >= end) return false;   // wrong: rejects d + s.size() == end
std::string d_str { d, d+s.size() };
return s.compare(d_str) == 0;
```

Reading `[d, d + s.size())` is in-bounds precisely when `d + s.size() <= end`.
The `>=` guard also rejected `d + s.size() == end`, i.e. a string that occupies
the reader right up to (but not past) its end. In that case `at_str` returned
`false` without ever comparing, so a legitimate match at end-of-input was
missed.

This surfaces for any input whose entire remaining content is exactly the
string being checked -- for example a file consisting solely of a format
header (`guess_file_type` calls `at_str` with each known header), or a header
that runs to the last byte with no trailing data.

## Fix

Use `>` so the in-bounds case `d + s.size() == end` is accepted:

```cpp
if (d + s.size() > end) return false;
```

The comparison itself is unchanged; only the off-by-one in the bounds guard is
corrected. No out-of-bounds read is introduced, since the range stays within
`[d, end]`.

## Scope

- Fix is confined to `FileReader::at_str` in
  `include/pigo/impl/pigo.impl.hpp`, regenerated into `release/pigo.hpp`.
- Regression test `tests/base/test_file_reader.cpp::at_str_exact_end` checks a
  string filling the reader exactly (must match), a longer string (must not),
  and a proper prefix (must match). The exact-end case fails on the prior code
  and passes on the fix.
