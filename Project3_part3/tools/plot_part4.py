#!/usr/bin/env python3
"""
Plots for Part 4 (tuning, step and load response), built on read_log() from plot_log.py.

Each run is cropped to start at Operational entry. The test profile in main() is
timed from Operational entry, so the entry time is found from the reference:
the step from 0 to 40 RPM happens exactly 1 s after entry.

Usage: run the script (from any folder). All plots are saved in logs/.
"""
from pathlib import Path
import statistics as st

import matplotlib.pyplot as plt

from plot_log import read_log

ROOT = Path(__file__).resolve().parent.parent
LOGS = ROOT / "logs"
PWM_LIMIT = 255


def load(name):
    """Read a log and return (t, ref, speed, pwm) with t = 0 at Operational entry."""
    t, r, s, u, _ = read_log(LOGS / name)
    t0 = 0.0
    for i in range(1, len(r)):
        if r[i] == 40 and r[i - 1] == 0:
            t0 = t[i] - 1.0          # last 0 -> 40 step, 1 s after entry
    idx = [i for i, x in enumerate(t) if x >= t0]
    return ([t[i] - t0 for i in idx], [r[i] for i in idx],
            [s[i] for i in idx], [u[i] for i in idx])


def crop(data, a, b, shift=0.0):
    t, r, s, u = data
    idx = [i for i, x in enumerate(t) if a <= x <= b]
    return ([t[i] - a + shift for i in idx], [r[i] for i in idx],
            [s[i] for i in idx], [u[i] for i in idx])


def speed_pwm_plot(data, title, out, spans=(), xlabel="Time since Operational entry (s)"):
    t, r, s, u = data
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6), sharex=True,
                                   gridspec_kw={"height_ratios": [2, 1]})
    for a, b, color, label in spans:
        for ax in (ax1, ax2):
            ax.axvspan(a, b, color=color, alpha=0.08, label=label if ax is ax1 else None)
    ax1.plot(t, r, color="tab:blue", linestyle="--", label="Reference speed")
    ax1.plot(t, s, color="tab:orange", linewidth=1, label="Actual speed")
    ax1.set_ylabel("Speed (RPM)")
    ax1.grid(alpha=0.3)
    ax1.legend(loc="lower center" if spans else "lower right", ncol=2 if spans else 1)
    ax1.set_title(title)
    ax2.plot(t, u, color="tab:red", linewidth=0.6)
    ax2.set_ylim(-PWM_LIMIT * 1.1, PWM_LIMIT * 1.1)
    ax2.axhline(0, color="black", linewidth=0.5)
    ax2.set_ylabel("Applied PWM")
    ax2.set_xlabel(xlabel)
    ax2.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(LOGS / out, dpi=150)
    plt.close(fig)


def step_stats(name, data):
    t, r, s, u = data
    for a, b, ref, prev in ((1, 8, 40, 0), (8, 99, 60, 40)):
        seg = [(ti, si) for ti, si in zip(t, s) if a <= ti < b]
        ss = [si for ti, si in seg if ti >= a + 1]
        mean = st.mean(ss)
        target = prev + 0.9 * (mean - prev)
        t90 = next((ti - a for ti, si in seg if si >= target), float("nan"))
        print(f"{name} {prev}->{ref} RPM: t90 {t90 * 1000:.0f} ms, max {max(si for _, si in seg):.2f}, "
              f"steady state {mean:.2f} (error {ref - mean:.2f}, std {st.pstdev(ss):.2f})")


def limit_cycle_stats(data, a, b):
    """Relay (Astrom-Hagglund) estimate of Ku and Tu from the one-count limit cycle."""
    t, r, s, u = crop(data, a, b)
    mean = st.mean(s)
    ups = [t[i] for i in range(1, len(s)) if s[i - 1] < mean <= s[i]]
    periods = [y - x for x, y in zip(ups, ups[1:])]
    pwm = [max(-PWM_LIMIT, min(PWM_LIMIT, x)) for x in u]
    d = (max(pwm) - min(pwm)) / 2
    amp = (max(s) - min(s)) / 2
    Tu = st.median(periods)
    Ku = 4 * d / (3.14159 * amp)
    print(f"Limit cycle: d = {d:.1f}, a = {amp:.2f} RPM, Tu = {Tu * 1000:.0f} ms "
          f"(range {min(periods) * 1000:.0f}-{max(periods) * 1000:.0f} ms), Ku = {Ku:.0f}")
    print(f"ZN PI: Kp = 0.45 Ku = {0.45 * Ku:.1f}, Ti = Tu / 1.2 = {Tu / 1.2:.3f} s")


