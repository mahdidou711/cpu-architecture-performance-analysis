# Timing methodology

This document describes the timing protocol, the measurement unit, the
statistics, and the limits of the benchmark suite. It is written for the
code in this repository; the numbers are not a substitute for a complete
performance study.

## 1. Unit and scope

The native unit is the increment of the Time Stamp Counter (`TSC`), reported
as `TSC_ticks`. The normalised value is `ticks / denominator`, where the
denominator is `N^2` for zeroing, copies, additions, and dot product, and
`N^3` for the three matrix multiplications. One iteration of a dot product
contains one multiplication and one addition; the result is therefore not a
number of ticks per FLOP.

The "cycles" of the subject are reported with this unit qualification. An
invariant TSC is a reference clock, not a hardware cycle counter: turbo,
frequency variation, SIMD, and hybrid cores prevent a direct identification.
Never multiply these ticks by the inverse of the instantaneous MHz reported
by `/proc/cpuinfo`.

The observed processor exposes `constant_tsc`, `nonstop_tsc`, and `rdtscp`,
and Linux uses `tsc` as the clocksource. CPUID.15 exposes a ratio and a
crystal frequency; their product describes the architectural TSC frequency
announced by the vendor. The raw results remain in ticks. A conversion to
seconds must cite its frequency and its origin; no effective core frequency
is measured here.

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

The CPUID check verifies SSE2 and RDTSCP before the program and the probe.
The qualified domain is this x86-64 Linux host, not a universal guarantee for
all x86 or hypervisors.

The `memory` clobber bounds the compiler's memory accesses; it does not
replace the hardware barriers. The inputs, outputs, and `SF` are global
memory. The compiler cannot assume their state is constant across the
barriers. The outputs are consumed by a checksum printed outside the
interval; no arithmetic data of the kernels is `volatile`. The checksum
provides observability; it does not replace element-by-element tests. The
outputs of each sample must be produced before the end barrier, even if only
the last one is printed.

The O2/O3 disassemblies of the N=64, M=31, BL=7 configuration are recorded
during the study. At O3, zeroing becomes a `memset` call and the dot
product uses `mulps` plus scalar additions: these transformations are
reported, not hidden by `volatile`. Any new combination of size, type,
options, LTO, fast-math, or compiler requires an adapted inspection of its own
binary; the suite does not guarantee future binaries.

## 3. Data state

A run selects one procedure: `zero`, `copy_ij`, `copy_ji`, `add_ij`,
`add_ji`, `ps`, `mm_ijk`, `mm_ikj`, or `mm_b_ijk`. `init_inputs()` initialises
deterministic 0/1 data before the series. Repetitions follow in the same
process; after the first, the data may be in cache. This protocol describes
repetitions on reused buffers, not a guaranteed cold cache. Initialisation,
printing, and the final checksum are outside the interval.

The additions keep the pedagogical in-place semantics: `AF += YF` at each
sample, so `AF_initial + M * YF` at the end of a call. Compare `ij` and `ji`
in two runs with the same initialisation and the same `M`. The driver refuses
conservative bounds that exceed the `TYPE` capacity: `N` for matrices, `N^2`
for the dot product, `M+1` for the addition on its own 0/1 inputs. This check
does not validate arbitrary data supplied by another caller. The tests use
bounded inputs: no overflow-free promise for arbitrary integer values. The
floating variants may round differently, especially through block-sum
grouping.

`N`, `M`, and `BL` must be positive; `N <= 46340` protects `N*N` and the
`int` indices. `BL <= INT_MAX - N` protects partial increments and bounds.
Arithmetically admissible dimensions may exceed memory or linkage limits;
they are not all executable on this host.

## 4. Repetitions and statistics

For a usable series: `M=31` by default (the pedagogical default `M=16` remains
compilable), remove the first sample as warm-up according to a rule announced
before the trial. Do not stop a series at the first favourable value. Keep
every raw measurement, `N`, `M`, `BL`, `TYPE`, GCC options, binary identity,
and affinity condition.

The `results/` directory is a placeholder for generated outputs. The suite
does not impose a single summariser; the following rules are the recommended
protocol:

1. Reject as invalid the rows with different AUX tags, a zero `valid` flag,
   non-finite or negative ticks, or a non-positive denominator. A rejected
   row stays in the raw data.
2. Remove the first sample, then compute Q1 and Q3 with the inclusive linear
   quantiles of Python (`statistics.quantiles(..., method='inclusive')`).
3. Mark as high outliers the values above `Q3 + 1.5 * IQR`. If `IQR = 0`, do
   not apply an automatic statistical rejection; make the situation visible.
   No exclusion of low values.
4. Report the retained median, the minimum, the maximum, the IQR, the median
   and the range before filtering, the sample counts, and the classification
   of each sample. Fewer than five valid or retained measurements fail the
   processing.

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
configuration. A proposed method benchmark: aim for a raw median at least 100
times the empty median (indicative contribution of about 1%, not a proven
error bound). Below that, mark the measurement as dominated or significantly
affected by the instrumentation; increase the size or prepare a grouped
measurement with inner repetitions and an explicitly adapted denominator. Do
not use a negative corrected result and do not declare a speed-up on that
basis.

At N=64, the zeroing and the qualification dot product do not reach this
benchmark; their role is to reveal the limit. The blocked multiplication
exceeds it. None of these three trials is a scientific campaign or a variant
comparison.

## 6. Hybrid machine and reproduction

The `cpu_probe` program prints the current logical processor, the TSC AUX
tag, the brand, the CPUID.1A core type, the native model ID, the CPUID.15
ratio, the CPUID.16 MHz, the CPUID.6 EAX, the CPUID.7 hybrid bit, the
invariant TSC bit, and the cache descriptors as one JSON line. Run it under
`taskset` to pin the affinity.

The SMT sibling, the system load, the temperature, the power, and the turbo
are not controlled here. The affinity is the main protection. Migrations are
not treated as simple high values.

Each sample keeps `IA32_TSC_AUX` at the start and at the end. Equality does
not prove the absence of a round-trip migration or a preemption; AUX is a
system-defined tag, not a portable identifier. The affinity is the main
protection.

## 7. Reproduction

From the repository root:

```sh
make
make test
make run
make check
```

The `make check` target runs the functional tests, the TSC probe, the
overhead probe, the CPU probe, the latency probes, and the matrix extras
smoke tests. The `make run` target runs the default operation set. The
`make bench` and `make bench-full` targets run the short and full benchmark
configurations.

## 8. Technical sources

- Intel SDM, volume 2, RDTSC and RDTSCP entries.
- GCC, Extended Asm, Volatile and Clobbers sections.
- The official subject, §1–3, and the A0-C1 and A0-C6 course material.

The comparisons of GCC performance and the cache studies remain outside the
scope of this suite.
