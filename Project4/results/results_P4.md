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

