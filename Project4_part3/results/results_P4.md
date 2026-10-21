## analog_out.h

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Timer_usec | uses a uint32_t for a period in nanoseconds, using built-in timer on zephyr | don't need our own timer, will use zephyr built-in timer instead |

Lines changed: +2 / -2

## analog_out.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Timer and pin initialization, set_duty_cycle() and ISRs | setting the pwm directly with pwm_set_dt and calculating the pulse from duty_cycle and period_ns | the timer hardware makes the PWM signal itself, so no ISRs or CPU time are needed |

Lines changed: +6 / -17

## digital_in.h, digital_in.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Pin mask on port D, DDRD and PORTD for input with pull-up, PIND to read | gpio_dt_spec from the overlay aliases (c1-pin, c2-pin, flt-pin), gpio_pin_configure_dt and gpio_pin_get_dt | the pin and pull-up are described in the overlay, so the class refers to pins by name and not by register |

Lines changed: +1 / -2 (h), +30 / -8 (cpp)

## digital_out.h, digital_out.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Pin mask on port B, DDRB for output, PORTB to set, clear and toggle | gpio_dt_spec from the overlay aliases (gpio-dir, led0), gpio_pin_configure_dt, gpio_pin_set_dt and gpio_pin_toggle_dt | same as Digital_in, the pins come from the overlay |

Lines changed: +1 / -1 (h), +24 / -7 (cpp)

## encoder.h

Only the comment on update() changed, it is now called from the C1 GPIO callback instead of ISR(PCINT2_vect). Lines changed: +1 / -2

## timer_msec.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Timer1 in CTC mode with a compare match interrupt, ISR in encoder.cpp | k_timer (K_TIMER_DEFINE, k_timer_start) that calls timer_msec_tick() every period | the kernel timer works on any board, no timer registers needed |
| set_duty_cycle() writing OCR1B | empty function, kept for the interface | OCR1B was never used, the function is kept so the interface is unchanged |

Lines changed: +13 / -20

## encoder.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Pin-change interrupt on C1 (PCMSK2, PCICR, ISR(PCINT2_vect), portd_history) | GPIO callback on both edges of C1 (gpio_pin_interrupt_configure_dt, gpio_init_callback, gpio_add_callback) | the GPIO driver sets up the interrupt and only calls us when C1 changes |
| ATOMIC_BLOCK in get_speed() | irq_lock() / irq_unlock() | same protection as before, interrupts are blocked while a shared value is read |
| ISR(TIMER1_COMPA_vect) | timer_msec_tick(), called by the k_timer in timer_msec.cpp | the k_timer calls a normal function instead of a hardware interrupt vector |

Lines changed: +25 / -22

## nmt_states.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Serial.print / Serial.println for status messages (Arduino) | printk | Serial is part of Arduino, which does not exist in Zephyr |
| ATOMIC_BLOCK when reading ms_since_start in Operational::on_entry() | irq_lock() / irq_unlock() | same protection as before, interrupts are blocked while a shared value is read |
| Arduino.h providing fabs() | math.h | fabs() used to come in through Arduino.h |

Lines changed: +11 / -22

## main.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Serial.begin() and sei() | removed, Zephyr starts the UART and enables interrupts itself | Zephyr does this at startup |
| Serial.print / Serial.println for the CSV output | printk | Serial is part of Arduino, which does not exist in Zephyr |
| Serial.available() / Serial.read() for commands | uart_poll_in() on the console UART (non-blocking) | reads a character without stopping the main loop |
| Serial.readStringUntil(), String, trim(), toDouble() for Kp, Ti and control law | read_line() helper with uart_poll_in(), char buffer and atof() | String and readStringUntil() are Arduino only |
| ATOMIC_BLOCK when reading ms_since_start | irq_lock() / irq_unlock() | same protection as before, interrupts are blocked while a shared value is read |

Lines changed: +48 / -29

## Lines changed per file

| File | Lines added / removed |
| --- | --- |
| analog_out.h | +2 / -2 |
| analog_out.cpp | +6 / -17 |
| digital_in.h | +1 / -2 |
| digital_in.cpp | +30 / -8 |
| digital_out.h | +1 / -1 |
| digital_out.cpp | +24 / -7 |
| timer_msec.cpp | +13 / -20 |
| encoder.h | +1 / -2 |
| encoder.cpp | +25 / -22 |
| nmt_states.cpp | +11 / -22 |
| main.cpp | +48 / -29 |
| timer_usec.h, timer_usec.cpp | removed (-43) |
| timer_msec.h, context.h, context.cpp, state.h, nmt_states.h, controller.h, controllers.h, controllers.cpp | unchanged |

