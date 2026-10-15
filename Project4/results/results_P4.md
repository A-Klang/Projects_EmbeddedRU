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

Lines changed: +21 / -22

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

Lines changed: +39 / -26

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
| encoder.cpp | +21 / -22 |
| nmt_states.cpp | +11 / -22 |
| main.cpp | +39 / -26 |
| timer_usec.h, timer_usec.cpp | removed (-43) |
| timer_msec.h, context.h, context.cpp, state.h, nmt_states.h, controller.h, controllers.h, controllers.cpp | unchanged |
