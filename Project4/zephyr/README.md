# Project 4 - Part 2 CANopen node

This directory contains the Nucleo-L432KC Zephyr application for CANopen node
ID 1. The portable protocol code has no Zephyr dependency; only
`zephyr_can_adapter.cpp` uses the Zephyr CAN API.

## Implemented services

| Service | COB-ID | State |
|---|---:|---|
| NMT | `0x000` | All states |
| Boot-up | `0x701` | On startup and after reset |
| SDO request/response | `0x601` / `0x581` | Pre-operational, Operational |
| RPDO1 | `0x201` | Operational |
| TPDO1 | `0x181` | Operational, every 20 control updates (50 Hz) |

The SDO server supports expedited transfers of one, two and four bytes. It
implements the required `0x06020000` (missing object) and `0x08000022`
(wrong NMT state) abort codes, plus read-only, size, command and range errors.

## Build and upload

From the `Project4` directory:

> PlatformIO's Zephyr integration does not accept spaces in the project path.
> Open or copy the repository from a path such as `~/Embeded_Prog/...` before
> running these commands.

```sh
pio run
pio run --target upload
pio device monitor
```

The firmware uses 125 kbit/s CAN. The Raspberry Pi interface must use the same
bitrate:

```sh
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 125000
sudo ip link set can0 up
candump can0
```

## Host tests

The CANopen layer can be tested without the Nucleo:

```sh
cmake -S zephyr/tests/host -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

## Basic acceptance sequence

After reset, `candump` should show `701#00`.

```sh
# Read device type in Pre-operational
cansend can0 601#40.00.10.00.00.00.00.00

# Select PI and set Kp = 0.5
cansend can0 601#2F.00.20.00.01.00.00.00
cansend can0 601#23.01.20.00.00.00.00.3F

# Set Ti = 0.1 s (IEEE-754 0x3DCCCCCD)
cansend can0 601#23.02.20.00.CD.CC.CC.3D

# Start node 1
cansend can0 000#01.01

# Target velocity = 1500 RPM
cansend can0 201#DC.05.00.00

# This configuration write must now abort with 0x08000022
cansend can0 601#23.01.20.00.00.00.00.3F

# Return to Pre-operational
cansend can0 000#80.01
```

## TPDO bus-load basis

A six-data-byte standard CAN frame occupies 95 bits including intermission
before stuffing, and at most about 115 bits using a conservative worst-case
stuffing estimate. At 50 TPDO/s this is approximately 5.75 kbit/s, or 4.6% of
a 125 kbit/s bus. RPDO, SDO, NMT and Boot-up traffic must be added to the final
report calculation and checked with `canbusload can0@125000`.