The counts for encoder.cpp and main.cpp include the debug pin for the latency measurement and the test profile below.

## Behaviour compared to Project 3

Tested with the same sequence as in Project 3, log in `logs/part1_step_PI_kp34_Ti003.log`.

- Encoder count and speed: the speed follows the reference in both directions and settles at 40.00, 59.7 and -40.00 RPM, with the same one count (2.86 RPM) steps as before.
- State machine: Boot-up is printed on reset, `p` and `o` switch between Pre-operational and Operational, and the profile restarts from Operational entry.
- Fault detection: pressing the button that pulls FLT low stops the motor (PWM 0) while the reference is still -40 RPM, so the node goes to Stopped like in Project 3.

## Encoder interrupt response time

D11 is set high at the start of the C1 GPIO callback and low at the end. CH1 is C1 (D4), CH2 is D11.

![ISR latency](isr_latency.png)

| | Time |
| --- | --- |
| C1 edge to start of callback | 4.64 µs |
| Callback duration | 3.62 µs |

In Project 1, Part 3 the time budget per quadrature state was 142.75 µs at full motor speed, and no pulses were missed with interrupts. The response time on Zephyr is about 3% of that, so the port has plenty of margin.

## PWM output

CH1 is the PWM pin (D9), CH2 is the direction pin (D6). The motor driver inputs are wired the other way round from the overlay, D9 to AIN2 and D6 to AIN1.

| Direction | D6 | Frequency | Duty on D9 | Duty seen by the motor |
| --- | --- | --- | --- | --- |
| Forward, 40 RPM | low (-21 mV) | 8.32 kHz | 50.8 % | 50.8 % |
| Reverse, -40 RPM | high (3.28 V) | 8.33 kHz | 47.9 % | 52.1 % |

![PWM forward](pwm_forward.png)

![PWM reverse](pwm_reverse.png)

The frequency matches the 120 µs period set in the overlay. In reverse the direction pin is high, so the motor is driven while D9 is low and the duty is inverted (255 - |u|), as in Project 3.

## Step response

Same PI parameters as in Project 3 (Kp = 34, Ti = 0.03 s) and the same profile, 0 RPM until 1 s, 40 RPM until 8 s, then 60 RPM. Plot made with `tools/plot_part1.py`.

![Step response P3 vs P4](../logs/part1_step_P3_vs_P4.png)

| | Project 3 (Nano) | Project 4 (Nucleo, Zephyr) |
| --- | --- | --- |
| 0 ==> 40 RPM, settles at | 40.00 RPM | 40.00 RPM |
| Rise time (to 90%) | 30 ms | 30 ms |
| 40 ==> 60 RPM, settles at | 59.71 RPM | 59.72 RPM |
| Rise time (to 90%) | 41 ms | 30 ms |
| Overshoot | one encoder count | one encoder count |

The response is the same as in Project 3, the only difference is a slightly faster rise on the second step.


# Part 2: CAN and a reduced CANopen interface

## CAN bring-up

The Nucleo is connected to the CAN Pal (3V3, GND, D10 to RX, D2 to TX), and the CAN HAT sits on the Raspberry Pi. CAN_H, CAN_L and GND are wired between the two. The termination was switched on at both ends, measured as 60 Ω between CAN_H and CAN_L. The bus runs at 125 kbit/s on both sides, `can0` is brought up with `sudo ip link set can0 up type can bitrate 125000`.

Frames sent by the Nucleo with the example code from L4.4 (id 0x123, 8 bytes, every 100 ms), seen with `candump can0`:

![candump 0x123](part2_1_candump_123.png)

After changing the id to 0x456 and the length to 4 bytes:

![candump 0x456](part2_1_candump_456.png)

Frames sent from the Raspberry Pi with `cansend`, and `candump` filtered on id 0x321:

![cansend](part2_1_cansend.png)

![candump filtered on 0x321](part2_1_candump_filter_321.png)

Only the 0x321 frame is shown, the 0x456 frames from the Nucleo and the 0x322 frame are filtered out.

