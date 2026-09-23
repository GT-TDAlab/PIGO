# ASCII write corrupts (and overruns on) the most-negative signed value

## Summary

Writing a signed integer whose value is the most-negative representable
(`INT8_MIN` = -128, `INT16_MIN`, `INT32_MIN`, `INT64_MIN`) produces **garbage
bytes** instead of the correct decimal, and in bulk **overruns the output file**
(heap/mmap buffer overflow). Affects `COO::write` and `Tensor::write` — every
ASCII (edge-list / `.tns`) write path for signed coordinate or weight types.

## Mechanism

`detail::get_positive` (`include/pigo/impl/pigo.impl.hpp`) computed the magnitude
by negating within the signed type:

```cpp
template<...signed...> T get_positive(T obj) {
    if (obj < 0) return -obj;   // <-- UB / wrong for T_MIN
    return obj;
}
```

For the most-negative value, `-obj` is not representable in `T`; the result wraps
back to the same negative value (`int8_t -128` stays `-128`). This is signed
overflow (undefined behavior), and it leaves a negative magnitude. `write_ascii`
then did:

```cpp
obj = detail::get_positive(obj);   // still negative for T_MIN
size_t num_size = write_size(obj);
do {
    *((char*)fp+pos--) = (obj%10)+'0';   // (negative % 10) + '0' => non-digit byte
    obj /= 10;
} while (obj != 0);
```

Two things go wrong:

1. **Corruption.** `obj % 10` on a negative value is negative, so `(obj%10)+'0'`
   writes bytes below `'0'` — punctuation/control characters. `-128` came out as
   `-\x00/.(` rather than `-128`.
2. **Size disagreement → overrun.** The independent sizing pass in
   `COO::write` / `Tensor::write` calls `write_size` on the *original signed*
   value (correct), while the digit loop above derives its own `num_size` from
   the corrupted magnitude. The two can differ, so pass 2 writes more bytes than
   pass 1 reserved. With enough most-negative entries the write runs past the
   end of the `mmap`; the overrun magnitude is optimization-level dependent
   (classic UB). A 2048-entry `int8_t` COO of `-128` reliably crashes under
   AddressSanitizer.

### Reproduction

```cpp
COO<int8_t,int,vector<int8_t>> c(1,1,2,2);
c.x()[0]=-128; c.y()[0]=-128; c.x()[1]=-1; c.y()[1]=-1;
c.write("out.el");
// out.el contains "- \x00/.( - \x00/.(\n-1 -1\n" instead of "-128 -128\n-1 -1\n"
```

## Fix

Never negate into the signed type. Compute the magnitude in the corresponding
`std::make_unsigned<T>::type`, where `0 - (unsigned)obj` is well-defined modular
arithmetic and yields the true magnitude for every value including `T_MIN`.
`get_positive` now returns the unsigned type, and `write_ascii` holds the
magnitude in an unsigned local so it is never re-truncated back into `T`:

```cpp
template<...signed...>
typename std::make_unsigned<T>::type get_positive(T obj) {
    typedef typename std::make_unsigned<T>::type U;
    if (obj < 0) return (U)0 - (U)obj;
    return (U)obj;
}

// write_ascii:
detail::write_neg_ascii(fp, obj);                       // sign first
typename std::make_unsigned<T>::type mag = detail::get_positive(obj);
size_t num_size = write_size(mag);                      // digits only, no sign
do { *((char*)fp+pos--) = (mag%10)+'0'; mag /= 10; } while (mag != 0);
```

`write_size(mag)` (unsigned) counts digits only, exactly matching the digits
emitted; the caller's sizing pass calls `write_size` on the signed value, which
adds one for the sign via `neg_size` — so total reserved (sign + digits) equals
total written (`write_neg_ascii` + digits). `<type_traits>` is already available
through the existing includes.

## Scope

- Fix is confined to `get_positive` and `write_ascii` in
  `include/pigo/impl/pigo.impl.hpp`, regenerated into `release/pigo.hpp`.
- Unsigned types are unaffected (their `get_positive`/`write_size` overloads are
  unchanged, and no unsigned value triggers the negate).
- Float writes go through `stbsp_to_chars`, a separate path, and are unaffected.
- Regression test: `tests/coo/test_coo_write.cpp::signed_min_ascii` writes
  `INT8_MIN` and `INT32_MIN` and checks the output bytes exactly. It fails on the
  prior code (garbage bytes) and passes on the fix.
