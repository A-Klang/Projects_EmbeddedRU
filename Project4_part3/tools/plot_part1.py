#!/usr/bin/env python3
"""
Part 1 step response on Zephyr vs Project 3, same PI parameters (Kp = 34, Ti = 0.03 s).
load() picks the last 0 -> 40 RPM step in the log, so the run after the parameters were set is used.
"""
from pathlib import Path
import sys

import matplotlib.pyplot as plt

from plot_part4 import load, crop, speed_pwm_plot, step_stats
import plot_part4

ROOT = Path(__file__).resolve().parent.parent
P3_LOGS = ROOT.parent / "Project3_part3" / "logs"


def main():
    p4 = crop(load("part1_step_PI_kp34_Ti003.log"), 0, 11.99)
    plot_part4.LOGS = P3_LOGS
    p3 = load("part4_tune1_kp34_Ti003.csv")
    plot_part4.LOGS = ROOT / "logs"
    p3 = crop(p3, 0, min(11.99, p3[0][-1]))

    step_stats("P4 Zephyr", p4)
    step_stats("P3 AVR   ", p3)
    speed_pwm_plot(p4, "Part 1: PI on Zephyr, Kp = 34, Ti = 0.03 s", "part1_step_PI_kp34_Ti003.png")

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6), sharex=True, gridspec_kw={"height_ratios": [2, 1]})
    ax1.plot(p4[0], p4[1], "--", color="black", linewidth=1, label="Reference speed")
    ax1.plot(p3[0], p3[2], color="tab:orange", linewidth=1, label="Project 3, Arduino Nano")
    ax1.plot(p4[0], p4[2], color="tab:green", linewidth=1, label="Project 4, Nucleo + Zephyr")
    ax1.set_ylabel("Speed (RPM)")
    ax1.grid(alpha=0.3)
    ax1.legend(loc="lower right")
    ax1.set_title("Step response, PI Kp = 34, Ti = 0.03 s: Project 3 vs Project 4")
    ax2.plot(p3[0], p3[3], color="tab:orange", linewidth=0.6)
    ax2.plot(p4[0], p4[3], color="tab:green", linewidth=0.6)
    ax2.set_ylim(-280, 280)
    ax2.set_ylabel("Controller output (PWM)")
    ax2.set_xlabel("Time since Operational entry (s)")
    ax2.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(ROOT / "logs" / "part1_step_P3_vs_P4.png", dpi=150)


if __name__ == "__main__":
    main()