Reception on the Nucleo uses a filter on id 0x321 and a message queue (`CAN_MSGQ_DEFINE()`, `can_add_rx_filter_msgq()`), and the frames are printed from the main loop. The 0x321 frame is received, the 0x322 frame is not:

![Nucleo reception](part2_1_nucleo_rx_321.png)

Frame decoded by the oscilloscope on CAN_H:

![Decoded CAN frame](part2_1_scope_can_frame.png)

The decoder shows id 0x456, DLC 4, data 01 02 03 04, the CRC and the ACK from the Raspberry Pi.

## Architecture

![Architecture Part 2](../docs/architecture_part2.svg)

| Layer | OSI | Contents |
| --- | --- | --- |
| Motor application | 7 | State machine, controllers, encoder and motor from Project 3, `MotorApp` |
| CANopen | 7 | NMT, Boot-up, SDO server, RPDO1, TPDO1, object dictionary, little-endian packing (`lib/canopen`) |
| CAN driver | 2 | `ZephyrCan`, which implements the `ICan` interface with the Zephyr CAN driver and the STM32 CAN controller |
| Transceiver | 1 | CAN Pal and CAN HAT, wiring and termination |

CANopen does not use OSI layers 3 to 6.

- The application does not know about COB-IDs, byte order or the frame format. The CANopen layer reads and writes it through the `NodeApp` interface, implemented by `MotorApp` with the Project 3 `Context`, encoder and controllers.
- The byte packing, the access rules and the abort codes are all in the CANopen layer (`canopen_node.cpp`, `object_dictionary.cpp`).
- The CANopen layer has no Zephyr calls. It sends and receives frames only through `ICan`.

## Testing the services

All tests from the Raspberry Pi with `cansend`, responses from `candump can0`.

### NMT and Boot-up

| Sent | Result |
| --- | --- |
| Reset button | `701 [1] 00`, LED blinks at 1 Hz (Pre-operational) |
| `000#01.01` | LED on (Operational) |
| `000#02.01` | LED blinks at 2 Hz (Stopped) |
| `000#80.01` | LED blinks at 1 Hz (Pre-operational) |
| `000#81.01` (reset node) | `701 [1] 00` |
| `000#82.01` (reset communication) | `701 [1] 00` |
| `000#01.00` (all nodes) | LED on (Operational) |
| `000#80.02` (node 2) | no change |

![NMT](part2_2_nmt_candump.png)

### SDO

| Test | Request on 0x601 | Response on 0x581 |
| --- | --- | --- |
| Read device type | `40 00 10 00 00 00 00 00` | `43 00 10 00 92 01 00 00` |
| Read Kp (40.0) | `40 01 20 00 00 00 00 00` | `43 01 20 00 00 00 20 42` |
| Write Kp = 34.0 | `23 01 20 00 00 00 08 42` | `60 01 20 00 00 00 00 00` |
| Write Ti = 0.03 | `23 02 20 00 8F C2 F5 3C` | `60 02 20 00 00 00 00 00` |
| Read controller type (PI) | `40 00 20 00 00 00 00 00` | `4F 00 20 00 01 00 00 00` |
| Object that does not exist | `40 00 30 00 00 00 00 00` | `80 00 30 00 00 00 02 06`, abort 0x06020000 |
| Write to read-only object | `23 00 10 00 01 00 00 00` | `80 00 10 00 02 00 01 06`, abort 0x06010002 |
| Write Kp in Operational | `23 01 20 00 00 00 08 42` | `80 01 20 00 22 00 00 08`, abort 0x08000022 |
| Read in Stopped | `40 00 10 00 00 00 00 00` | no response |

![SDO tests](part2_2_sdo_tests.png)

### RPDO1 and TPDO1

With the node Operational, `201#28.00.00.00` sets the target velocity to 40 RPM. The velocity actual value and the duty cycle read back over SDO:

![RPDO1 and SDO read-back](part2_2_rpdo_40rpm.png)

The velocity is 0x28 = 40 RPM (0x2B = 43 is one encoder count), the duty cycle around 0x1C5 = 45.3 %.

TPDO1 is sent every 10 ms in Operational, with the velocity in bytes 0-3 and the duty cycle in 0.1 % in bytes 4-5 (`candump -td can0,181:7FF`):

![TPDO1](part2_2_tpdo1.png)

### Behaviour in each NMT state

