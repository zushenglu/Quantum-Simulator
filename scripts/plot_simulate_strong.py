"""Plot the serial and parallel CSVs from a strong-scaling run."""

import argparse
import csv
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read_results(path):
    with path.open(newline="") as csv_file:
        return sorted(csv.DictReader(csv_file), key=lambda row: int(row["thread"]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_dir", type=Path, help="Directory containing the two benchmark CSVs")
    parser.add_argument("--subtitle", help="Optional description of the circuit and trials")
    args = parser.parse_args()

    serial = read_results(args.run_dir / "simulate.csv")[0]
    parallel = read_results(args.run_dir / "simulate_parallel.csv")
    threads = [int(row["thread"]) for row in parallel]
    panels = [
        ("time", "Time (s)", [float(serial["time"])] * len(threads), "Serial baseline"),
        ("speedup", "Speedup (×)", threads, "Ideal scaling"),
        ("speedup efficiency", "Efficiency (%)", [100] * len(threads), "Ideal scaling"),
    ]

    fig, axes = plt.subplots(1, 3, figsize=(12, 4), constrained_layout=True)
    title = "Dense simulator strong scaling"
    if args.subtitle:
        title += f"\n{args.subtitle}"
    fig.suptitle(title)

    for ax, (column, label, reference, reference_label) in zip(axes, panels):
        ax.plot(threads, reference, "--", color="0.45", label=reference_label)
        ax.plot(threads, [float(row[column]) for row in parallel], "o-",
                color="#166b99", label="Parallel")
        ax.scatter(int(serial["thread"]), float(serial[column]), marker="s",
                   color="#b34b20", label="Serial", zorder=4)
        ax.set(xlabel="Threads", ylabel=label, xticks=threads)
        ax.grid(axis="y", alpha=0.25)
        ax.legend(fontsize=8)

    fig.savefig(args.run_dir / "strong_scaling.png", dpi=180)


if __name__ == "__main__":
    main()
