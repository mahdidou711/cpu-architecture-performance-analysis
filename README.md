# Processor Performance Benchmarks

A compact, self-contained C benchmark suite for teaching processor
performance. It covers matrix kernels, memory access patterns, and TSC-based
timing, together with functional tests and a small methodology document.

The suite is intentionally small and dependency-free. It is meant to be read,
compiled, and run on a single x86-64 Linux host.

## Features

- **Matrix kernels**: zeroing, row/column copies, row/column additions,
  dot product, and matrix multiplication in `ijk`, `ikj`, and blocked
  `mm_b_ijk` variants.
- **Memory access patterns**: `ij` and `ji` traversals, transposition, and
  transposed multiplication.
- **Timing**: `RDTSC`/`RDTSCP` with `MFENCE`/`LFENCE` barriers, TSC ticks
  normalised by the operation count.
- **Functional tests**: independent long-double oracles, repeated calls,
  input-preservation checks, and an optional ASan/UBSan run.
- **Latency probes**: explicit x86 assembly for integer and floating-point
  add, multiply, divide, and empty-loop chains.
- **Platform probes**: CPUID, TSC, cache, and overhead characterisation.

## Requirements

- x86-64 Linux with SSE2 and `RDTSCP`.
- A C compiler (GCC is the default; `CC` can be overridden).
- GNU Make.
- `-lm` (standard maths library).

## Build

```sh
make
```

The default build uses `N=1000`, `TYPE=float`, `BL=32`, and `M=16`. The
build outputs are written to `build/bin/`.

## Run

```sh
make run          # run the default operation set
make bench        # short run, N=32, BL=8, M=4
make bench-full   # full run, N=1000, BL=32, M=16
```

Each run prints one CSV block per operation, with a header line, the raw
samples, and a final checksum. The timing unit is the TSC increment; see
[docs/METHODOLOGY.md](docs/METHODOLOGY.md).

## Tests

```sh
make test          # functional tests
make test-sanitize # functional tests with ASan/UBSan
make check         # tests plus a short smoke benchmark
```

The tests build the 12 variants (float, int, double; `N=32` and `N=33`)
and check the kernels against independent long-double oracles. The test
target prints `PASS`/`FAIL` lines to stderr; the benchmark's stdout is
suppressed.

## Operations

| Program | Operations |
| --- | --- |
| `tp1` | `zero`, `copy_ij`, `copy_ji`, `add_ij`, `add_ji`, `ps`, `mm_ijk`, `mm_ikj`, `mm_b_ijk` |
| `matrix_extras` | `transpose`, `trans_mul`, `trans_total`, `ps4`, `mm_ijk`, `mm_ikj`, `mm_b_ijk`, `ps` |
| `latency` | `int_add_1`, `int_add_4`, `int_mul_1`, `int_mul_4`, `int_div_1`, `int_div_4`, `int_empty_1`, `int_empty_4`, `float_add_1`, `float_add_4`, `float_mul_1`, `float_mul_4`, `float_div_1`, `float_div_4`, `float_empty_1`, `float_empty_4`, `int_divempty_1`, `int_divempty_4` |
| `tsc_probe` | empty TSC overhead, CPUID.15, cross-check against `CLOCK_MONOTONIC_RAW` |
| `overhead_probe` | TSC instrumentation overhead vs. dependent-add chain |
| `cpu_probe` | CPUID, brand, core type, caches, invariant TSC, as one JSON line |

## Project structure

```
src/        C sources
tests/      functional tests
docs/       methodology
results/    generated results (empty)
Makefile    build, test, and run targets
```

## Methodology

The timing harness uses `RDTSC`/`RDTSCP` with explicit memory barriers. The
native unit is the TSC increment, not a core cycle. The TSC is invariant
and is not a hardware cycle counter; turbo, frequency scaling, SIMD, and
hybrid cores prevent a direct mapping. The suite reports raw TSC ticks and
normalises by the operation count. The raw samples are retained, including
invalid or migrated ones.

See [docs/METHODOLOGY.md](docs/METHODOLOGY.md) for the timing protocol,
the statistics, and the limits.

## License

MIT. See [LICENSE](LICENSE).

## Notes

- The suite is single-threaded and uses only x86-64 features.
- The benchmark is intended for teaching and for relative comparisons on
  the same host, not for absolute performance claims.
- The code is not a substitute for a full performance-engineering study.