| Service | Pre-operational | Operational | Stopped |
| --- | --- | --- | --- |
| NMT | accepted | accepted | accepted |
| SDO | answered | answered, configuration objects aborted with 0x08000022 | no response |
| RPDO1 | ignored, motor stays still | target velocity set | ignored |
| TPDO1 | not sent | sent every 10 ms | not sent |

## TPDO rate and bus load

TPDO1 is sent on every 10th control update, so at 100 Hz. That is the rate the speed measurement changes at, since the speed is computed over a 10 ms window.

A standard frame with n data bytes has 47 + 8n bits including the 3 bit interframe space. In the worst case a stuff bit is added after every 4 bits in the 34 + 8n bits from SOF to the CRC, which is ⌊(33 + 8n)/4⌋ extra bits.

| Frame | Bytes | Bits | Stuff bits (worst case) | Total |
| --- | --- | --- | --- | --- |
| TPDO1 | 6 | 95 | 20 | 115 |
| RPDO1 | 4 | 79 | 16 | 95 |
| SDO request or response | 8 | 111 | 24 | 135 |
| NMT | 2 | 63 | 12 | 75 |

The other frames are sent by the Raspberry Pi. As a worst case RPDO1 is also sent at 100 Hz, plus 10 SDO transfers per second (request and response), and NMT is negligible:

| Frame | Rate | Load |
| --- | --- | --- |
| TPDO1 | 100 Hz | 11.5 kbit/s |
| RPDO1 | 100 Hz | 9.5 kbit/s |
| SDO | 10 transfers/s | 2.7 kbit/s |
| Total | | 23.7 kbit/s |

That is 19 % of 125 kbit/s. The minimum bitrate at 100 % load would be 23.7 kbit/s. With the usual limit of about 50 % bus load it is 47.4 kbit/s, so the lowest standard bitrate that works is 50 kbit/s.

Bus load measured with `canbusload -t -b -r -e can0@125000`, with only TPDO1 on the bus:

![canbusload](part2_2_canbusload.png)

100 frames/s and 10207 bit/s, so 8 % load. That is about 102 bits per frame, between 95 without stuffing and the worst case of 115, which fits the calculated 11.5 kbit/s (9.2 %) as an upper bound.

## Parameters over SDO, target on RPDO1, check on TPDO1

In Pre-operational, Kp = 34 and Ti = 0.03 s were written over SDO (0x2001, 0x2002). Then the node was set Operational and 40 RPM sent on RPDO1. On TPDO1 the speed goes from 0 to 0x28 = 40 RPM in about 40 ms and then stays at 37-43 RPM (one encoder count), and the duty cycle settles around 0x1FC = 50.8 %, see the TPDO1 screenshot above.

# Part 3: Zephyr task design and Linux client

Raw results, to be written up properly later.

## Control-loop jitter (3.1)

**Setup.** D11 (PB5) is toggled at the start of every control update (`dbg_pin.toggle()` right after `k_sem_take(&control_sem)` in `controller_task`). The control period is 1 ms, so D11 is a 500 Hz square wave and every half-period is one control interval. Scope (R&S RTB2004) on D11, Autoset, PW+ and PW− measurements with statistics, about 10 s per run. The node was Operational with target 0 (wheel still), so the control loop and TPDO1 run as normal. Note: D11 read 0 V at first because the Nucleo had an older build without the toggle; reflashing fixed it.

**Load.** `cangen can0 -g 1 -I 201 -L 4 -D 00000000` on the Pi. Random IDs (plain `cangen -g 1`) were not used since they can hit NMT (0x000), RPDO1 (0x201) or SDO (0x601), and all other IDs are dropped by the hardware filter, so the CPU would never see them. RPDO1 frames with target 0 pass the filter and go through the CAN RX ISR, message queue and CAN thread about 1000 times per second without changing what the motor does. `canbusload` showed 1007 frames/s, 97665 bit/s, 78 % load (screenshot taken with the earlier 40 RPM payload, same frame size).

![bus load under cangen](part3_1_busload_cangen.png)

| Run | Measurement | Min | Max | Mean | Std dev | Peak-to-peak | Count |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Idle | PW+ | 1.000960 ms | 1.001926 ms | 1.001396 ms | 0.177 µs | 0.97 µs | 409 |
| Idle | PW− | 1.000822 ms | 1.002018 ms | 1.001384 ms | 0.175 µs | 1.20 µs | 410 |
| 78 % load | PW+ | 981.916 µs | 1.020510 ms | 999.738 µs | 5.64 µs | 38.6 µs | 336 |
| 78 % load | PW− | 982.192 µs | 1.020602 ms | 1.001494 ms | 5.60 µs | 38.4 µs | 336 |

