#!/usr/bin/env python3
"""Render the six showcase figures and summarize the archived measurements."""
import csv
from collections import Counter
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets"
RESULTS = ROOT / "results"
BLUE = "#245B82"
TEAL = "#007A70"
AMBER = "#A04E13"
GRAY = "#526276"
PAIRED = "median_of_paired_ratios_"

plt.rcParams.update({
    "font.family": "DejaVu Sans",
    "axes.spines.top": False,
    "axes.spines.right": False,
    "axes.titleweight": "bold",
    "axes.labelcolor": "#293B4C",
    "xtick.color": "#293B4C",
    "ytick.color": "#293B4C",
    "grid.color": "#D7DFE7",
    "legend.frameon": False,
})


def read_table(name):
    with (ROOT / "data" / f"{name}.csv").open(encoding="utf-8", newline="") as stream:
        return list(csv.DictReader(stream))


def numbers(rows, column):
    return [float(row[column]) for row in rows]


def chart(title, xlabel, ylabel, height=4.2):
    fig, ax = plt.subplots(figsize=(8.6, height), layout="constrained")
    ax.set(title=title, xlabel=xlabel, ylabel=ylabel)
    ax.set_axisbelow(True)
    ax.grid(axis="y")
    return fig, ax


def save(fig, name):
    fig.savefig(ASSETS / f"{name}.png", dpi=170)
    plt.close(fig)


def paired_intervals(ax, x, rows, ratio_column, color, label=None, hollow=False):
    ratios = numbers(rows, PAIRED + ratio_column)
    low = numbers(rows, "paired_ratio_ci95_low")
    high = numbers(rows, "paired_ratio_ci95_high")
    ax.errorbar(
        x, ratios,
        yerr=([r - lo for r, lo in zip(ratios, low)], [hi - r for r, hi in zip(ratios, high)]),
        fmt="o", color=color, markerfacecolor="white" if hollow else color,
        capsize=3, label=label,
    )


def plot_gcc(rows):
    levels = ("-O0", "-O1", "-O3")
    configs = list(dict.fromkeys((r["kernel"], r["element_type"], r["N"]) for r in rows))
    ratios = {(r["gcc_level"], r["kernel"], r["element_type"], r["N"]):
              float(r["ratio_of_median_durations_vs_O2"]) for r in rows}
    values = [[ratios[(level, *config)] for config in configs] for level in levels]
    fig, ax = plt.subplots(figsize=(10.4, 4.2), layout="constrained")
    heatmap = ax.imshow(values, aspect="auto", cmap="Blues", norm=LogNorm(vmin=0.5, vmax=20))
    ax.set_title("How GCC optimization changes kernel duration")
    ax.set_xticks(range(len(configs)), [f"{k}\n{t}, {n}" for k, t, n in configs],
                  rotation=55, ha="right", fontsize=8)
    ax.set_yticks(range(len(levels)), levels)
    for y, row in enumerate(values):
        for x, ratio in enumerate(row):
            ax.text(x, y, f"{ratio:.2g}", ha="center", va="center", fontsize=8,
                    color="white" if ratio >= 4 else "#132535")
    fig.colorbar(heatmap, ax=ax, label="Ratio of median durations vs. -O2")
    save(fig, "gcc_optimization_levels")


def plot_copy(rows):
    fig, ax = chart("Column-wise copy takes longer than row-wise copy", "Matrix dimension N",
                    "Column-wise / row-wise duration")
    for offset, kind, color in ((-13, "float", BLUE), (13, "double", TEAL)):
        group = [r for r in rows if r["element_type"] == kind]
        qualified = [r for r in group if r["duration_at_least_100x_timer_cost"] == "1"]
        short = [r for r in group if r["duration_at_least_100x_timer_cost"] == "0"]
        paired_intervals(ax, [int(r["N"]) + offset for r in qualified], qualified,
                         "ji_over_ij", color, kind)
        if short:
            paired_intervals(ax, [int(r["N"]) + offset for r in short], short,
                             "ji_over_ij", color, f"{kind}, below timer threshold", hollow=True)
    ax.set_xticks(sorted({int(r["N"]) for r in rows}))
    ax.axhline(1, color=GRAY, linestyle="--", linewidth=1)
    ax.set_ylim(bottom=0)
    ax.legend(loc="upper left")
    save(fig, "copy_ij_vs_ji")


def plot_matrix(rows):
    fig, ax = chart("Matrix multiplication is sensitive to loop order and size",
                    "Tested matrix dimensions (N)", "Median TSC ticks per inner iteration")
    sizes = [row["N"] for row in rows]
    ax.plot(sizes, numbers(rows, "ijk_S1_ticks_per_iteration"), "o-", color=BLUE, label="ijk")
    ax.plot(sizes, numbers(rows, "ikj_S1_ticks_per_iteration"), "o-", color=TEAL, label="ikj")
    ax.set_yscale("log")
    ticks = [0.25, 0.5, 1, 2, 4, 8]
    ax.set_yticks(ticks, [str(value) for value in ticks])
    ax.legend()
    save(fig, "matrix_multiply_ijk_vs_ikj")


