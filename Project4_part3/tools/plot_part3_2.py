#!/usr/bin/env python3
"""
Part 3.2 step and load response, logged on the Raspberry Pi from TPDO1 by canopen_client,
compared to Project 3 with the same PI parameters (Kp = 34, Ti = 0.03 s).

The client CSV has columns time_s,target_rpm,speed_rpm,duty_pct with t = 0 just before NMT start,
so the time axis matches Project 3's "time since Operational entry". The duty cycle is converted
to PWM counts (x 2.55) to share an axis with the Project 3 logs.
"""
from pathlib import Path
import csv
import statistics as st

import matplotlib.pyplot as plt

from plot_part4 import load, crop, speed_pwm_plot, PWM_LIMIT
import plot_part4

ROOT = Path(__file__).resolve().parent.parent
LOGS = ROOT / "logs"
P3_LOGS = ROOT.parent / "Project3_part3" / "logs"
P4_LOAD = (6.1, 12.0)   # load applied by hand, read off the duty cycle
P3_LOAD = (18.0, 23.9)  # moderate load in Project 3, same intervals as plot_part4.py


def read_client(name):
    """Read a canopen_client CSV and return (t, target, speed, pwm)."""
    t, r, s, u = [], [], [], []
    with open(LOGS / name) as f:
        for row in csv.DictReader(f):
            t.append(float(row["time_s"]))
            r.append(int(row["target_rpm"]))
            s.append(int(row["speed_rpm"]))
            u.append(float(row["duty_pct"]) * 2.55)
    return t, r, s, u


def load_p3(name):
    plot_part4.LOGS = P3_LOGS
    data = load(name)
    plot_part4.LOGS = LOGS
    return data


def step_stats(name, data, ref=40):
    """Rise time to 90 % of the reference and steady state from 2 to 8 s."""
    t, r, s, u = data
    step = next(ti for ti, ri in zip(t, r) if ri == ref)
    t90 = next(ti for ti, si in zip(t, s) if ti >= step and si >= 0.9 * ref) - step
    ss = [si for ti, si in zip(t, s) if 2 <= ti < 8]
    print(f"{name} 0->{ref} RPM: t90 {t90 * 1000:.0f} ms, max {max(s):.2f}, "
          f"steady state {st.mean(ss):.2f} (std {st.pstdev(ss):.2f})")


def compare_plot(p3, p4, title, out):
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6), sharex=True, gridspec_kw={"height_ratios": [2, 1]})
    ax1.plot(p4[0], p4[1], "--", color="black", linewidth=1, label="Reference speed")
    ax1.plot(p3[0], p3[2], color="tab:orange", linewidth=1, label="Project 3, Arduino Nano (serial log)")
    ax1.plot(p4[0], p4[2], color="tab:green", linewidth=1, label="Project 4, Nucleo + Zephyr (TPDO1 on the Pi)")
    ax1.set_ylabel("Speed (RPM)")
    ax1.grid(alpha=0.3)
    ax1.legend(loc="lower right")
    ax1.set_title(title)
    ax2.plot(p3[0], p3[3], color="tab:orange", linewidth=0.6)
    ax2.plot(p4[0], p4[3], color="tab:green", linewidth=0.6)
    ax2.set_ylim(-280, 280)
    ax2.axhline(0, color="black", linewidth=0.5)
    ax2.set_ylabel("Controller output (PWM)")
    ax2.set_xlabel("Time since Operational entry (s)")
    ax2.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(LOGS / out, dpi=150)
    plt.close(fig)


def load_compare_plot(p3, p4, out):
    """Project 3 and Project 4 side by side, each from 2 s before to 8 s after the load onset."""
    fig, axs = plt.subplots(2, 2, figsize=(12, 6), sharex=True, sharey="row", gridspec_kw={"height_ratios": [2, 1]})
    for col, data, (a, b), label, color in ((0, p3, P3_LOAD, "Project 3, Arduino Nano", "tab:orange"),
                                            (1, p4, P4_LOAD, "Project 4, Nucleo + Zephyr (TPDO1 on the Pi)", "tab:green")):
        t, r, s, u = crop(data, a - 2, a + 8)
        for row in (0, 1):
            axs[row][col].axvspan(2, 2 + b - a, color="tab:purple", alpha=0.08,
                                  label="Load applied" if row == 0 else None)
            axs[row][col].grid(alpha=0.3)
        axs[0][col].plot(t, r, "--", color="black", linewidth=1, label="Reference")
        axs[0][col].plot(t, s, color=color, linewidth=1, label="Speed")
        axs[0][col].set_ylim(28, 45)
        axs[0][col].set_title(label)
        axs[0][col].legend(loc="lower right", ncol=3)
        axs[1][col].plot(t, u, color=color, linewidth=0.6)
        axs[1][col].set_ylim(0, 280)
        axs[1][col].set_xlabel("Time from 2 s before load onset (s)")
    axs[0][0].set_ylabel("Speed (RPM)")
    axs[1][0].set_ylabel("Controller output (PWM)")
    fig.suptitle("Load response, PI Kp = 34, Ti = 0.03 s, moderate load by hand")
    fig.tight_layout()
    fig.savefig(LOGS / out, dpi=150)
    plt.close(fig)


def main():
    # Step response
    p4_step = crop(read_client("step_40.csv"), 0, 7.99)
    p3_step = load_p3("part4_tune1_kp34_Ti003.csv")
    p3_step = crop(p3_step, 0, min(7.99, p3_step[0][-1]))  # Project 3 steps on to 60 RPM at 8 s
    step_stats("P4 over CAN", p4_step)
    step_stats("P3 serial  ", p3_step)
    speed_pwm_plot(p4_step, "Part 3.2: step response logged from TPDO1, PI Kp = 34, Ti = 0.03 s", "part3_2_step.png")
    compare_plot(p3_step, p4_step, "Step response, PI Kp = 34, Ti = 0.03 s: Project 3 vs Project 4",
                 "part3_2_step_P3_vs_P4.png")

    # True step (acc = dec = 0) from the 0 -> 40 -> 0 run, the same kind of step as in Project 3
    p4_true = crop(read_client("trap_acc0.csv"), 0, 3.99)
    p3_true = crop(p3_step, 0, 3.99)
    step_stats("P4 true step", p4_true)
    compare_plot(p3_true, p4_true, "True step (no acceleration limit), PI Kp = 34, Ti = 0.03 s: Project 3 vs Project 4",
                 "part3_2_truestep_P3_vs_P4.png")

    # Load response
    p4_load = read_client("load_40.csv")
    p3_load = load_p3("part4_PI_loadApplied.csv")
    speed_pwm_plot(p4_load, "Part 3.2: load response logged from TPDO1, PI Kp = 34, Ti = 0.03 s", "part3_2_load.png",
                   spans=((*P4_LOAD, "tab:purple", "Load applied"),))
    load_compare_plot(p3_load, p4_load, "part3_2_load_P3_vs_P4.png")
    print("Plots saved in", LOGS)


if __name__ == "__main__":
    main()
