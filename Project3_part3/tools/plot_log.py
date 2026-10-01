#!/usr/bin/env python3
"""
Plot reference speed, actual speed and PWM vs time from a serial log saved with
the PlatformIO monitor's log2file filter (columns: time_ms,ref_speed,actual_speed,pwm_value).

Handles the state machine's resets: "Boot-up" lines are skipped for the data but
used as reset markers, and since time_ms restarts from 0 after each reset, an
offset is added so the time axis runs continuously.

Usage: set LOG_FILE below and run the script (from any folder).
Saves the plot next to the log with the same name and a .png extension.
"""
from pathlib import Path

import matplotlib.pyplot as plt

LOG_FILE = "logs/part2_faultDet_wire_backtoOperational.csv"   # relative to the Project3 folder
TITLE ="Part 2: fault detection using manual low"   # or None for no title
PWM_LIMIT = 255   # Analog_out::set() clamps the magnitude to this, so the plot shows what is actually applied


def read_log(path):
    time_s, ref, speed, pwm, resets = [], [], [], [], []
    offset_ms = 0.0
    last_ms = 0.0
    for line in Path(path).read_text().splitlines():
        line = line.strip()
        if line == "Boot-up":
            if time_s:                      # a reset after data has started: continue the time axis
                offset_ms += last_ms
                resets.append(offset_ms / 1000.0)
            continue
        parts = line.split(",")
        try:
            t_ms, r, s, u = (float(p) for p in parts)
        except ValueError:                  # header, monitor banner or a partial line
            continue
        last_ms = t_ms
        time_s.append((t_ms + offset_ms) / 1000.0)
        ref.append(r)
        speed.append(s)
        pwm.append(max(-PWM_LIMIT, min(PWM_LIMIT, u)))
    return time_s, ref, speed, pwm, resets


def main():
    csv_path = Path(__file__).resolve().parent.parent / LOG_FILE
    time_s, ref, speed, pwm, resets = read_log(csv_path)

    # Speed on top, PWM below on a shared time axis: the PWM toggles on every
    # encoder-count quantization step and would hide the speed on a shared y axis.
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6), sharex=True,
                                   gridspec_kw={"height_ratios": [2, 1]})

    ax1.set_ylabel("Speed (RPM)")
    ax1.plot(time_s, ref, color="tab:blue", linestyle="--", label="Reference speed")
    ax1.plot(time_s, speed, color="tab:orange", linewidth=1, label="Actual speed")
    ax1.grid(alpha=0.3)

    ax2.set_xlabel("Time (s)")
    ax2.set_ylabel("Applied PWM")
    ax2.plot(time_s, pwm, color="tab:red", linewidth=0.6)
    ax2.set_ylim(-PWM_LIMIT * 1.1, PWM_LIMIT * 1.1)
    ax2.axhline(0, color="black", linewidth=0.5)
    ax2.grid(alpha=0.3)

    for i, t in enumerate(resets):
        for ax in (ax1, ax2):
            ax.axvline(t, color="black", linestyle=":", label="Reset (r)" if i == 0 and ax is ax1 else None)

    ax1.legend(loc="lower right")
    if TITLE:
        ax1.set_title(TITLE)
    fig.tight_layout()

    out_path = csv_path.with_suffix(".png")
    fig.savefig(out_path, dpi=150)
    print(f"Saved plot to {out_path}")


if __name__ == "__main__":
    main()
