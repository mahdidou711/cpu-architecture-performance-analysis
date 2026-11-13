# Timing methodology

This document describes the timing protocol, the measurement unit, the
statistics, and the limits of the benchmark programs in this repository. The
reference results in [`../results`](../results) come from a longer campaign
protocol than the one the programs implement; section 7 states the
differences. The numbers are not a substitute for a complete performance
study.

## 1. Unit and scope

The native unit is the increment of the Time Stamp Counter (`TSC`), reported
as `TSC_ticks`. The normalised value is `ticks / denominator`, where the
denominator is `N^2` for zeroing, copies, additions, and dot product, and
`N^3` for the three matrix multiplications. One iteration of a dot product
contains one multiplication and one addition; the result is therefore not a
number of ticks per FLOP.

An invariant TSC is a reference clock, not a hardware cycle counter: turbo,
frequency variation, SIMD, and hybrid cores prevent a direct identification
of ticks with core cycles. Never multiply these ticks by the inverse of the
instantaneous MHz reported by `/proc/cpuinfo`.

The reference platform, an Intel Core i5-1335U, exposes `constant_tsc`,
`nonstop_tsc`, and `rdtscp`, reports an invariant TSC through CPUID, and Linux
uses `tsc` as the clocksource. CPUID leaf 0x15 exposes a ratio and a crystal
frequency; their product (130/2 x 38.4 MHz = 2.496 GHz) is the architectural
TSC frequency announced by the vendor. The raw results remain in ticks. A
conversion to seconds must cite its frequency and its origin; no effective
core frequency is measured here. Other machines must be checked with
`cpu_probe`, which reports the invariant-TSC bit; the invariance is a property
of the platform, not of the code.

## 2. Hardware and compiler framing

The timer is `tsc.h`. It exposes `start_timer`, `stop_timer`, and `dtime`.
Each endpoint uses `MFENCE; RDTSCP; LFENCE`, with EAX/EDX/ECX outputs and the
GCC `memory` clobber.

According to the Intel manual, RDTSC alone does not wait for earlier
execution and does not prevent later execution. RDTSCP waits for earlier
execution and loads, but not for the global visibility of stores; MFENCE
before it covers the latter point, and LFENCE after it prevents later
instructions from crossing the timestamp. The chosen protocol therefore
includes the draining of stores, not just their emission. It does not measure
a physical write to DRAM.

The CPUID check verifies SSE2 and RDTSCP before the program and the probes.
The qualified domain is an x86-64 Linux host, not a universal guarantee for
all x86 processors or hypervisors.

The `memory` clobber bounds the compiler's memory accesses; it does not
replace the hardware barriers. The inputs, outputs, and `SF` are global
memory. The compiler cannot assume their state is constant across the
barriers. The outputs are consumed by a checksum printed outside the
interval; no arithmetic data of the kernels is `volatile`. The checksum
provides observability; it does not replace element-by-element tests. The
outputs of each sample must be produced before the end barrier, even if only
the last one is printed.

Code generation depends on the compiler and the options. With GCC 13.3.0 on
the public sources (`-DN=64 -DM=16 -DBL=7`, checked with
`objdump -dr -Mintel --disassemble=<function>` on the object file):

| Function | `-O0`, `-O1` | `-O2`, `-O3` |
| --- | --- | --- |
| `zero` | scalar store loop | loop replaced by a call to `memset` |
| `ps` (float) | scalar | packed `mulps` for the products, scalar `addss` for the sum (the sum order is kept because floating-point addition is not reassociated without `-ffast-math`) |
| `mm_ikj` (float) | scalar | packed `mulps` in the inner loop; many more at `-O3` (unrolled) |

These transformations are reported, not hidden by `volatile`. Any new
combination of size, type, options, LTO, fast-math, or compiler requires an
inspection of its own binary; the programs do not guarantee future binaries.

## 3. Data state

A run selects one procedure: `zero`, `copy_ij`, `copy_ji`, `add_ij`,
`add_ji`, `ps`, `mm_ijk`, `mm_ikj`, or `mm_b_ijk`. `init_inputs()` initialises
deterministic 0/1 data before the series. Repetitions follow in the same
process; after the first, the data may be in cache. This protocol describes
repetitions on reused buffers, not a guaranteed cold cache. Initialisation,
printing, and the final checksum are outside the interval.

The additions keep in-place semantics: `AF += YF` at each sample, so
`AF_initial + M * YF` at the end of a call. Compare `ij` and `ji` in two runs
with the same initialisation and the same `M`. The driver refuses
conservative bounds that exceed the `TYPE` capacity: `N` for matrices, `N^2`
for the dot product, `M+1` for the addition on its own 0/1 inputs. This check
does not validate arbitrary data supplied by another caller. The tests use
bounded inputs: no overflow-free promise for arbitrary integer values. The
floating variants may round differently, especially through block-sum
grouping and the four-accumulator dot product (`ps4`).

`N`, `M`, and `BL` must be positive; `N <= 46340` protects `N*N` and the
`int` indices. `BL <= INT_MAX - N` protects partial increments and bounds.
Arithmetically admissible dimensions may exceed memory or linkage limits;
they are not all executable on a given host.

## 4. Repetitions and statistics

The programs print the raw samples of each call and leave the summary to the
reader. The default is `M=16` (the Makefile and `tp1.c`); the reference
campaigns used `M=51` for the small kernels and `M=3` for the large matrix
multiplications, whose single call lasts up to several seconds. In every case
the first sample is treated as warm-up. Do not stop a series at the first
favourable value. Keep every raw measurement, `N`, `M`, `BL`, `TYPE`, GCC
options, binary identity, and affinity condition.

