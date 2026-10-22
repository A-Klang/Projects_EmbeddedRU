# Project 4: Distributed control over CANopen with Zephyr

Motor controller from Project 3 as a CANopen node (Node ID 1) on a Nucleo-L432KC running Zephyr, managed over CAN at 125 kbit/s from a Raspberry Pi Zero 2 W with a SocketCAN client.

## Where to find each part

| Part | Location | Notes |
| --- | --- | --- |
| Part 1, port to Zephyr (keyboard interface) | `Project4/` at commit `d0c1ccd` | `git checkout d0c1ccd -- Project4` or browse that commit on GitHub. Later commits replaced the keyboard interface in `Project4/` with Part 2. |
| Part 2.1, CAN bring-up example | `Project4_cantest/` | Example code from L4.4, sending id 0x456 (4 bytes) and receiving 0x321 through a filter and message queue |
| Part 2.2, reduced CANopen node | `Project4/` | Superloop version with NMT, Boot-up, SDO, RPDO1, TPDO1 |
| Part 3.1, native Zephyr tasks and setpoint generator | `Project4_part3/` (this folder) | Final Nucleo code |
| Part 3.2, Linux control application | `Project4_part3/rpi/canopen_client.c` | Raspberry Pi code |
| Results and report material | `Project4_part3/results/results_P4.md` | Covers all parts, with screenshots in `results/` and plots in `logs/` |

## Layout of this folder

| Path | Contents |
| --- | --- |
| `src/`, `include/` | Drivers (encoder, PWM, GPIO, timer), state classes, `MotorApp`, Zephyr CAN adapter, `main.cpp` with the task setup |
| `lib/canopen/` | CANopen layer: NMT, SDO server, PDOs, object dictionary. No Zephyr calls, only the `ICan` interface |
| `lib/controllers/` | P and PI controllers, setpoint generator |
| `test/test_setpoint/` | Unity tests for the setpoint generator |
| `zephyr/` | `prj.conf` and the board overlay `boards/nucleo_l432kc.overlay` |
| `rpi/` | `canopen_client.c` for the Raspberry Pi |
| `docs/` | Architecture diagrams (PlantUML and SVG), Part 2 and Part 3 |
| `logs/` | Logged runs (CSV, candump) and plots |
| `tools/` | Python plot scripts (`plot_part1.py`, `plot_part3_1.py`, `plot_part3_2.py`) |

## Building and running

**Nucleo** (PlatformIO, Zephyr): open this folder in PlatformIO and run Build and Upload for the `nucleo_l432kc` environment, or from the command line:

```bash
pio run -e nucleo_l432kc -t upload
```

Setpoint generator tests on the host:

```bash
pio test -e native
```

**Raspberry Pi**: bring up the CAN interface and build the client:

```bash
sudo ip link set can0 up type can bitrate 125000
gcc -Wall -O2 -o canopen_client canopen_client.c
./canopen_client can0 log.csv 100
```

Arguments: CAN interface, CSV log file, profile acceleration and deceleration in RPM/s (0 = true step, default 1000). The client resets the node, configures it over SDO with read-back, sets it Operational, runs the target 0 → 40 → 0 RPM (4 s each) on RPDO1 while logging TPDO1 to the CSV file, and returns the node to Pre-operational on exit or Ctrl-C.

## Videos

- Part 1, Zephyr port with the keyboard interface: <https://youtube.com/shorts/efe7f53iNW0>
- Part 3.2, Linux client over CAN: <https://youtube.com/shorts/omiR3ibQbaE>