def plot_blocks(rows):
    fig, ax = chart("Blocking helps at selected tile sizes", "Block size (elements per side)",
                    "Unblocked / blocked duration")
    positions = range(len(rows))
    paired_intervals(ax, positions, rows, "ijk_over_blocked", BLUE)
    significant = [(i, float(row[PAIRED + "ijk_over_blocked"])) for i, row in enumerate(rows)
                   if row["gain_over_5pct_significant"] == "true"]
    ax.scatter([x for x, _ in significant], [y for _, y in significant],
               color=TEAL, marker="D", zorder=4, label="Gain meets the 5% criterion")
    ax.axhline(1, color=GRAY, linestyle="--", linewidth=1, label="Equal duration")
    ax.set_xticks(positions, [row["BL"] for row in rows])
    ax.legend(loc="lower right")
    save(fig, "block_size_n1000")


def plot_dot(rows):
    fig, ax = chart("Four accumulators reduce dot-product duration", "Square root N of vector length",
                    "One / four accumulator duration")
    sizes = numbers(rows, "N")
    paired_intervals(ax, sizes, rows, "ps_over_ps4", TEAL)
    ax.axhline(1, color=GRAY, linestyle="--", linewidth=1)
    ax.set_xticks(sizes)
    ax.set_ylim(bottom=0)
    save(fig, "dot_product_ps_vs_ps4")


def plot_latency(rows):
    fig, ax = chart("Independent instruction chains improve throughput",
                    "Operation and operand type", "Median TSC ticks per instruction")
    operations = [(kind, op) for kind in ("int", "float") for op in ("add", "mul", "div")]
    series = {(r["element_type"], r["operation"], r["chains"]):
              float(r["median_ticks_per_operation"]) for r in rows
              if r["loop_trips"] == "20000" and r["role"] == "instruction"}
    for chain, offset, color, label in (
        ("1", -0.18, BLUE, "One dependent chain"),
        ("4", 0.18, TEAL, "Four independent chains"),
    ):
        ax.bar([i + offset for i in range(len(operations))],
               [series[(*op, chain)] for op in operations], width=0.36, color=color, label=label)
    ax.set_xticks(range(len(operations)), [f"{kind} {op}" for kind, op in operations])
    ax.legend()
    save(fig, "instruction_latency")


def summarize(gcc, copy, matrix, blocks, dot, latency):
    o0 = numbers([r for r in gcc if r["gcc_level"] == "-O0"], "ratio_of_median_durations_vs_O2")
    o1 = Counter(r["decision_vs_O2"] for r in gcc if r["gcc_level"] == "-O1")
    copy_gains = numbers([r for r in copy if r["duration_at_least_100x_timer_cost"] == "1"],
                         PAIRED + "ji_over_ij")
    mm = next(r for r in matrix if r["N"] == "1000")
    block = min((r for r in blocks if r["gain_over_5pct_significant"] == "true"),
                key=lambda r: int(r["BL"]))
    gains = numbers(dot, PAIRED + "ps_over_ps4")
    int_add = [r for r in latency if r["loop_trips"] == "20000"
               and r["operation"] == "add" and r["element_type"] == "int"]
    add = {r["chains"]: float(r["median_ticks_per_operation"]) for r in int_add}
    summary = [
        f"- GCC `-O0` took **{min(o0):.1f}–{max(o0):.1f}×** the `-O2` duration. "
        f"`-O1` was slower in {o1['slower']} of {sum(o1.values())} comparisons.",
        f"- Column-wise copy took **{min(copy_gains):.2f}–{max(copy_gains):.2f}×** the row-wise duration "
        "among pairs above the timer threshold.",
        f"- At N=1000, `ijk` took **{float(mm[PAIRED + 'ijk_over_ikj']):.3f}×** the `ikj` duration "
        f"(95% interval {float(mm['paired_ratio_ci95_low']):.3f}–{float(mm['paired_ratio_ci95_high']):.3f}). "
        f"The ratio of median durations was {float(mm['ratio_of_median_durations_ijk_over_ikj']):.3f}.",
        f"- At N=1000, **{block['BL']}** was the smallest tested block size that met the 5% gain criterion "
        f"({float(block[PAIRED + 'ijk_over_blocked']):.3f}× for unblocked/blocked duration).",
        "- The one/four-accumulator dot-product ratios were **" +
        ", ".join(f"{gain:.2f}× at N={row['N']}" for gain, row in zip(gains, dot)) + "**.",
        f"- Median integer-addition cost was **{add['1']:.3f} TSC ticks per instruction** on one dependent "
        f"chain and **{add['4']:.3f}** across four independent chains.",
    ]
    return "\n".join(summary) + "\n"


def main():
    ASSETS.mkdir(exist_ok=True)
    RESULTS.mkdir(exist_ok=True)
    datasets = [
        ("gcc_optimization_levels", plot_gcc),
        ("copy_ij_vs_ji", plot_copy),
        ("matrix_multiply_ijk_vs_ikj", plot_matrix),
        ("block_size_n1000", plot_blocks),
        ("dot_product_ps_vs_ps4", plot_dot),
        ("instruction_latency", plot_latency),
    ]
    tables = []
    for name, plot in datasets:
        rows = read_table(name)
        plot(rows)
        tables.append(rows)
    (RESULTS / "summary.md").write_text(summarize(*tables), encoding="utf-8")
    print(f"Rendered {len(tables)} figures in assets/ and wrote results/summary.md")


if __name__ == "__main__":
    main()
