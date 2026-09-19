#!/usr/bin/env python3
"""
Capture the Nano's `time_ms,ref_speed,actual_speed,pwm_value` serial stream
(printed by print_data() in main.cpp) to a CSV file.

Opening the serial port resets the Arduino Nano (DTR toggle), so this
naturally re-triggers the firmware's boot-then-step sequence - start
capturing right when you run this script.

Edit the settings below, then just run:
    python3 capture_serial.py
"""
import sys
import time

import serial
import serial.tools.list_ports

# --- Edit these, then run the script ---
OUT_PATH = "test_new_build.csv"
SECONDS = 15.0          # how long to capture (8 steps x 3s dwell for the open-loop staircase = 24s)
PORT = None             # e.g. "/dev/cu.usbserial-110", or None to auto-detect
BAUD = 115200
# ----------------------------------------

EXPECTED_HEADER = "time_ms,ref_speed,actual_speed,pwm_value"


def find_port() -> str:
    candidates = [
        p.device
        for p in serial.tools.list_ports.comports()
        if p.vid is not None  # real USB devices report a vendor id; Bluetooth/debug ports don't
    ]
    if not candidates:
        sys.exit(
            "No USB serial device found. Plug in the Nano, or set PORT explicitly "
            "(see available ports with: python3 -m serial.tools.list_ports)"
        )
    if len(candidates) > 1:
        print(f"Multiple serial devices found: {candidates}. Using the first one: {candidates[0]}")
    return candidates[0]


def main():
    port = PORT or find_port()
    print(f"Opening {port} at {BAUD} baud (this will reset the Nano)...")

    lines = []
    with serial.Serial(port, BAUD, timeout=1) as ser:
        time.sleep(2.0)  # let the bootloader reset settle before the firmware starts printing
        ser.reset_input_buffer()

        start = time.monotonic()
        while time.monotonic() - start < SECONDS:
            raw = ser.readline().decode("utf-8", errors="ignore").strip()
            if not raw:
                continue

            if raw == EXPECTED_HEADER:
                if not lines:
                    lines.append(raw)
                continue

            parts = raw.split(",")
            if len(parts) != 4:
                continue  # skip boot noise / partial lines
            try:
                [float(p) for p in parts]
            except ValueError:
                continue

            lines.append(raw)
            print(raw)

    if len(lines) <= 1:
        sys.exit("No valid data captured - check wiring/port/baud and try again.")

    if lines[0] != EXPECTED_HEADER:
        lines.insert(0, EXPECTED_HEADER)

    with open(OUT_PATH, "w", newline="") as f:
        f.write("\n".join(lines) + "\n")

    print(f"\nSaved {len(lines) - 1} rows to {OUT_PATH}")


if __name__ == "__main__":
    main()
