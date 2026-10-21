#!/usr/bin/env python3
"""
Part 3.1 tracking controller: target 0 -> 40 -> 0 RPM with and without the acceleration limit,
logged on the Raspberry Pi from TPDO1 by canopen_client (PI, Kp = 34, Ti = 0.03 s).

The setpoint generator's reference is not sent on TPDO1, so it is reconstructed here from the
target and the limit (a ramp of `limit` RPM/s, or the target itself for limit 0).
"""
import statistics as st

import matplotlib.pyplot as plt

from plot_part3_2 import read_client, LOGS, PWM_LIMIT

RUNS = (("trap_acc0.csv", 0, "No limit (acc = dec = 0, true step)"),
        ("trap_acc100.csv", 100, "Limit 100 RPM/s"))


def reference(t, target, limit):
    """Reference from the setpoint generator model: move towards the target by at most limit * dt."""
    ref, out = 0.0, []
    for i, (ti, tg) in enumerate(zip(t, target)):
        dt = ti - t[i - 1] if i else 0.0
        step = limit * dt
        if limit == 0 or abs(tg - ref) <= step:
            ref = tg
        else:
            ref += step if tg > ref else -step
        out.append(ref)
    return out


def stats(name, t, target, speed, pwm, ref):
    """Tracking error in the 1 s after each target change, and saturation."""
    changes = [t[i] for i in range(1, len(t)) if target[i] != target[i - 1]]
    win = [i for i, ti in enumerate(t) if any(c <= ti < c + 1 for c in changes)]
    e_target = [abs(target[i] - speed[i]) for i in win]
    e_ref = [abs(ref[i] - speed[i]) for i in win]
    sat = sum(abs(u) >= 0.999 * PWM_LIMIT for u in pwm)
    print(f"{name}: |target - speed| mean {st.mean(e_target):.2f} max {max(e_target)}, "
          f"|reference - speed| mean {st.mean(e_ref):.2f} max {max(e_ref):.1f}, "
          f"PWM {min(pwm):.0f} to {max(pwm):.0f}, {sat} samples at the limit")


def main():
    fig, axs = plt.subplots(2, 2, figsize=(12, 6), sharex=True, sharey="row", gridspec_kw={"height_ratios": [2, 1]})
    for col, (name, limit, title) in enumerate(RUNS):
        t, target, speed, pwm = read_client(name)
        ref = reference(t, target, limit)
        stats(title, t, target, speed, pwm, ref)
        axs[0][col].plot(t, target, "--", color="black", linewidth=1, label="Target (RPDO1)")
        if limit:
            axs[0][col].plot(t, ref, ":", color="tab:blue", linewidth=1.5, label="Reference (setpoint generator)")
        axs[0][col].plot(t, speed, color="tab:orange", linewidth=1, label="Actual speed")
        axs[0][col].set_title(title)
        axs[0][col].legend(loc="center")
        axs[1][col].plot(t, pwm, color="tab:red", linewidth=0.6)
        axs[1][col].axhline(0, color="black", linewidth=0.5)
        axs[1][col].set_ylim(-PWM_LIMIT * 1.1, PWM_LIMIT * 1.1)
        axs[1][col].set_xlabel("Time since Operational entry (s)")
        for row in (0, 1):
            axs[row][col].grid(alpha=0.3)
    axs[0][0].set_ylabel("Speed (RPM)")
    axs[1][0].set_ylabel("Controller output (PWM)")
    fig.suptitle("Part 3.1: target 0 -> 40 -> 0 RPM, PI Kp = 34, Ti = 0.03 s")
    fig.tight_layout()
    fig.savefig(LOGS / "part3_1_trapezoid.png", dpi=150)
    plt.close(fig)
    print("Plot saved in", LOGS)


if __name__ == "__main__":
    main()