def main():
    # Ziegler-Nichols identification: P, Kp = 60, logged every 3 ms
    chatter = load("part4_chatter_Kp60_3ms.csv")
    limit_cycle_stats(chatter, 3.0, 99)
    zoom = crop(chatter, 4.0, 4.3)
    speed_pwm_plot(zoom, "Part 4: limit cycle with P, Kp = 60 (3 ms logging, 300 ms window)",
                   "part4_chatter_Kp60_3ms_zoom.png", xlabel="Time (s), window starting 4 s after Operational entry")

    # Step response
    pi_step = load("part4_tune1_kp34_Ti003.csv")
    p_step = load("part4_Pcont_Kp40.csv")
    step_stats("PI", pi_step)
    step_stats("P ", p_step)
    speed_pwm_plot(pi_step, "Part 4: ZN-tuned PI, Kp = 34, Ti = 0.03 s", "part4_tune1_kp34_Ti003.png")
    speed_pwm_plot(p_step, "Part 4: P controller, Kp = 40", "part4_Pcont_Kp40.png")

    fig, ax = plt.subplots(figsize=(10, 5))
    ax.plot(pi_step[0], pi_step[1], "--", color="black", linewidth=1, label="Reference speed")
    ax.plot(p_step[0], p_step[2], color="tab:orange", linewidth=1, label="P, Kp = 40")
    ax.plot(pi_step[0], pi_step[2], color="tab:green", linewidth=1, label="PI, Kp = 34, Ti = 0.03 s (ZN)")
    ax.set_xlim(0, min(p_step[0][-1], pi_step[0][-1]))
    ax.set_xlabel("Time since Operational entry (s)")
    ax.set_ylabel("Speed (RPM)")
    ax.grid(alpha=0.3)
    ax.legend(loc="lower right")
    ax.set_title("Part 4: Step response, P vs PI")
    fig.tight_layout()
    fig.savefig(LOGS / "part4_step_P_vs_PI.png", dpi=150)
    plt.close(fig)

    # Load response (load applied by hand, intervals read off the PWM)
    pi_load = crop(load("part4_PI_loadApplied.csv"), 0, 25.2)
    p_load = load("part4_P_loadapplied.csv")
    speed_pwm_plot(pi_load, "Part 4: Load response, PI (Kp = 34, Ti = 0.03 s)", "part4_PI_loadApplied.png",
                   spans=((4.8, 12.5, "tab:red", "Heavy load (saturates)"),
                          (18.0, 23.9, "tab:purple", "Moderate load")))
    speed_pwm_plot(p_load, "Part 4: Load response, P (Kp = 40)", "part4_P_loadapplied.png",
                   spans=((3.6, 10.4, "tab:purple", "Load applied"),))

    fig, axs = plt.subplots(2, 1, figsize=(10, 6), sharex=True)
    for ax, data, label, color, onset, dur in (
            (axs[0], p_load, "P, Kp = 40", "tab:orange", 3.6, 6.8),
            (axs[1], pi_load, "PI, Kp = 34, Ti = 0.03 s", "tab:green", 18.0, 5.9)):
        t, r, s, u = crop(data, onset - 2, onset + 8)
        ax.axvspan(2, 2 + dur, color="tab:purple", alpha=0.08, label="Load applied")
        ax.plot(t, r, "--", color="black", linewidth=1, label="Reference")
        ax.plot(t, s, color=color, linewidth=1, label=label)
        ax.set_ylim(28, 45)
        ax.set_ylabel("Speed (RPM)")
        ax.grid(alpha=0.3)
        ax.legend(loc="lower right", ncol=3)
    axs[0].set_title("Part 4: Load response, P vs PI (load onset aligned at 2 s)")
    axs[1].set_xlabel("Time (s)")
    fig.tight_layout()
    fig.savefig(LOGS / "part4_load_P_vs_PI.png", dpi=150)
    plt.close(fig)
    print("Plots saved in", LOGS)


if __name__ == "__main__":
    main()
