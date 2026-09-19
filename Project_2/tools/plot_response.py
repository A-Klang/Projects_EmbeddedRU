#!/usr/bin/env python3
"""
Plot reference speed, actual speed, and PWM value vs time from a CSV
produced by capture_serial.py (columns: time_ms,ref_speed,actual_speed,pwm_value).

Edit the settings below, then just run:
    python3 plot_response.py
"""
import csv

import matplotlib.pyplot as plt

# --- Edit these, then run the script ---
CSV_PATH = "test_new_build.csv"
TITLE = "Two steps"          # e.g. "Step response (Kp=3, ref 0->30 RPM)", or None for no title
OUT_PATH = None        # e.g. "step_response.png" to save instead of showing, or None to just show it
# ----------------------------------------


def main():
    time_s, ref_speed, actual_speed, pwm_value = [], [], [], []
    with open(CSV_PATH, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            time_s.append(float(row["time_ms"]) / 1000.0)
            ref_speed.append(float(row["ref_speed"]))
            actual_speed.append(float(row["actual_speed"]))
            pwm_value.append(float(row["pwm_value"]))

    fig, ax1 = plt.subplots(figsize=(10, 5))

    ax1.set_xlabel("Time (s)")
    ax1.set_ylabel("Speed (RPM)", color="tab:blue")
    ax1.plot(time_s, ref_speed, color="tab:blue", linestyle="--", label="Reference speed")
    ax1.plot(time_s, actual_speed, color="tab:blue", label="Actual speed")
    ax1.tick_params(axis="y", labelcolor="tab:blue")

    ax2 = ax1.twinx()
    ax2.set_ylabel("PWM value", color="tab:red")
    ax2.plot(time_s, pwm_value, color="tab:red", alpha=0.6, label="PWM value")
    ax2.tick_params(axis="y", labelcolor="tab:red")

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc="lower right")

    if TITLE:
        plt.title(TITLE)
    fig.tight_layout()

    if OUT_PATH:
        fig.savefig(OUT_PATH, dpi=150)
        print(f"Saved plot to {OUT_PATH}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