![jitter idle](part3_1_jitter_idle_stats.png)

![jitter under load](part3_1_jitter_load_stats.png)

Observations:

- Idle jitter is about 1 µs peak-to-peak. Under load it is about 39 µs peak-to-peak (±19 µs), roughly 40 times more, std dev 0.18 µs → 5.6 µs.
- The mean stays at 1.000–1.001 ms under load, so updates are shifted in time, not skipped. An update that comes late is followed by a shorter interval, which is why min and max are symmetric around 1 ms.
- The idle mean is 1.0014 ms, 0.14 % above nominal, in both PW+ and PW−. That is a constant offset of the timer clock, not jitter.
- The control thread has priority 1, above the main/CAN thread (3) and the system work queue (4), all preemptive. So the CAN thread cannot delay the control thread. The likely source of the extra jitter is interrupt-level work: the CAN RX ISR and message-queue put about 1000 times per second, which can delay the timer ISR that gives `control_sem`. Not verified further.
- `part3_1_jitter_first_attempt.png` (PW+ 990.9 µs to 1.012 ms, std dev 4.3 µs) was taken at 12:16 while `cangen` with the 40 RPM payload was most likely still running and the wheel was spinning. Not used, kept for reference only.

**Lost frames.** TPDO1 counted on the Pi for 10 s with `timeout -s INT 10 candump -ta can0,181:7FF > tpdo_X.log`. Logs in `logs/tpdo_idle.log` and `logs/tpdo_load.log`.

| Run | Frames in 10 s | Mean gap | Min gap | Max gap |
| --- | --- | --- | --- | --- |
| Idle | 999 | 10.014 ms | 9.967 ms | 10.067 ms |
| 78 % load | 998 | 10.015 ms | 9.733 ms | 10.710 ms |

![TPDO count idle](part3_1_tpdo_count_idle.png)

![TPDO count load](part3_1_tpdo_count_load.png)

No TPDO1 frames are lost under load (a lost frame would show as a gap of about 20 ms, the largest is 10.7 ms). The extra spread under load fits arbitration: TPDO1 (0x181) has higher priority than the flood frames (0x201), so it always wins arbitration but may have to wait for a 0x201 frame that is already being sent. A 4 byte frame takes 79–95 bits, 0.63–0.76 ms at 125 kbit/s, which matches the +0.7 ms maximum gap. Whether RPDO1 frames were dropped on the Nucleo side (message queue of 16) was not checked.

## Tracking controller, 0 → v → 0 (3.1)

Run with the Linux client (`./canopen_client can0 <log.csv> <limit>`, the third argument sets 0x6083 and 0x6084, 0 = true step). Target 0 until 1 s, 40 RPM until 4 s, then 0 until 6 s. PI, Kp 34, Ti 0.03 s. Logs `logs/trap_acc0.csv` and `logs/trap_acc100.csv`, plot from `tools/plot_part3_1.py`. 100 RPM/s was used instead of 1000 RPM/s since a 40 ms ramp is about the motor's own rise time and the trapezoid would hardly show. The setpoint generator's reference is not on TPDO1, so the plot reconstructs it from the target and the limit.

![0 → 40 → 0 with and without limit](../logs/part3_1_trapezoid.png)

| | No limit (true step) | 100 RPM/s |
| --- | --- | --- |
| 0 → 40 RPM | 30 ms | 371 ms (ramp 400 ms) |
| 40 → 0 RPM | 50 ms to first 0, then hunts ±1 count around 0 | 381 ms, then stays at 0 |
| Mean \|target − speed\| in the 1 s after each change | 1.90 RPM | 8.32 RPM |
| Mean \|reference − speed\| in the 1 s after each change | 1.90 RPM (reference = target) | 1.01 RPM (max 4) |
| PWM range | −255 to 255 | −57 to 252 |
| Samples at the PWM limit | 3 (2 on the step up, 1 on the step down) | 0 |

Observations:

