# Reference results

This directory contains six archived measurement tables used as inputs to
`make figures`. The plotting script writes the charts to `assets/` and a
numerical summary to `results/summary.md`.

Most configurations used 30 randomized, paired rounds on a single pinned
performance core. Instruction chains used 10 launches per configuration.
The original campaign drivers and raw traces are separate from this repository.
`make run` collects new samples on the current machine. Expect absolute
durations and effect sizes to vary with platform and operating conditions.

## Platform and conditions

| Item | Value |
| --- | --- |
| Processor | Intel Core i5-1335U (hybrid: performance and efficiency cores) |
| Core used | one performance core (CPUID leaf 0x1A core type `0x40`), `taskset` |
| Caches (CPUID leaf 4) | L1D 48 KiB, L2 1.25 MiB per performance core, L3 12 MiB |
| TSC | invariant, 2.496 GHz (CPUID leaf 0x15: 130/2 x 38.4 MHz) |
| Compiler | GCC 13.3.0 (Ubuntu 24.04), `-std=gnu11 -Wall -Wextra -Werror`, `-O0` to `-O3` as stated, no `-march`, no `-ffast-math`, no LTO |
| Operating system | Linux |
| Not controlled | core frequency state (sysfs readings from 0.4 to 4.6 GHz), package temperature (70 to 100 C observed, median 87 to 89 C depending on the campaign), background activity, SMT sibling activity, interrupts |

Measurement protocol by data set

| Dataset | Samples per launch | Per-launch filtering | Launches |
| --- | --- | --- | --- |
| `gcc_optimization_levels.csv` | 51, first discarded | high outliers above Q3 + 1.5 IQR removed | 30 |
| `copy_ij_vs_ji.csv` | 51, first discarded | high outliers above Q3 + 3 IQR removed (if IQR > 0) | 30 |
| `matrix_multiply_ijk_vs_ikj.csv`, `block_size_n1000.csv`, `dot_product_ps_vs_ps4.csv` | 3, first discarded, two kept | none (two samples) | 30 |
| `instruction_latency.csv` | 31 per launch, first discarded; per-launch median | none | 10 |

The float copy pair at N=256 falls below the 100× empty-timer threshold.
Treat that comparison as descriptive. The smallest ratio among the matrix,
block, and dot-product configurations is 134 (`ikj`, N=32). The instruction
table provides a separate qualification flag for each row.

## Statistics

- **Tick**: one TSC increment, not a core clock cycle. Seconds are
  `ticks / 2.496e9`. Turbo, frequency changes, and hybrid cores prevent a
  direct conversion to core cycles. See [`../docs/METHODOLOGY.md`](../docs/METHODOLOGY.md).
- **Ticks per iteration**: ticks of one call divided by `N^2` (zero, copy,
  addition, dot product) or `N^3` (matrix multiplications). The matrix
  multiplication `ikj` and blocked versions include zeroing the result.
- **S1**: median over launches of the per-launch median. **S2**: median over
  launches of the per-launch minimum (best case, sensitivity only).
- **Two different ratios are published for comparisons; they are not the same
  quantity**:
  - `ratio_of_median_durations_*` is the ratio `S1(a) / S1(b)` of two
    statistics.
  - `median_of_paired_ratios_*` is the median over the 30 rounds of the ratio
    `t_a / t_b` of the two measurements taken in the same round. It is the
    primary estimator of a paired gain. Its 95 % interval
    (`paired_ratio_ci95_low`, `paired_ratio_ci95_high`) is a percentile
    bootstrap (10 000 resamples of the 30 paired ratios, fixed seed).
  - In `gcc_optimization_levels.csv`, only the ratio of `S1` values is
    available; its interval is a bootstrap over rounds resampled in pairs.
- **Decision rule against `-O2`** (`decision_vs_O2`): `faster` or `slower`
  when the 95 % interval excludes 1 and the ratio differs from 1 by more than
  5 %; `equivalent` when the interval lies within [0.95, 1.05];
  `inconclusive` otherwise. Thirteen comparisons whose decision was fragile
  (`flagged_fragile = 1` in the main run) were repeated with 60 launches; where
  the two runs disagreed the decision is `inconclusive`. The ratio and interval
  columns keep the 30-launch values. No correction for multiple comparisons
  was applied to this table; read it as exploratory.
- **Block-size gain** (`gain_over_5pct_significant`): the median of paired
  ratios exceeds 1.05 and an exact one-sided sign test against 1.05, with
  Holm correction over the 12 block sizes at 5 %, rejects (`p_holm` is the
  adjusted p-value). A result that is not significant is not evidence of
  equivalence.

