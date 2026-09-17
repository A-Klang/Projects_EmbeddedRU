#!/usr/bin/env python3
"""
Plot actual speed and PWM value vs time from a CSV produced by capture_serial.py.

Usage:
    python3 plot_response.py step_response.csv --title "Step response (Kp=1, ref 0->50 RPM)"
    python3 plot_response.py load_response.csv --title "Load disturbance response (Kp=1, ref=50 RPM)" --out load_response.png
"""
import argparse
import csv

import matplotlib.pyplot as plt


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("csv_path", help="Input CSV file (time_s,actual_rpm,pwm_value)")
    parser.add_argument("--title", default=None, help="Plot title")
    parser.add_argument("--out", default=None, help="Save the plot to this PNG path instead of just showing it")
    parser.add_argument("--ref", type=float, default=None, help="Draw a horizontal reference-speed line at this RPM")
    args = parser.parse_args()

    time_s, actual_rpm, pwm_value = [], [], []
    with open(args.csv_path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            time_s.append(float(row["time_s"]))
            actual_rpm.append(float(row["actual_rpm"]))
            pwm_value.append(float(row["pwm_value"]))

    fig, ax1 = plt.subplots(figsize=(10, 5))

    ax1.set_xlabel("Time (s)")
    ax1.set_ylabel("Actual speed (RPM)", color="tab:blue")
    ax1.plot(time_s, actual_rpm, color="tab:blue", label="Actual speed")
    ax1.tick_params(axis="y", labelcolor="tab:blue")
    if args.ref is not None:
        ax1.axhline(args.ref, color="tab:blue", linestyle="--", linewidth=1, label=f"Reference ({args.ref} RPM)")

    ax2 = ax1.twinx()
    ax2.set_ylabel("PWM value", color="tab:red")
    ax2.plot(time_s, pwm_value, color="tab:red", alpha=0.6, label="PWM value")
    ax2.tick_params(axis="y", labelcolor="tab:red")

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc="lower right")

    if args.title:
        plt.title(args.title)
    fig.tight_layout()

    if args.out:
        fig.savefig(args.out, dpi=150)
        print(f"Saved plot to {args.out}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
