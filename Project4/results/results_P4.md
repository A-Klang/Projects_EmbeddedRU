## analog_out.h

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Timer_usec | uses a uint32_t for a period in nanoseconds, using built-in timer on zephyr | don't need our own timer, will use zephyr built-in timer instead |

## analog_out.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Timer and pin initialization, set_duty_cycle() and ISRs | setting the pwm directly with pwm_set_dt and calculating the pulse from duty_cycle and period_ns | TODO |

## timer_msec.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Timer1 in CTC mode with a compare match interrupt, ISR in encoder.cpp | k_timer (K_TIMER_DEFINE, k_timer_start) that calls timer_msec_tick() every period | TODO |
| set_duty_cycle() writing OCR1B | empty function, kept for the interface | TODO |

## encoder.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Pin-change interrupt on C1 (PCMSK2, PCICR, ISR(PCINT2_vect), portd_history) | GPIO callback on both edges of C1 (gpio_pin_interrupt_configure_dt, gpio_init_callback, gpio_add_callback) | TODO |
| ATOMIC_BLOCK in get_speed() | irq_lock() / irq_unlock() | TODO |
| ISR(TIMER1_COMPA_vect) | timer_msec_tick(), called by the k_timer in timer_msec.cpp | TODO |

## nmt_states.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Serial.print / Serial.println for status messages (Arduino) | printk | TODO |
| ATOMIC_BLOCK when reading ms_since_start in Operational::on_entry() | irq_lock() / irq_unlock() | TODO |
| Arduino.h providing fabs() | math.h | TODO |

## main.cpp

| AVR mechanism | Zephyr API | Why |
| --- | --- | --- |
| Serial.begin() and sei() | removed, Zephyr starts the UART and enables interrupts itself | TODO |
| Serial.print / Serial.println for the CSV output | printk | TODO |
| Serial.available() / Serial.read() for commands | uart_poll_in() on the console UART (non-blocking) | TODO |
| Serial.readStringUntil(), String, trim(), toDouble() for Kp, Ti and control law | read_line() helper with uart_poll_in(), char buffer and atof() | TODO |
| ATOMIC_BLOCK when reading ms_since_start | irq_lock() / irq_unlock() | TODO |
