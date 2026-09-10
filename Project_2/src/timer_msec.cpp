#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer_msec.h>
    
Timer_msec::Timer_msec(){}

void Timer_msec:: init(int period_ms, int duty_cycle = 50){
    TCCR1A = 0; // Setting timer1 to normal operation
    TCCR1B = 0;
    TCNT1 = 0; // Initialize counter value

    uint32_t period_ticks = (F_CPU / 1024UL) * (uint32_t)period_ms / 1000UL; // F_CPU = 16 000 000
    if (period_ticks > 65536UL) period_ticks = 65536UL;      // 16-bit clamp
    OCR1A = (uint16_t)(period_ticks - 1); // F_CPU = 16 000 000
    set_duty_cycle(duty_cycle);

    TCCR1B |= (1<<WGM12);
    TIMSK1 |= (1<<OCIE1A);
    TCCR1B |= (1<<CS12) | (1<<CS10);
}
void Timer_msec::set_duty_cycle(int duty_cycle){
    uint16_t on_ticks = (uint16_t)((uint32_t)OCR1A * duty_cycle / 100UL);

    uint8_t sreg = SREG;
    cli();
    OCR1B = on_ticks;
    SREG = sreg;
}