The rules below are those used to build the reference tables; the summariser
itself is not distributed with this repository.

1. Reject as invalid the rows with different AUX tags, a zero `valid` flag,
   non-finite or negative ticks, or a non-positive denominator. A rejected
   row stays in the raw data.
2. Remove the first sample, then compute the first and third quartiles `Q1`
   and `Q3` with inclusive linear quantiles: for `n` sorted values `x_0 <= ...
   <= x_(n-1)` and a probability `p`, let `h = (n-1) p`, `k = floor(h)`; then
   `Q(p) = x_k + (h - k) (x_(k+1) - x_k)`.
3. Mark as high outliers the values above `Q3 + c * IQR`, with `IQR = Q3 - Q1`.
   The reference tables use `c = 1.5` for the GCC-level data and `c = 3` for
   the copy data. If `IQR = 0`, do not apply an automatic rejection. No
   exclusion of low values. With only two retained samples per launch (large
   matrix campaign) no outlier rule is applied inside a launch.
4. Report the median of the retained values (per launch), the minimum, the
   maximum, the IQR, the sample counts, and the classification of each
   sample. Fewer than five valid or retained measurements fail the
   processing of a launch whenever `M` allows more.
5. When launches are repeated, summarise a configuration by `S1`, the median
   over launches of the per-launch median (and `S2`, the median of the
   per-launch minima, as a sensitivity check). Compare two configurations
   with the median of paired per-round ratios and a bootstrap interval; the
   ratio `S1(a) / S1(b)` is a different quantity and must be named as such
   (see [`../results/README.md`](../results/README.md)).

The Tukey threshold is an operational convention, not a proof that an
interruption caused each removed value. A high rejection rate or a strong
dispersion must lead to examining and repeating the series, not to hiding
the noise. Small dispersion is compatible with quantisation, placement, and
microarchitectural states; large delays are compatible with interrupts,
scheduling, or contention. These causes are not individually demonstrated by
the ticks alone. Do not automatically attribute a difference to caches.

## 5. Instrumentation overhead and very short operations

The empty probe records 1001 pairs without a constant correction: `dtime`
only performs an unsigned difference and a conversion to `double`. The time
includes the framing between reads, the handling of the initial timestamp,
and some harness accesses. The empty probe does not have exactly the same
code as each kernel: subtracting its median would not give a universal
correction.

Before a series, compare its raw duration with the empty cost of the same
configuration. The reference campaigns required a raw median at least 100
times the empty median (indicative contribution of about 1 %, not a proven
error bound). Below that, mark the measurement as dominated or significantly
affected by the instrumentation; increase the size or prepare a grouped
measurement with inner repetitions and an explicitly adapted denominator. Do
not use a negative corrected result and do not declare a speed-up on that
basis. In the published tables the only rows below the criterion are the
float `copy_ij` / `copy_ji` N=256 rows, flagged
`duration_at_least_100x_timer_cost = 0`.

## 6. Hybrid machine and affinity

The `cpu_probe` program prints the current logical processor, the TSC AUX
tag, the brand, the CPUID.1A core type, the native model ID, the CPUID.15
ratio, the CPUID.16 MHz, the CPUID.6 EAX, the CPUID.7 hybrid bit, the
invariant TSC bit, and the cache descriptors as one JSON line. Run it under
`taskset` to pin the affinity.

The SMT sibling, the system load, the temperature, the power, and the turbo
are not controlled by the programs. The affinity is the main protection.
Migrations are not treated as simple high values.

Each sample keeps `IA32_TSC_AUX` at the start and at the end. Equality does
not prove the absence of a round-trip migration or a preemption; AUX is a
system-defined tag, not a portable identifier.

## 7. Reference results versus the public harness

| Aspect | `make run` / `make bench` | Reference campaigns |
| --- | --- | --- |
| Processes | one process, `M` samples | 30 launches (rounds) per configuration |
| Order | fixed | random order of all configurations in each round |
| Affinity | none unless `taskset` is used | one performance core, `taskset` |
| Warm-up | first sample printed, discarded by the reader | first sample discarded |
| `M` | 16 by default | 51 or 3 |
| Environment sensors | none | package temperature and frequency state logged |
| Summary | raw CSV only | `S1`, `S2`, paired ratios, bootstrap intervals |

The reference values are therefore archived results. The repository does not
claim that `make` regenerates them. A reader can obtain the same kind of
data for their own machine with a shell loop (pinning to the core of their
choice, here core 0):

```sh
make N=256 M=5
BIN=$(make N=256 M=5 print-bin-dir)
mkdir -p results/local
for round in $(seq 1 30); do taskset -c 0 "$BIN/tp1" mm_ikj; done > results/local/mm_ikj_30_launches.csv
```

and summarise the 30 launches with the rules of section 4.

## 8. Reproduction

From the repository root:

```sh
make                 # build the programs (N=1000, TYPE=float, BL=32, M=16)
make test            # 66 functional checks, a few seconds
make test-sanitize   # the same checks with ASan and UBSan, built separately
make check           # tests plus a smoke run of every program, about 15 s
make bench           # quick tp1 run, N=32, BL=8, M=4
make bench-full      # tp1 at N=1000, BL=32, M=16 (duration depends on the hardware and conditions)
```

`make check` runs the functional tests, the TSC probe, the overhead probe,
the CPU probe, two latency probes, and the matrix extras at reduced size
(`CHECK_N=64`). The build directory name encodes `CC`, `CFLAGS`, `N`, `TYPE`,
`BL`, and `M`; `make print-bin-dir` prints it.

## 9. Technical sources

- Intel SDM, volume 2, RDTSC and RDTSCP entries.
- GCC, Extended Asm, Volatile and Clobbers sections.