- Without the limit the controller saturates on both steps, at +100 % on the way up and −100 % on the way down, where it drives the motor backwards to brake.
- With the limit the controller never saturates. It follows the ramp to within about one encoder count (mean 1.0 RPM from the reference), and the duty only reaches −22 % when slowing down. The error to the target is larger (8.3 RPM) simply because the reference deliberately lags the target. That is the point of the profile.
- After the true step down the speed overshoots through 0 to −3 RPM and then hunts between −3, 0 and +3 RPM (one count) for the rest of the run, with the PWM swinging about ±80. This is the same quantisation limit cycle as in Project 3: one count of error × Kp 34 is about 100 PWM, enough to break static friction in either direction. With the ramp the motor arrives at 0 slowly, static friction holds it, and it stays still.
- With the ramp the PWM stays at 16.6 % (42 PWM) after stopping. The error is 0, so the integral keeps its last value, which is below what is needed to overcome static friction. Harmless.
- At 40 RPM the step run happened to read exactly 40 every sample, while the ramp run chatters between 37 and 43. Which one happens depends on where the true speed sits between two encoder counts, not on the profile.

## Linux client (3.2)

Code in `rpi/canopen_client.c`, built on the Pi with `gcc -Wall -O2 -o canopen_client canopen_client.c`, run as `./canopen_client can0 <log.csv>`. Sequence:

1. NMT reset node, wait for Boot-up (0x701, 3 s timeout).
2. Read device type 0x1000 → 0x00000192 (= 402, the CiA 402 drive profile number).
3. Write by SDO and read back to verify, in Pre-operational: controller type 0x2000 = 1 (PI), Kp 0x2001 = 34.0, Ti 0x2002 = 0.03 s (the ZN-tuned values from Project 3, also used in Part 1), profile acceleration 0x6083 = 1000 RPM/s, deceleration 0x6084 = 1000 RPM/s. REAL32 values are sent and compared as their IEEE 754 bit pattern (e.g. 40.0 = 0x42200000, 0.1 = 0x3DCCCCCD in the first test runs), so the read-back comparison is exact. Controller type is written first since the Kp getter returns the Kp of the active controller.
4. NMT start.
5. Target 0 for 1 s, then step to 40 RPM on RPDO1. Every TPDO1 is logged to CSV as `time_s,target_rpm,speed_rpm,duty_pct`, time-stamped with `CLOCK_MONOTONIC` right after reception (µs resolution).
6. On exit, after 20 s, on a TPDO1 timeout (100 ms) or on Ctrl-C: target 0 on RPDO1 while still Operational, then NMT Pre-operational. The order matters since RPDO1 is ignored outside Operational.

Every SDO and NMT call is checked, and `report()` prints timeouts, I/O errors, unexpected responses and SDO abort codes.

**Abort test.** With an NMT start sent before the configuration block, the first write fails with `write 0x2000: SDO abort 0x08000022` (data cannot be transferred because of the present device state), so the configuration objects are correctly locked outside Pre-operational.

**Ctrl-C test.** SIGINT handler only sets a `volatile sig_atomic_t` flag, checked in the loop condition. Ctrl-C also interrupts `poll()` in `can_recv_id`, so the loop exits at once. candump shows the last frames as `201 [4] 00 00 00 00` then `000 [2] 80 01`, after which TPDO1 stops.

![Ctrl-C stop sequence](part3_2_ctrlc_candump.png)

**Step 0 → 40 RPM** (`logs/step_40.csv`, PI, Kp 34, Ti 0.03 s, acc 1000 RPM/s, 20 s run). Step at 1.014 s, speed first reaches 40 RPM at 1.054 s (40 ms, the same as the 1000 RPM/s ramp to 40 RPM, so the motor follows the ramp), duty briefly at 100 % during the rise. From 3 s on the speed averages exactly 40.00 RPM (single-count chatter 37–43) at a mean duty of 50.5 %, so no steady-state error. The duty jumps whenever a sample reads one count off: the speed resolution at a 10 ms window is about 3 RPM (2.86 RPM, Project 3), and one count × Kp 34 is a large kick per sample.

An earlier 5 s run with the reset defaults (Kp 40, Ti 0.1 s) behaved the same way (40 RPM reached in 40 ms, mean duty 47.5 %), but was overwritten by the run above so that the comparison with Project 3 uses the same tuning.

An earlier test with a 100 RPM target saturated at 100 % duty and 74–77 RPM, since the motor maximum is 79.6 RPM (Project 3), so targets must stay below about 75 RPM.

