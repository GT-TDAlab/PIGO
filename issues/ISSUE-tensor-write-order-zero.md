# `Tensor::write` overruns the output allocation for order-0 tensors

## Summary

`Tensor::write` builds the output file in two passes: a first pass sizes the
allocation, and a second pass writes into it. The two passes disagreed for a
tensor whose entries have **no coordinates** (order 0), so the writing pass
emitted one byte per entry past the end of the mapped file -- a
heap-buffer-overflow of `m()` bytes.

An order-0 weighted tensor arises from a weighted edge-list file with a single
token per line (only a weight, no coordinates); this is also what a weighted
single-column file collapses to (see `ISSUE-tensor-order-eol-detection.md`).

## Mechanism

Both passes iterate every entry `e`. The sizing pass
(`include/pigo/impl/tensor.impl.hpp`) charged a separator byte after *every*
coordinate:

```cpp
for (O idx = 0; idx < order_; ++idx) {
    my_size += write_size(x);
    my_size += 1;               // separator after every coordinate
}
if (wgt) { my_size += 1; my_size += write_size(w); }
// nothing charged for the trailing newline
```

The writing pass emits a separator only *between* coordinates, and one
unconditional newline per entry:

```cpp
for (O idx = 0; idx < order_; ++idx) {
    write_ascii(my_fp, x);
    if (idx < order_-1) pigo::write(my_fp, ' ');   // order_-1 separators
}
if (wgt) { pigo::write(my_fp, ' '); write_ascii(my_fp, w); }
pigo::write(my_fp, '\n');                          // one newline, always
```

For `order_ >= 1` the two happen to balance: the sizing pass over-counts
separators by one (`order_` instead of `order_-1`), and that spare byte pays
for the newline the sizing pass never mentions. For `order_ == 0` the
coordinate loop never runs, so no spare separator is counted, nothing pays for
the newline, and the allocation is short by exactly one byte per entry. The
writing pass then overruns by `m()` bytes.

Reproduced with a 20000-line weighted single-column file under
`-fsanitize=address`: a `heap-buffer-overflow` WRITE of size 4 in
`stbsp_to_chars` via `Tensor::write` (`pigo.impl.hpp:330` /
`tensor.impl.hpp:386`). Fewer entries do not fault because the overrun stays
inside `mmap`'s page rounding; 20000 pushes the deficit past a page.

## Fix

Make the sizing pass mirror the writing pass exactly, rather than relying on an
over-count coincidentally cancelling an omission: count a separator only
between coordinates, and add one byte for the trailing newline.

```cpp
for (O idx = 0; idx < order_; ++idx) {
    my_size += write_size(x);
    if (idx < order_-1)
        my_size += 1;           // separating space, matching pass 2
}
if (wgt) { my_size += 1; my_size += write_size(w); }
my_size += 1;                   // trailing newline, matching pass 2
```

Now the two passes agree for every `order_`, including 0. The byte layout for
`order_ >= 1` is unchanged.

## Scope

- Fix is confined to the sizing loop of `Tensor::write` in
  `include/pigo/impl/tensor.impl.hpp`, regenerated into `release/pigo.hpp`.
- Binary writes (`Tensor::save`) size the allocation from `sizeof` products,
  not this loop, and are unaffected.
- `COO`/`CSR` ASCII writers have their own sizing loops and are not touched
  here.
- Regression test `tests/tensor/test_tensor.cpp::write_order_zero` writes and
  reads back a 20000-entry order-0 weighted tensor. It faults on the prior code
  under ASan and passes on the fix.
