#!/usr/bin/env python3
"""
Capture the Nano's `actual_rpm,pwm_value` serial stream to a CSV file.

Opening the serial port resets the Arduino Nano (DTR toggle), so this
naturally re-triggers the firmware's boot-delay-then-step sequence -
start capturing right when you run this script.

Usage:
    python3 capture_serial.py --out step_response.csv --seconds 10
    python3 capture_serial.py --out load_response.csv --seconds 20 --port /dev/cu.usbserial-XXXX

Each printed line is assumed to arrive on a fixed cadence (matching the
firmware's print interval, 20ms by default) so a time column is added
from the row index rather than relying on host-side timestamps.
"""
import argparse
import csv
import sys
import time

import serial
import serial.tools.list_ports


def find_port() -> str:
    candidates = [
        p.device
        for p in serial.tools.list_ports.comports()
        if p.vid is not None  # real USB devices report a vendor id; Bluetooth/debug ports don't
    ]
    if not candidates:
        sys.exit(
            "No USB serial device found. Plug in the Nano, or pass --port explicitly "
            "(see available ports with: python3 -m serial.tools.list_ports)"
        )
    if len(candidates) > 1:
        print(f"Multiple serial devices found: {candidates}. Using the first one: {candidates[0]}")
    return candidates[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", required=True, help="Output CSV file path")
    parser.add_argument("--seconds", type=float, default=10.0, help="How long to capture (default: 10s)")
    parser.add_argument("--port", default=None, help="Serial port (default: auto-detect)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument(
        "--print-interval-ms",
        type=float,
        default=20.0,
        help="Firmware's print interval in ms, used to build the time column (default: 20)",
    )
    args = parser.parse_args()

    port = args.port or find_port()
    print(f"Opening {port} at {args.baud} baud (this will reset the Nano)...")

    rows = []
    with serial.Serial(port, args.baud, timeout=1) as ser:
        # Let the reset settle and flush whatever partial garbage line follows it.
        time.sleep(0.5)
        ser.reset_input_buffer()

        start = time.monotonic()
        while time.monotonic() - start < args.seconds:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue
            parts = line.split(",")
            if len(parts) != 2:
                continue  # skip anything that isn't "actual,pwm"
            try:
                actual_rpm, pwm_value = float(parts[0]), float(parts[1])
            except ValueError:
                continue
            t_s = len(rows) * args.print_interval_ms / 1000.0
            rows.append((t_s, actual_rpm, pwm_value))
            print(f"{t_s:6.2f}s  actual={actual_rpm:7.2f}  pwm={pwm_value:7.2f}")

    if not rows:
        sys.exit("No valid data captured - check wiring/port/baud and try again.")

    with open(args.out, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["time_s", "actual_rpm", "pwm_value"])
        writer.writerows(rows)

    print(f"\nSaved {len(rows)} rows to {args.out}")


if __name__ == "__main__":
    main()