The intervals are descriptive. Rounds are not shown to be independent; the
conditions above (thermal state, frequency, background load) were not
controlled, so the results are local to this machine and state. No
hardware performance counters were available, so cache or memory-system
explanations are hypotheses compatible with the data, not demonstrated causes.

## Files

| File | Rows | Content |
| --- | --- | --- |
| [`gcc_optimization_levels.csv`](gcc_optimization_levels.csv) | 64 | 7 kernels, GCC `-O0` to `-O3`, float and int, two sizes each |
| [`copy_ij_vs_ji.csv`](copy_ij_vs_ji.csv) | 8 | `copy_ij` vs `copy_ji`, float and double, N = 256, 512, 1024, 1536, GCC `-O2` |
| [`matrix_multiply_ijk_vs_ikj.csv`](matrix_multiply_ijk_vs_ikj.csv) | 8 | `mm_ijk` vs `mm_ikj`, float, N = 32 to 1024, GCC `-O2` |
| [`block_size_n1000.csv`](block_size_n1000.csv) | 12 | `mm_b_ijk` for 12 block sizes vs `mm_ijk`, N = 1000, float, GCC `-O2` |
| [`dot_product_ps_vs_ps4.csv`](dot_product_ps_vs_ps4.csv) | 3 | `ps` (one accumulator) vs `ps4` (four accumulators), float, N = 256, 512, 1000, GCC `-O2` |
| [`instruction_latency.csv`](instruction_latency.csv) | 36 | x86 assembly chains: add, multiply, divide for integer and float, one dependent chain or four independent chains, plus empty-loop controls, 2000 and 20000 loop trips |

### Column dictionary

Common columns: `element_type` (`float`, `int`, `double`), `N` (matrix
dimension; vectors have `N^2` elements), `gcc_level`, `launches` (number of
launches), `BL` (block size).

| Column | Unit and meaning |
| --- | --- |
| `S1_ticks_per_iteration`, `S2_ticks_per_iteration` | TSC ticks per iteration (see above); S1 median of launch medians, S2 median of launch minima |
| `S1_ci95_low`, `S1_ci95_high` | 95 % interval of S1, same unit |
| `ij_S1_ticks_per_element`, `ji_S1_ticks_per_element` | ticks per matrix element, S1 |
| `ijk_S1_ticks`, `ikj_S1_ticks`, `blocked_S1_ticks`, `ps_S1_ticks`, `ps4_S1_ticks` | ticks of one complete call, S1 |
| `*_S1_ticks_per_iteration`, `*_S1_ticks_per_term` | the same, divided by `N^3` or by the number of dot-product terms `N^2` |
| `ratio_of_median_durations_*` | ratio of two S1 values (dimensionless) |
| `median_of_paired_ratios_*` | median of the per-round ratios (dimensionless, primary estimator) |
| `paired_ratio_ci95_low`, `paired_ratio_ci95_high` | 95 % bootstrap interval of the median of paired ratios |
| `ratio_ci95_low`, `ratio_ci95_high` | interval of the ratio of S1 values (GCC file only) |
| `ijk_GFLOPs`, `ikj_GFLOPs` | `2 N^3` operations divided by the TSC-based duration, in 10^9 per second (convention: one multiplication and one addition per inner iteration) |
| `decision_vs_O2` | `faster`, `slower`, `equivalent`, `inconclusive`, or `reference` (the `-O2` row) |
| `flagged_fragile` | 1 if the decision differed between statistics in the main run |
| `duration_at_least_100x_timer_cost` | 1 if the call lasts at least 100 times an empty timer pair |
| `p_holm`, `gain_over_5pct_significant` | see the block-size rule above |
| `operation`, `role` | `add`, `mul`, `div` are `instruction` chains; `empty` (loop counter only) and `divempty` (the operand moves of `div` without the division) are `control` chains |
| `chains` | 1: one dependent chain; 4: four independent chains |
| `loop_trips` | iterations of the assembly loop (2000 or 20000) |
| `median_ticks_per_operation`, `ns_per_operation` | ticks per operation of the chain: median over the 10 launches of the per-launch median of 30 samples; `ns_per_operation` is that value divided by 2.496 (TSC frequency in GHz), not a core-clock time |
| `p10_ticks_per_operation`, `p90_ticks_per_operation` | 10th and 90th percentile of the 10 per-launch medians |
| `min_duration_over_timer_cost`, `all_launches_long_enough` | smallest ratio over launches of the median call duration to the median cost of an empty timer pair; `true` if at least 100. Empty-loop controls at 2000 trips are shorter than that and are descriptive only |