**Load response** (`logs/load_40.csv`, same settings). Moderate braking by hand on the shaft from about 6 s to 12 s, as in the Project 3 load test.

| Interval | Speed mean | Duty mean | Samples at 100 % duty |
| --- | --- | --- | --- |
| 2–6 s, no load | 40.0 RPM | 50.0–50.4 % | 0 |
| 6–12 s, load | 39.3–39.7 RPM | 62–70 % | 6–25 per second (single-sample kicks, not continuous saturation) |
| 13–20 s, released | 40.0 RPM | 50.2–50.6 % | 0 |

The PI controller holds the mean speed within half an encoder count of the reference under load and gives the extra duty through the integral. After release it returns to 40 RPM with no visible overshoot. Project 3 PI with moderate load: PWM ~122 → ~185 of 255 (48 → 73 %), speed 39.0–39.3 RPM, so the two are very close. From 13 s on the speed reads exactly 40 every sample and the duty stays at 50–52.6 %, so the count chatter can also vanish depending on where the speed sits between two encoder counts.

**Plots** (`tools/plot_part3_2.py`, run with a Python that has matplotlib, e.g. `/usr/local/bin/python3`; saved in `logs/`). P4 duty is converted to PWM counts (× 2.55) to share the axis with Project 3. Project 3 logs: `Project3_part3/logs/part4_tune1_kp34_Ti003.csv` (step) and `part4_PI_loadApplied.csv` (moderate load 18.0–23.9 s).

| Run | t90 | Max | Steady state 2–8 s | Std dev |
| --- | --- | --- | --- | --- |
| Project 4, TPDO1 on the Pi | 40 ms | 43.0 RPM | 40.00 RPM | 0.30 RPM |
| Project 3, serial log | 30 ms | 42.9 RPM | 40.00 RPM | 0.35 RPM |

The P4 rise is set by the 1000 RPM/s acceleration limit (40 ms to 40 RPM), while Project 3 had a true step. With acc = 0 the rise would be expected to match. Overshoot is one encoder count in both, and the steady state is identical. Steady-state PWM is about 128 in P4 against about 122 in Project 3, which is within run-to-run differences in friction.

![Step P3 vs P4](../logs/part3_2_step_P3_vs_P4.png)

The load plots show both runs side by side from 2 s before onset. Both hold the speed at 37–40 RPM (single-count chatter) under the load, with the PWM rising from about 125 to about 155 plus kicks to 255 on the samples that read 37. Both recover to 40 RPM with no overshoot. The Project 3 trace drops to 0 at about 9.3 s because the run was stopped there.

![Load P3 vs P4](../logs/part3_2_load_P3_vs_P4.png)

Single-run plots: `logs/part3_2_step.png`, `logs/part3_2_load.png`.

**Linux-side time stamps.** TPDO1 interval from the client's `CLOCK_MONOTONIC` time stamps:

| Log | Rows | Mean | Min | Max | Std dev |
| --- | --- | --- | --- | --- | --- |
| `step_40.csv` | 1998 | 10.014 ms | 9.905 ms | 10.109 ms | 12 µs |
| `load_40.csv` | 1998 | 10.013 ms | 9.901 ms | 10.143 ms | 15 µs |

For comparison the candump kernel time stamps on an idle bus spread 9.967–10.067 ms (`logs/tpdo_idle.log`). So the user-space time stamps spread about ±0.1 ms around the 10 ms period, roughly twice the spread of the kernel time stamps. The node's control loop jitters only about 1 µs, so the spread comes from TPDO1 being sent from the work queue on the node plus reception and scheduling on the Pi, with the user-space part adding about ±0.05 ms on top of the kernel time stamps. Not separated further. The mean of 10.014 ms instead of 10 ms matches the 1.0014 ms control period measured on the scope (TPDO1 every 10th update).

## Still to do for Part 3

- 3.1: higher inertia test (weight on a string), torque budget, choose acc/dec limits so the PWM does not saturate.
- 3.1: task table (trigger, period, mechanism, priority, why), shared data and protection, priority and time-slicing justification against Assignment 2.2. TDD test output before/after is in `part3_1_tests_before.png` and `part3_1_tests_after.png`.
- 3.3: start on boot (systemd), plus heartbeat if time allows; paragraph on the features not done.
- Video and report.
