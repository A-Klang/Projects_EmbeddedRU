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

## Part 2.2 hardware test procedure

The following procedure covers the requested service and NMT-state tests. Run
`candump` in one Raspberry Pi terminal and send the commands from another:

```sh
candump can0
```

The outputs shown below are expected values. Replace them in the report with
the actual `candump`, `canbusload`, and oscilloscope results measured on the
hardware.

### NMT states and Boot-up

```sh
# Set node 1 Operational
cansend can0 000#01.01

# Stop node 1
cansend can0 000#02.01

# Set node 1 Pre-operational
cansend can0 000#80.01

# Reset node; candump should show 701#00
cansend can0 000#81.01

# Reset communication; candump should show 701#00
cansend can0 000#82.01
```

| Service | Pre-operational | Operational | Stopped |
|---|---|---|---|
| NMT | Accepted | Accepted | Accepted |
| SDO | Accepted | Accepted | Ignored |
| RPDO1 | Ignored | Accepted | Ignored |
| TPDO1 | Not sent | Sent at 50 Hz | Not sent |

### SDO responses and aborts

In Pre-operational, read the Device type and request an object that does not
exist:

```sh
cansend can0 601#40.00.10.00.00.00.00.00
cansend can0 601#40.99.99.00.00.00.00.00
```

The missing object must produce:

```text
581#8099990000000206
```

The last four bytes are abort code `0x06020000` in little-endian order. Next,
enter Operational and attempt to write the configuration object Kp:

```sh
cansend can0 000#01.01
cansend can0 601#23.01.20.00.00.00.00.3F
```

The expected response is:

```text
581#8001200022000008
```

This contains abort code `0x08000022`, because configuration writes are only
permitted in Pre-operational.

### Controller configuration and process data

Return to Pre-operational and configure the PI controller:

```sh
cansend can0 000#80.01

# Controller type = PI
cansend can0 601#2F.00.20.00.01.00.00.00

# Kp = 0.5 (REAL32 0x3F000000)
cansend can0 601#23.01.20.00.00.00.00.3F

# Ti = 0.1 s (REAL32 0x3DCCCCCD)
cansend can0 601#23.02.20.00.CD.CC.CC.3D
```

Then enter Operational and send a target velocity of 1500 RPM:

```sh
cansend can0 000#01.01
cansend can0 201#DC.05.00.00
```

TPDO1 frames with COB-ID `0x181` should now appear every 20 ms:

```text
can0  181  [6]  vv vv vv vv dd dd
```

Bytes 0-3 (`vv`) contain the signed `INTEGER32` actual velocity in RPM. Bytes
4-5 (`dd`) contain the signed `INTEGER16` duty cycle in units of 0.1%. Both
values use little-endian byte order. Sending Stop or Pre-operational must stop
the periodic TPDO1 transmission.

### Bus-load calculation

The selected TPDO1 rate is 50 Hz: one TPDO1 every 20 control updates at a
1 kHz control rate. A standard CAN data frame with six data bytes uses 95 bits
including intermission before stuffing, or approximately 115 bits with a
conservative worst-case stuffing allowance:

```text
50 frames/s * 115 bits/frame = 5,750 bit/s
5,750 / 125,000 * 100 = 4.6% bus load
```

For an example traffic budget of 10 RPDO1 frames/s, 10 complete SDO
transactions/s, one NMT frame/s, and one Boot-up frame/s, the conservative
total is approximately:

| Traffic | Approximate worst-case rate |
|---|---:|
| 50 TPDO1/s | 5,750 bit/s |
| 10 RPDO1/s | 950 bit/s |
| 10 SDO request/response pairs/s | 2,700 bit/s |
| NMT and Boot-up | 140 bit/s |
| **Total** | **9,540 bit/s** |

The theoretical minimum is therefore close to 10 kbit/s for these assumptions,
but that would leave almost no margin. Allowing at most 50% utilisation gives a
minimum of about 20 kbit/s. The selected 125 kbit/s bitrate is comfortably
above this requirement and gives an estimated total load of about 7.6%.

Measure the real load while the node is Operational:

```sh
canbusload can0@125000
```

With only the Raspberry Pi and the Nucleo on the bus, enable a 120-ohm
termination resistor at both ends. Configure the oscilloscope CAN decoder for
125 kbit/s and include at least one decoded frame in the report.
